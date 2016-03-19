#include "role.hpp"

void
Role :: periodic(uint64_t ts) {
	switch (m_state) {
	case Leader:
		periodic_leader(ts);
		break;
	case PotentialLeader:
		periodic_potential_leader(ts);
		break;
	case Follower:
		periodic_follower(ts);
		break;
	}
}

void
Role :: periodic_leader(uint64_t ts) {
	if (ts - m_leader_data->m_last_broadcast > 300e6) {
		// It's been over 300 ms since the last broadcast.
		if (m_leader_data->m_pending_votes > 0) {
			// Didn't get a majority of the votes.
			if (m_leader_data->m_callback != nullptr) {
				// Append was not confirmed by a majority.
				m_leader_data->m_callback(-1, m_leader_data->m_callback_data);
			}
			DLOG(INFO) << "Can't confirm leadership. Dropping down to PotentialLeader state.";
			DLOG(INFO) << "Missing " << m_leader_data->m_pending_votes << " votes";
			m_state = PotentialLeader;
			if (m_client_callbacks.lost_leadership != nullptr) {
				m_client_callbacks.lost_leadership(m_client_callbacks_data);
			}
			m_leader_data = nullptr;
			m_potential_leader_data = std::make_unique<PotentialLeaderData>();
			m_potential_leader_data->m_pending_votes = m_cluster_size/2;
			return;
		}
	}

	if (m_leader_data->m_pending_votes == 0) {
		// We got all of the votes already.
		if (m_leader_data->m_callback != nullptr) {
			// Pending callback. Call it.
			m_leader_data->m_callback(0, m_leader_data->m_callback_data);
			m_leader_data->m_callback = nullptr;
			m_leader_data->m_callback_data = nullptr;
		}
		if (ts - m_leader_data->m_last_broadcast > 100e6) {
			// It's been over 100 ms since the last broadcast.
			// Broadcast another heartbeat.
			++m_seq;
			LeaderActiveMessage msg(m_id, m_round, m_seq);
			m_registry.broadcast(&msg);
			m_leader_data->m_pending_votes = m_cluster_size/2;
			m_leader_data->m_last_broadcast = ts;
		}
	}
}

void
Role :: periodic_potential_leader(uint64_t ts) {
	if (ts - m_potential_leader_data->m_last_broadcast > 300e6) {
		// It's been over 1 second since the last broadcast.
		if (m_potential_leader_data->m_pending_votes > 0) {
			// Didn't get a majority of the votes.
			m_potential_leader_data->m_pending_votes = m_cluster_size/2;
			m_potential_leader_data->m_ready_to_commit = 0;
			++m_seq;
			LeaderActiveMessage msg(m_id, m_round, m_seq);
			m_registry.broadcast(&msg);
			m_potential_leader_data->m_pending_votes = m_cluster_size/2;
			m_potential_leader_data->m_last_broadcast = ts;
			return;
		}
	}

	if (m_potential_leader_data->m_pending_votes == 0) {
		// We got all of the votes.
		LOG(INFO) << "Received majority vote. Assuming leadership.";
		if (m_pending_commit == m_round+1) {
			m_potential_leader_data->m_ready_to_commit++;
		}
		if (m_potential_leader_data->m_ready_to_commit == m_cluster_size/2) {
			// A majority are ready to commit the next round,
			// so we'll do it.
			m_round++;
		}
		m_state = Leader;
		if (m_client_callbacks.gained_leadership != nullptr) {
			m_client_callbacks.gained_leadership(m_client_callbacks_data);
		}
		m_potential_leader_data = nullptr;
		m_leader_data = std::make_unique<LeaderData>();
		++m_seq;
		LeaderActiveMessage msg(m_id, m_round, m_seq);
		m_registry.broadcast(&msg);
		m_leader_data->m_pending_votes = m_cluster_size/2;
		m_leader_data->m_last_broadcast = ts;
	}
}

void
Role :: periodic_follower(uint64_t ts) {
	if (ts - m_follower_data->m_last_leader_active > 1000e6) {
		// Leader hasn't been active for over 1 sec
		DLOG(INFO) << "Leader has timed out. Moving up to PotentialLeader state.";
		m_follower_data = nullptr;
		m_state = PotentialLeader;
		m_potential_leader_data = std::make_unique<PotentialLeaderData>();
		m_potential_leader_data->m_pending_votes = m_cluster_size/2;
	}
}

void
Role :: handle_leader_active(uint64_t ts, const LeaderActiveMessage& msg) {
	if (m_state == Follower) {
		if (m_follower_data->m_current_leader != 0) {
			// We have a leader already.
			if (m_follower_data->m_current_leader != msg.id &&
				ts - m_follower_data->m_last_leader_active <= 1000e6) {
				// Leader hasn't timed out yet.
				return;
			}
			if (m_round > msg.round) {
				// The other node is behind.
				return;
			}
			if (m_round == msg.round && msg.id > m_id) {
				// It's less authoritative.
				return;
			}
			if (m_follower_data->m_current_leader < msg.id) {
				// It's less authoritative than the current
				// leader so we'll respect current leadership.
				return;
			}
		}
		if (m_follower_data->m_current_leader > 0 &&
			msg.id != m_follower_data->m_current_leader) {
			DLOG(INFO) << "Acking leadership from " << msg.id;
			if (m_client_callbacks.on_leader_change != nullptr) {
				m_client_callbacks.on_leader_change(msg.id, m_client_callbacks_data);
			}
		}
		m_follower_data->m_last_leader_active = ts;
		m_follower_data->m_current_leader = msg.id;
		m_round = msg.round;
		if (m_round > 0 && m_round == m_pending_commit) {
			if (m_client_callbacks.on_commit != nullptr) {
				m_client_callbacks.on_commit(m_round, m_client_callbacks_data);
			}
			m_pending_commit = 0;
		}
		m_seq = msg.seq;
		LeaderActiveAck ack(msg.seq, m_pending_commit);
		m_registry.send(msg.source, &ack);
		return;
	}

	// Not currently a follower.
	if (msg.round > m_round || (msg.round >= m_round && msg.id < m_id)) {
		// From a node that's ahead and/or more authoritative.
		// Drop down to a follower state and make this node our leader.
		DLOG(INFO) << "Active leader msg from more authoritative node. Dropping to follower state.";
		m_leader_data = nullptr;
		m_potential_leader_data = nullptr;
		m_follower_data = std::make_unique<FollowerData>();
		m_state = Follower;
		if (m_client_callbacks.on_leader_change != nullptr) {
			m_client_callbacks.on_leader_change(msg.id, m_client_callbacks_data);
		}
		m_follower_data->m_current_leader = msg.id;
		m_follower_data->m_last_leader_active = ts;
		m_round = msg.round;
		DLOG(INFO) << "Acking leadership from " << msg.id;
		LeaderActiveAck ack(msg.seq, m_pending_commit);
		m_registry.send(msg.source, &ack);
		return;
	}
}
#pragma once

#include <memory>
#include <glog/logging.h>

#include "message/message.hpp"
#include "peer_registry.hpp"

enum State
{
	Follower,
	PotentialLeader,
	Leader
};

class Role
{
public:
	Role(PeerRegistry& registry, uint64_t id, int cluster_size)
	: m_registry(registry)
	, m_state(Follower)
	, m_id(id)
	, m_seq(0)
	, m_current_leader(0)
	, m_last_leader_active(0)
	, m_cluster_size(cluster_size)
	, m_pending_votes(cluster_size)
	{
	}

	void
	periodic(uint64_t ts)
	{
		if (m_state == Follower) {
			// Follower state
			if (ts - m_last_leader_active > 1e9) {
				// Leader hasn't been active for over 1 sec
				DLOG(INFO) << "Leader has timed out. Moving up to PotentialLeader state.";
				m_state = PotentialLeader;
				m_pending_votes = m_cluster_size-1;
			}
		}

		if (m_state == PotentialLeader) {
			if (m_pending_votes >= m_cluster_size/2+1) {
				m_pending_votes = m_cluster_size-1;
				m_current_leader = 0;
			} else {
				m_state = Leader;
				DLOG(INFO) << "Leadership ack'd by majority of cluster. Assuming leadership.";
			}
		}

		if (m_state == Leader) {
			if (m_pending_votes >= m_cluster_size/2+1) {
				m_pending_votes = m_cluster_size-1;
				m_state = PotentialLeader;
				DLOG(INFO) << "Can't confirm leadership. Dropping down to PotentialLeader state.";
				m_current_leader = 0;
			}
		}

		if (m_state != Follower) {
			LeaderActiveMessage msg(m_id, m_seq);
			m_registry.broadcast(&msg);
			m_pending_votes = m_cluster_size-1;
		}

		if (m_state == Leader && (ts/1000000000)%3 == 0) {
			m_seq++;
		}

		if (m_state != Leader && (ts/1000000000)%3 == 0 && m_current_leader != 0) {
			LOG(INFO) << "Current leader is " << m_current_leader;
		}
	}

	void
	handle_leader_active(uint64_t ts, const LeaderActiveMessage& msg)
	{
		if (m_state == Follower) {
			if (msg.id == m_current_leader && m_id > m_current_leader) {
				m_last_leader_active = ts;
				m_seq = msg.seq;
				LeaderActiveAck ack;
				m_registry.send(msg.source, &ack);
				return;
			}
			// Follower state
			if (msg.id > m_id) {
				// From a node with a higher ID
				if (msg.seq <= m_seq) {
					// Same seq number and less authoritative, so do nothing.
					return;
				} else {
					m_last_leader_active = ts;
					m_current_leader = msg.id;
					m_seq = msg.seq;
					DLOG(INFO) << "acking leadership";
					LeaderActiveAck ack;
					m_registry.send(msg.source, &ack);
					return;
				}
			} else {
				// From a node with a lower ID
				if (msg.seq < m_seq) {
					// ...but it's behind, so do nothing.
					return;
				}
				DLOG(INFO) << "Node " << msg.id << " is more authoritative. Treating as current leader.";
				DLOG(INFO) << "this: (" << m_id << ", " << m_seq << "), other: (" << msg.id <<
					", " << msg.seq << ")";
				m_last_leader_active = ts;
				m_current_leader = msg.id;
				m_seq = msg.seq;
				LeaderActiveAck ack;
				m_registry.send(msg.source, &ack);
				return;
			}
		}

		// Not currently a follower.
		if (msg.seq > m_seq || (msg.seq >= m_seq && msg.id < m_id)) {
			// From a node that's ahead and/or more authoritative.
			// Drop down to a follower state and make this node our leader.
			DLOG(INFO) << "Active leader msg from more authoritative node. Dropping to follower state.";
			DLOG(INFO) << "this: (" << m_id << ", " << m_seq << "), other: (" << msg.id <<
				", " << msg.seq << ")";
			m_state = Follower;
			m_last_leader_active = ts;
			m_seq = msg.seq;
			DLOG(INFO) << "Acking leadership from " << msg.id;
			LeaderActiveAck ack;
			m_registry.send(msg.source, &ack);
			return;
		}
	}

	void
	handle_leader_active_ack(uint64_t ts, const LeaderActiveAck& msg)
	{
		m_pending_votes--;
	}

private:
	PeerRegistry& m_registry;
	State         m_state;
	uint64_t      m_id;
	uint64_t      m_seq;

	uint64_t      m_current_leader;
	uint64_t      m_last_leader_active;

	int           m_cluster_size;
	int           m_pending_votes;

friend class RoleState;
}; // Role

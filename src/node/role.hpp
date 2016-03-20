#pragma once

#include <memory>
#include <functional>
#include <glog/logging.h>

#include "ab.h"
#include "message/message.hpp"
#include "peer_registry.hpp"

enum State
{
	Leader,
	PotentialLeader,
	Follower
};

struct LeaderData
{
	LeaderData()
	: m_pending_votes(0)
	, m_last_broadcast(0)
	, m_pending_round(0)
	{
	}

	uint64_t m_pending_votes;
	uint64_t m_last_broadcast;
	uint64_t m_pending_round;
	uint64_t m_ready_to_commit;
	uint64_t m_commit_check_seq;
	std::function<void(int, void*)> m_callback;
	void* m_callback_data;
}; // LeaderData

struct PotentialLeaderData
{
	uint64_t m_pending_votes;
	uint64_t m_last_broadcast;
	uint64_t m_ready_to_commit;
}; // PotentialLeaderData

struct FollowerData
{
	uint64_t m_current_leader;
	uint64_t m_last_leader_active;
}; // FollowerData

class Role
{
public:
	Role(PeerRegistry& registry, uint64_t id, int cluster_size)
	: m_registry(registry)
	, m_state(Follower)
	, m_follower_data(std::make_unique<FollowerData>())
	, m_id(id)
	, m_round(0)
	, m_cluster_size(cluster_size)
	, m_pending_commit(0)
	{
	}

	void
	periodic(uint64_t ts);

	void
	handle_leader_active(uint64_t ts, const LeaderActiveMessage& msg);

	void
	handle_append(uint64_t ts, const AppendMessage& msg)
	{
		LOG(INFO) << "handle_append";
		if (msg.round == m_round+1) {
			if (m_client_callbacks.on_append != nullptr) {
				m_pending_append = std::make_unique<AppendMessage>(msg);
				m_client_callbacks.on_append(msg.round, msg.append_content.c_str(),
					msg.append_content.size(), m_client_callbacks_data);
			}
		}
	}

	void
	client_confirm_append(uint64_t round)
	{
		LOG(INFO) << "client_confirm_append " << round;
		if (m_pending_append == nullptr) {
			LOG(INFO) << "m_pending_append == nullptr";
			return;
		}
		if (m_pending_append->round != round) {
			LOG(INFO) << "m_pending_append->round != round";
			return;
		}
		AppendAck ack(m_pending_append->round);
		m_registry.send(m_pending_append->source, &ack);
		m_pending_commit = round;
		m_pending_append = nullptr;
	}

	void
	handle_append_ack(uint64_t ts, const AppendAck& msg)
	{
		LOG(INFO) << "handle_append_ack";
		if (m_state != Leader) {
			return;
		}

		if (m_leader_data->m_callback == nullptr) {
			return;
		}

		if (msg.round == m_leader_data->m_pending_round) {
			if (m_leader_data->m_pending_votes > 0) {
				m_leader_data->m_pending_votes--;
			}
		}
		LOG(INFO) << "pending votes: " << m_leader_data->m_pending_votes;
	}

	void
	handle_leader_active_ack(uint64_t ts, const LeaderActiveAck& msg)
	{
		if (m_seq > msg.seq) {
			LOG(INFO) << "seq " << m_seq << " " << msg.seq;
			return;
		}
		switch (m_state) {
		case Leader:
			if (msg.uncommitted_round == m_pending_commit) {
				m_leader_data->m_ready_to_commit++;
			} else {
				LOG(INFO) << "uncommitted_round, m_pending_commit = " <<
					msg.uncommitted_round << ", " << m_pending_commit;
			}
			if (m_leader_data->m_pending_votes > 0) {
				//LOG(INFO) << "handle_leader_active_ack";
				m_leader_data->m_pending_votes--;
			}
			break;
		case PotentialLeader:
			if (msg.uncommitted_round == m_round+1) {
				m_potential_leader_data->m_ready_to_commit++;
			}
			if (m_potential_leader_data->m_pending_votes > 0) {
				m_potential_leader_data->m_pending_votes--;
			}
			break;
		case Follower:
			// Nothing to do.
			break;
		}
	}

	void
	send_append(std::string append_content, std::function<void(int, void*)> cb, void* data)
	{
		if (m_state != Leader) {
			cb(-1, data);
			return;
		}
		m_seq++;
		m_leader_data->m_pending_votes = m_cluster_size/2;
		m_leader_data->m_pending_round = m_round+1;
		m_leader_data->m_callback = cb;
		m_leader_data->m_callback_data = data;
		m_leader_data->m_ready_to_commit = 0;
		AppendMessage msg(m_leader_data->m_pending_round, append_content);
		m_registry.broadcast(&msg);
		LOG(INFO) << "Sending append...";
		m_leader_data->m_last_broadcast = uv_hrtime();
	}

	void
	set_callbacks(ab_callbacks_t callbacks, void* callbacks_data)
	{
		m_client_callbacks = callbacks;
		m_client_callbacks_data = callbacks_data;
	}

private:
	void
	periodic_leader(uint64_t ts);

	void
	periodic_potential_leader(uint64_t ts);

	void
	periodic_follower(uint64_t ts);

private:
	PeerRegistry& m_registry;
	uint64_t      m_id;
	uint64_t      m_round;
	uint64_t      m_seq;
	int           m_cluster_size;
	State         m_state;

	std::unique_ptr<AppendMessage> m_pending_append;
	uint64_t                       m_pending_commit;

	// Per-state data
	std::unique_ptr<LeaderData>          m_leader_data;
	std::unique_ptr<PotentialLeaderData> m_potential_leader_data;
	std::unique_ptr<FollowerData>        m_follower_data;

	ab_callbacks_t  m_client_callbacks;
	void*           m_client_callbacks_data;
}; // Role

#pragma once

#include <memory>
#include <functional>
#include <glog/logging.h>

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
	std::function<void(int, void*)> m_callback;
	void* m_callback_data;
}; // LeaderData

struct PotentialLeaderData
{
	uint64_t m_pending_votes;
	uint64_t m_last_broadcast;
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
	{
	}

	void
	periodic(uint64_t ts);

	void
	handle_leader_active(uint64_t ts, const LeaderActiveMessage& msg);

	void
	handle_append(uint64_t ts, const AppendMessage& msg)
	{
		if (msg.round == m_round+1) {
			AppendAck ack(msg.round);
			++m_round;
			m_registry.send(msg.source, &ack);
		}
	}

	void
	handle_append_ack(uint64_t ts, const AppendAck& msg)
	{
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

		if (m_leader_data->m_pending_votes == 0) {
			// We got all of the votes already.
			m_round++;
			m_leader_data->m_pending_round = 0;
			m_leader_data->m_callback(0, m_leader_data->m_callback_data);
			m_leader_data->m_callback = nullptr;
			m_leader_data->m_callback_data = nullptr;
		}
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
			if (m_leader_data->m_pending_votes > 0) {
				m_leader_data->m_pending_votes--;
			}
			break;
		case PotentialLeader:
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
		m_leader_data->m_pending_round = m_round+1;
		m_leader_data->m_callback = cb;
		m_leader_data->m_callback_data = data;
		AppendMessage msg(m_leader_data->m_pending_round, append_content);
		m_registry.broadcast(&msg);
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

	// Per-state data
	std::unique_ptr<LeaderData>          m_leader_data;
	std::unique_ptr<PotentialLeaderData> m_potential_leader_data;
	std::unique_ptr<FollowerData>        m_follower_data;
}; // Role

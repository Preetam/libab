#pragma once

#include <memory>
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
	uint64_t m_pending_votes;
	uint64_t m_last_broadcast;
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

#pragma once

#include <memory>
#include <glog/logging.h>

#include "message/message.hpp"
#include "peer_registry.hpp"

class RoleState
{
public:
	RoleState()
	: m_seq(0)
	{
	}

	// Disable copying.
	RoleState(const RoleState& rhs) = delete;
	RoleState& operator =(RoleState& rhs) = delete;

	virtual void
	periodic(uint64_t ts) = 0;

	uint64_t m_seq;
}; // RoleState

class FollowerState : public RoleState
{
public:
	FollowerState()
	: m_current_leader(0)
	{
		DLOG(INFO) << "FollowerState";
	}

	FollowerState(uint64_t current_leader)
	: m_current_leader(current_leader)
	{
		DLOG(INFO) << "FollowerState";
	}

	virtual void
	periodic(uint64_t ts)
	{
		DLOG(INFO) << "FollowerState::periodic()";
		// TODO
	}

public:
	uint64_t m_current_leader;
	uint64_t m_last_leader_active;
}; // FollowerState

class PotentialLeaderState : public RoleState
{
public:
	PotentialLeaderState(int cluster_size)
	: m_cluster_size(cluster_size)
	, m_pending_votes(cluster_size)
	{
		DLOG(INFO) << "PotentialLeaderState";
	}

	virtual void
	periodic(uint64_t ts)
	{
		DLOG(INFO) << "PotentialLeaderState::periodic()";
		// TODO
	}

public:
	int m_cluster_size;
	int m_pending_votes;
}; // PotentialLeaderState

class LeaderState : public RoleState
{
public:
	LeaderState()
	{
		DLOG(INFO) << "LeaderState";
	}

	virtual void
	periodic(uint64_t ts)
	{
		DLOG(INFO) << "LeaderState::periodic()";
		// TODO
	}
private:
}; // LeaderState

class Role
{
public:
	Role(PeerRegistry& registry, uint64_t id, int cluster_size)
	: m_registry(registry)
	, m_state(std::make_unique<FollowerState>())
	, m_id(id)
	, m_cluster_size(cluster_size)
	{
	}

	void
	periodic(uint64_t ts)
	{
		m_state->periodic(ts);
		auto follower_state = dynamic_cast<FollowerState*>(m_state.get());
		if (follower_state != nullptr) {
			// Follower state
			if (ts - follower_state->m_last_leader_active > 1e9) {
				// Leader hasn't been active for over 1 sec
				m_state = std::make_unique<PotentialLeaderState>(m_cluster_size);
				return;
			}
		}
	}

	void
	handle_leader_active(uint64_t ts, const LeaderActiveMessage& msg)
	{
		auto follower_state = dynamic_cast<FollowerState*>(m_state.get());
		if (follower_state != nullptr) {
			// Follower state
			if (msg.id > m_id) {
				// From a node with a higher ID
				if (msg.seq <= m_state->m_seq) {
					// Same seq number and less authoritative, so do nothing.
					return;
				} else {
					follower_state->m_last_leader_active = ts;
					follower_state->m_current_leader = msg.id;
					m_state->m_seq = msg.seq;
					DLOG(INFO) << "acking leadership";
					LeaderActiveAck ack;
					m_registry.send(msg.source, &ack);
					return;
				}
			} else {
				// From a node with a lower ID
				if (msg.seq < m_state->m_seq) {
					// ...but it's behind, so do nothing.
					return;
				} else {
					follower_state->m_last_leader_active = ts;
					follower_state->m_current_leader = msg.id;
					m_state->m_seq = msg.seq;
					DLOG(INFO) << "acking leadership";
					LeaderActiveAck ack;
					m_registry.send(msg.source, &ack);
					return;
				}
			}
		}

		// Not currently a follower.

		auto potential_leader_state = dynamic_cast<PotentialLeaderState*>(m_state.get());
		if (potential_leader_state != nullptr) {
			// Potential leader.
			if (msg.seq > m_state->m_seq || (msg.seq >= m_state->m_seq && msg.id < m_id)) {
				// From a node that's ahead and/or more authoritative.
				// Drop down to a follower state and make this node our leader.
				m_state = std::make_unique<FollowerState>(msg.id);
				follower_state = dynamic_cast<FollowerState*>(m_state.get());
				follower_state->m_last_leader_active = ts;
				m_state->m_seq = msg.seq;
				DLOG(INFO) << "acking leadership";
				LeaderActiveAck ack;
				m_registry.send(msg.source, &ack);
				return;
			}
			return;
		}

		auto leader_state = dynamic_cast<LeaderState*>(m_state.get());
		if (leader_state != nullptr) {
			// Leader.
			if (msg.seq > m_state->m_seq || (msg.seq >= m_state->m_seq && msg.id < m_id)) {
				// From a node that's ahead and/or more authoritative.
				// Drop down to a follower state and make this node our leader.
				m_state = std::make_unique<FollowerState>(msg.id);
				follower_state = dynamic_cast<FollowerState*>(m_state.get());
				follower_state->m_last_leader_active = ts;
				m_state->m_seq = msg.seq;
				DLOG(INFO) << "acking leadership";
				LeaderActiveAck ack;
				m_registry.send(msg.source, &ack);
				return;
			}
			return;
		}
	}

private:
	PeerRegistry&              m_registry;
	std::unique_ptr<RoleState> m_state;
	uint64_t                   m_id;
	int                        m_cluster_size;
}; // Role

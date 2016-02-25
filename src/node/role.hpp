#pragma once

#include <memory>
#include <glog/logging.h>

#include "peer_registry.hpp"

class RoleState
{
public:
	RoleState()
	: m_term(0)
	, m_seq(0)
	{
	}

	// Disable copying.
	RoleState(const RoleState& rhs) = delete;
	RoleState& operator =(RoleState& rhs) = delete;

	virtual void
	periodic(uint64_t ts) = 0;

private:
	int64_t m_term;
	int64_t m_seq;
}; // RoleState

class FollowerState : public RoleState
{
public:
	FollowerState()
	: m_current_leader(0)
	{
		DLOG(INFO) << "FollowerState";
	}

	virtual void
	periodic(uint64_t ts)
	{
		DLOG(INFO) << "FollowerState::periodic()";
		// TODO
	}

private:
	uint64_t m_current_leader;
}; // FollowerState

class PotentialLeaderState : public RoleState
{
	// TODO
private:
	int m_cluster_size;
	int m_pending_votes;
}; // PotentialLeaderState

class LeaderState : public RoleState
{
	// TODO
}; // LeaderState

class Role
{
public:
	Role(PeerRegistry& registry)
	: m_registry(registry)
	, m_state(std::make_unique<FollowerState>())
	{
	}

	void
	periodic(uint64_t ts)
	{
		m_state->periodic(ts);
	}

private:
	PeerRegistry&              m_registry;
	std::unique_ptr<RoleState> m_state;
}; // Role

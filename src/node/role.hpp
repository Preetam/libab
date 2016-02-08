#pragma once

#include <memory>

class RoleState
{
	// TODO

private:
	int64_t m_term;
	int64_t m_seq;
}; // RoleState

class FollowerState : RoleState
{

private:
	uint64_t m_current_leader;
}; // FollowerState

class PotentialLeaderState : RoleState
{
	// TODO
private:
	int m_cluster_size;
	int m_pending_votes;
}; // PotentialLeaderState

class LeaderState : RoleState
{
	// TODO
}; // LeaderState

class Role
{
	// TODO

private:
	std::unique_ptr<RoleState> m_state;
}; // Role

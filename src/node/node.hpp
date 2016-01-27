#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <cerrno>

#include <uv.h>
#include <glog/logging.h>
#include <cpl/net/sockaddr.hpp>

#include "peer/peer.hpp"
#include "peer_registry.hpp"
#include "message_queue/message_queue.hpp"
#include "message/identity_message.hpp"

class Node
{
public:
	Node(uint64_t id, int cluster_size)
	: m_id(id)
	, m_cluster_size(cluster_size)
	, m_peer_registry(std::make_unique<PeerRegistry>())
	, m_mq(std::make_shared<Message_Queue>())
	{
		LOG(INFO) << "Node initialized with cluster size " << m_cluster_size;
	}

	// start starts a node listening at address.
	// A negative value is returned for errors.
	int
	start(std::string address);

	// run starts the main processing routine.
	void
	run();

	// connect_to_peer connects to a peer with the given
	// address and creates a new Peer instance for the node.
	// If the node is unable to establish a connection, the
	// peer is registered as a failed node.
	void
	connect_to_peer(cpl::net::SockAddr&);

private:
	uint64_t                              m_id;
	std::string                           m_listen_address;
	std::unique_ptr<uv_loop_t>            m_uv_loop;
	std::unique_ptr<uv_tcp_t>             m_tcp;
	std::unique_ptr<PeerRegistry>         m_peer_registry;
	int                                   m_index_counter;
	std::shared_ptr<Message_Queue>        m_mq;
	int                                   m_cluster_size;

	uint64_t                              m_trusted_peer;
	std::chrono::steady_clock::time_point m_last_leader_active;

	void
	periodic();

	static void
	on_connect(uv_stream_t* server, int status);

	void
	handle_message(const Message*);

	void
	handle_ident(const Message&);
}; // Node

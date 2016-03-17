#pragma once

#include <thread>
#include <memory>
#include <cerrno>
#include <future>

#include <uv.h>
#include <glog/logging.h>
#include <cpl/net/sockaddr.hpp>

#include "sacq.h"
#include "role.hpp"
#include "peer/peer.hpp"
#include "peer_registry.hpp"
#include "message/message.hpp"

const uint64_t leader_timeout_ns = 1e9;

class Node
{
public:
	Node(uint64_t id, int cluster_size)
	: m_id(id)
	, m_cluster_size(cluster_size)
	, m_peer_registry(std::make_unique<PeerRegistry>())
	, m_trusted_peer(0)
	, m_last_leader_active(uv_hrtime())
	, m_role(std::make_unique<Role>(*m_peer_registry, id, cluster_size))
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

	void
	append(std::string content, ab_append_cb cb, void* data)
	{
		auto async = new uv_async_t;
		auto task = new std::packaged_task<void()>([=]() {
			m_role->send_append(content, cb, data);
			uv_close((uv_handle_t*)async, [](uv_handle_t* handle) {
				// delete packaged_task
				auto func = reinterpret_cast<std::packaged_task<void()>*>(handle->data);
				delete func;
				// delete async
				delete handle;
			});
		});
		async->data = task;
		uv_async_init(m_uv_loop.get(), async, [](uv_async_t* handle) {
			auto func = reinterpret_cast<std::packaged_task<void()>*>(handle->data);
			(*func)();
		});
		uv_async_send(async);
	}

private:
	uint64_t                      m_id;
	std::string                   m_listen_address;
	std::unique_ptr<uv_loop_t>    m_uv_loop;
	std::unique_ptr<uv_tcp_t>     m_tcp;
	std::unique_ptr<PeerRegistry> m_peer_registry;
	int                           m_index_counter;
	int                           m_cluster_size;

	uint64_t                      m_trusted_peer;
	uint64_t                      m_last_leader_active;
	std::unique_ptr<Role>         m_role; // TODO

	void
	periodic();

	static void
	on_connect(uv_stream_t* server, int status);

	void
	handle_message(const Message*);
}; // Node

#include "node.hpp"

int
Node :: start(std::string address) {
	// Create a libuv event loop.
	m_uv_loop = std::make_unique<uv_loop_t>();
	if (uv_loop_init(m_uv_loop.get()) < 0) {
		LOG(ERROR) << "failed to initialize event loop";
		return -1;
	}

	// Parse address string.
	struct sockaddr_storage sockaddr;
	cpl::net::SockAddr addr;
	if (addr.parse(address) < 0) {
		LOG(ERROR) << "failed to parse listen address";
		return -1;
	}
	addr.get_sockaddr(reinterpret_cast<struct sockaddr*>(&sockaddr));

	// Create a TCP handle.
	m_tcp = std::make_unique<uv_tcp_t>();
	if (uv_tcp_init(m_uv_loop.get(), m_tcp.get()) < 0) {
		LOG(ERROR) << "failed to initialize TCP handle";
		return -1;
	}
	m_tcp->data = this;
	auto status = uv_tcp_bind(m_tcp.get(), reinterpret_cast<struct sockaddr*>(&sockaddr), 0);
	if (status < 0) {
		LOG(ERROR) << "failed to bind to address: " << uv_strerror(status);
		return -1;
	}

	if (uv_listen((uv_stream_t*)m_tcp.get(), 8, Node::on_connect) < 0) {
		LOG(ERROR) << "uv_listen failed";
		return -1;
	}

	m_listen_address = address;
	return 0;
}

void
Node :: connect_to_peer(cpl::net::SockAddr& addr) {
	// Create a TCP handle.
	auto handle = std::make_unique<uv_tcp_t>();
	if (uv_tcp_init(m_uv_loop.get(), handle.get()) < 0) {
		LOG(ERROR) << "failed to initialize TCP handle";
		return;
	}

	auto peer = std::make_shared<Peer>(addr, std::move(handle), m_mq);
	IdentityMessage ident_msg(m_id, m_listen_address);
	peer->send(&ident_msg);
	m_peer_registry->register_peer(++m_index_counter, peer);
	return;
}

void
Node :: run() {
	uv_timer_t periodic_timer;
	uv_timer_init(m_uv_loop.get(), &periodic_timer);
	periodic_timer.data = this;
	uv_timer_start(&periodic_timer, [](uv_timer_t* timer) {
		auto self = (Node*)timer->data;
		self->periodic();
	},
	1000, 1000);
	if (uv_run(m_uv_loop.get(), UV_RUN_DEFAULT) < 0) {
		LOG(ERROR) << "failed to run event loop";
		return;
	}
	uv_loop_close(m_uv_loop.get());
}

void
Node ::	on_connect(uv_stream_t* server, int status) {
	auto self = (Node*)server->data;
	if (status < 0) {
		return;
	}
	auto client = std::make_unique<uv_tcp_t>();
	uv_tcp_init(server->loop, client.get());
	uv_accept(server, (uv_stream_t*)client.get());
	auto peer = std::make_shared<Peer>(std::move(client), self->m_mq);
	IdentityMessage ident_msg(self->m_id, self->m_listen_address);
	peer->send(&ident_msg);
	self->m_peer_registry->register_peer(++self->m_index_counter, peer);
}

void
Node :: periodic() {
	LOG(INFO) << "periodic run for node (id " << m_id << ")";

	// Clean up the registry
	m_peer_registry->cleanup();

	// Process messages
	while (m_mq->size() > 0) {
		auto msg = m_mq->pop();
		LOG(INFO) << "got a new message of type " << MSG_STR(msg->type) <<
			" from index " << msg->source;

		handle_message(msg.get());
	}
}

void
Node :: handle_message(const Message* msg) {
	switch (msg->type) {
	case MSG_IDENT:
		handle_ident(*msg);
		break;
	}
}

void
Node :: handle_ident(const Message& msg) {
	auto ident_msg = static_cast<const IdentityMessage&>(msg);
	m_peer_registry->set_identity(ident_msg.source, ident_msg.id, ident_msg.address);
	LOG(INFO) << ident_msg.source << " has ID " << ident_msg.id << " and address " <<
		ident_msg.address;
}

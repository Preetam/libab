#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>

#include "peer/peer.hpp"

class PeerRegistry
{
	using shared_peer = std::shared_ptr<Peer>;

public:
	PeerRegistry()
	{
	}

	void
	register_peer(int index, shared_peer peer)
	{
		peer->set_index(index);
		m_peers[index] = peer;
		if (peer->id() > 0) {
			m_peer_id_index[peer->id()] = index;
		}
	}

	void
	set_identity(const int index, const uint64_t id, const std::string& address)
	{
		Peer* peer;
		try {
			auto peer_ptr = m_peers.at(index);
			peer = peer_ptr.get();
		} catch (...) {
			return;
		}
		peer->set_identity(id, address);
		// Check if another index has the same ID. If so,
		// update the other one.
		for (auto i = std::begin(m_peers); i != std::end(m_peers); ++i) {
			auto p = i->second;
			if (i->first != index && p->id() == id) {
				LOG(INFO) << "duplicate peer " << id;
				*p = std::move(*peer);
				cleanup();
				break;
			}
		}
		m_peer_id_index[id] = index;
	}

	uint64_t
	trusted_after(const uint64_t id)
	{
		for (auto i = std::begin(m_peer_id_index); i != std::end(m_peer_id_index); ++i) {
			if (i->first >= id) {
				return i->first;
			}
		}
		return 0;
	}

	void
	cleanup()
	{
		for (auto i = std::begin(m_peers); i != std::end(m_peers); ) {
			if (i->second == nullptr) {
				i = m_peers.erase(i);
			} else if (i->second->done()) {
				auto id = i->second->id();
				auto id_index_it = m_peer_id_index.find(id);
				if (id_index_it != m_peer_id_index.end()) {
					m_peer_id_index.erase(id_index_it);
				}
				i = m_peers.erase(i);
			} else {
				++i;
			}
		}
	}

private:
	std::unordered_map<int, shared_peer> m_peers;
	std::map<uint64_t, int>              m_peer_id_index;
}; // PeerRegistry

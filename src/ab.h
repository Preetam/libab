#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void (*on_append)(uint64_t round, uint64_t commit, const char* data, int data_len, void* cb_data);
	void (*on_commit)(uint64_t round, uint64_t commit, void* cb_data);
	void (*gained_leadership)(void* cb_data);
	void (*lost_leadership)(void* cb_data);
	void (*on_leader_change)(uint64_t leader_id, void* cb_data);
} ab_callbacks_t;

typedef void (*ab_append_cb)(int status, uint64_t round, uint64_t commit, void* data);

typedef struct ab_node_t ab_node_t;

ab_node_t*
ab_node_create(uint64_t id, int cluster_size, ab_callbacks_t callbacks, void* data);

void
ab_set_committed(ab_node_t* node, uint64_t round, uint64_t commit);

int
ab_listen(ab_node_t* node, const char* address);

int
ab_connect_to_peer(ab_node_t* node, const char* address);

int
ab_run(ab_node_t* node);

int
ab_append(ab_node_t* node, const char* content, int content_len, ab_append_cb cb, void* data);

void
ab_confirm_append(ab_node_t* node, uint64_t round, uint64_t commit);

int
ab_destroy(ab_node_t* node);

#ifdef __cplusplus
}
#endif

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ab_append_cb)(int status, void* data);

typedef struct ab_node_t ab_node_t;

ab_node_t*
ab_node_create(uint64_t id, int cluster_size);

int
ab_listen(ab_node_t* node, const char* address);

int
ab_connect_to_peer(ab_node_t* node, const char* address);

int
ab_run(ab_node_t* node);

int
ab_append(ab_node_t* node, const uint8_t* content, int content_len, ab_append_cb cb, void* data);

int
ab_destroy(ab_node_t* node);

#ifdef __cplusplus
}
#endif

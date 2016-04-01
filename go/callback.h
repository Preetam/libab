#pragma once

#include <ab.h>

extern void on_append_go_cb(uint64_t round, uint64_t commit, char*, int, void*);
void on_commit_go_cb(uint64_t round, uint64_t commit, void* cb_data);
extern void gained_leadership_go_cb(void* cb_data);
extern void lost_leadership_go_cb(void* cb_data);
extern void on_leader_change_go_cb(uint64_t, void*);

void on_append_go_cb_gateway(uint64_t round, uint64_t commit, const char* data, int data_len, void* cb_data);
void on_commit_go_cb_gateway(uint64_t round, uint64_t commit, void* cb_data);
void gained_leadership_go_cb_gateway(void* cb_data);
void lost_leadership_go_cb_gateway(void* cb_data);
void on_leader_change_go_cb_gateway(uint64_t leader_id, void* cb_data);

void set_callbacks(ab_callbacks_t* callbacks);

void append_go_gateway(ab_node_t* n, char* data, int data_len, int callbackNum);
void append_go_cb(int status, uint64_t round, uint64_t commit, void* cb_data);

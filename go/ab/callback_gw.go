package ab

/*
#include <stdint.h>
#include <stdlib.h>
#include "callback.h"

void on_append_go_cb_gateway(uint64_t round, uint64_t commit, const char* data, int data_len, void* cb_data) {
	onAppendGoCb(round, commit, (char*)data, data_len, cb_data);
}

void on_commit_go_cb_gateway(uint64_t round, uint64_t commit, void* cb_data) {
	onCommitGoCb(round, commit, cb_data);
}

void gained_leadership_go_cb_gateway(void* cb_data) {
	gainedLeadershipGoCb(cb_data);
}

void lost_leadership_go_cb_gateway(void* cb_data) {
	lostLeadershipGoCb(cb_data);
}

void on_leader_change_go_cb_gateway(uint64_t leader_id, void* cb_data) {
	onLeaderChangeGoCb(leader_id, cb_data);
}

void set_callbacks(ab_callbacks_t* callbacks) {
	callbacks->on_append = &on_append_go_cb_gateway;
	callbacks->on_commit = &on_commit_go_cb_gateway;
	callbacks->gained_leadership = &gained_leadership_go_cb_gateway;
	callbacks->lost_leadership = &lost_leadership_go_cb_gateway;
	callbacks->on_leader_change = &on_leader_change_go_cb_gateway;
}

void append_go_gateway(ab_node_t* n, char* data, int data_len, void* cb_data) {
	ab_append(n, data, data_len, appendGoCb, cb_data);
}
*/
import "C"

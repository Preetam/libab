package ab

import (
	"fmt"
	"sync"
	"unsafe"
)

/*
#cgo LDFLAGS: -lab

#include <stdlib.h>
#include <ab.h>
#include "callback.h"
*/
import "C"

var registeredNodesLock sync.RWMutex
var registrationCounter int
var registeredNodes = map[int]*Node{}

type Node struct {
	ptr             *C.ab_node_t
	callbacksNum    int
	callbackHandler CallbackHandler
	appendResult    chan appendResult
}

type appendResult struct {
	status int
	round  uint64
	commit uint64
}

type CallbackHandler interface {
	OnAppend(round uint64, commit uint64, data string)
	OnCommit(round uint64, commit uint64)
	GainedLeadership()
	LostLeadership()
	OnLeaderChange(leaderID uint64)
}

func NewNode(id uint64,
	listen string,
	callbackHandler CallbackHandler,
	clusterSize int) (*Node, error) {

	n := &Node{
		callbackHandler: callbackHandler,
	}
	cCallbacks := C.ab_callbacks_t{}
	C.set_callbacks(&cCallbacks)
	registeredNodesLock.Lock()
	defer registeredNodesLock.Unlock()
	registrationCounter++
	registeredNodes[registrationCounter] = n
	n.callbacksNum = registrationCounter
	ptr := C.ab_node_create(C.uint64_t(id), C.int(clusterSize), cCallbacks, unsafe.Pointer(&n.callbacksNum))
	listenStr := C.CString(listen)
	defer C.free(unsafe.Pointer(listenStr))
	ret := C.ab_listen(ptr, listenStr)
	if ret < 0 {
		C.ab_destroy(ptr)
		return nil, fmt.Errorf("error listening")
	}
	n.ptr = ptr
	return n, nil
}

func (n *Node) SetCommitted(round, commit uint64) {
	C.ab_set_committed(n.ptr, C.uint64_t(round), C.uint64_t(commit))
}

func (n *Node) Run() error {
	C.ab_run(n.ptr)
	return fmt.Errorf("failed")
}

func (n *Node) Append(data string) (uint64, uint64, error) {
	n.appendResult = make(chan appendResult)
	cData := C.CString(data)
	defer C.free(unsafe.Pointer(cData))
	C.append_go_gateway(n.ptr, cData, C.int(len(data)), C.int(n.callbacksNum))
	result := <-n.appendResult
	n.appendResult = nil
	if result.status == 0 {
		return result.round, result.commit, nil
	}
	return 0, 0, fmt.Errorf("append failed :(")
}

func (n *Node) ConfirmAppend(round, commit uint64) {
	C.ab_confirm_append(n.ptr, C.uint64_t(round), C.uint64_t(commit))
}

func (n *Node) AddPeer(address string) error {
	cAddr := C.CString(address)
	defer C.free(unsafe.Pointer(cAddr))
	ret := C.ab_connect_to_peer(n.ptr, cAddr)
	if ret < 0 {
		return fmt.Errorf("error adding peer")
	}
	return nil
}

//export on_append_go_cb
func on_append_go_cb(round C.uint64_t, commit C.uint64_t, str *C.char, length C.int, p unsafe.Pointer) {
	i := *(*int)(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	node.callbackHandler.OnAppend(uint64(round), uint64(commit), C.GoStringN(str, length))
}

//export on_commit_go_cb
func on_commit_go_cb(round C.uint64_t, commit C.uint64_t, p unsafe.Pointer) {
	i := *(*int)(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	node.callbackHandler.OnCommit(uint64(round), uint64(commit))
}

//export gained_leadership_go_cb
func gained_leadership_go_cb(p unsafe.Pointer) {
	i := *(*int)(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	node.callbackHandler.GainedLeadership()
}

//export lost_leadership_go_cb
func lost_leadership_go_cb(p unsafe.Pointer) {
	i := *(*int)(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	node.callbackHandler.LostLeadership()
}

//export on_leader_change_go_cb
func on_leader_change_go_cb(id C.uint64_t, p unsafe.Pointer) {
	i := *(*int)(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	node.callbackHandler.OnLeaderChange(uint64(id))
}

//export append_go_cb
func append_go_cb(status C.int, round C.uint64_t, commit C.uint64_t, p unsafe.Pointer) {
	i := *(*int)(p)
	C.free(p)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	node := registeredNodes[i]
	if node != nil && node.appendResult != nil {
		node.appendResult <- appendResult{
			status: int(status),
			round:  uint64(round),
			commit: uint64(commit),
		}
	}
}

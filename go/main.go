package main

import (
	"flag"
	"fmt"
	"time"
)

type SimpleHandler struct {
	nodeNumber int
}

func (h *SimpleHandler) OnAppend(round uint64, data string) {
	fmt.Println("OnAppend", round, data)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	registeredNodes[h.nodeNumber].ConfirmAppend(round)
}

func (h *SimpleHandler) OnCommit(round uint64) {
	fmt.Println("OnCommit", round)
}

func (h *SimpleHandler) GainedLeadership() {
	fmt.Println("GainedLeadership")
}

func (h *SimpleHandler) LostLeadership() {
	fmt.Println("LostLeadership")
}

func (h *SimpleHandler) OnLeaderChange(leaderID uint64) {
	fmt.Println("OnLeaderChange", leaderID)
}

func main() {
	nodeID := flag.Uint64("id", 1, "")
	clusterSize := flag.Int("cluster-size", 1, "")
	listen := flag.String("listen", "127.0.0.1:2020", "")
	flag.Parse()

	handler := &SimpleHandler{}
	node, err := NewNode(*nodeID, *listen, nil, *clusterSize)
	if err != nil {
		fmt.Println(err)
		return
	}
	handler.nodeNumber = node.callbacksNum
	node.callbackHandler = handler
	fmt.Println(node.AddPeer("127.0.0.1:2021"))
	go func() {
		node.Run()
		panic("unreachable")
	}()

	<-time.After(time.Second)
	for _ = range time.Tick(time.Second) {
		fmt.Println(node.Append("foobar"))
	}
	<-make(chan struct{})
}

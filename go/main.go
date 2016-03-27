package main

import (
	"flag"
	"fmt"
	"time"
)

type SimpleHandler struct {
	nodeNumber int
}

func (h *SimpleHandler) OnAppend(commit uint64, data string) {
	fmt.Println("OnAppend", commit, data)
	registeredNodesLock.RLock()
	defer registeredNodesLock.RUnlock()
	registeredNodes[h.nodeNumber].ConfirmAppend(commit)
}

func (h *SimpleHandler) OnCommit(commit uint64) {
	fmt.Println("OnCommit", commit)
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
	commit := flag.Uint64("commit", 0, "")
	flag.Parse()

	handler := &SimpleHandler{}
	node, err := NewNode(*nodeID, *listen, nil, *clusterSize)
	if err != nil {
		fmt.Println(err)
		return
	}
	if *commit > 0 {
		node.SetCommitted(*commit)
	}
	handler.nodeNumber = node.callbacksNum
	node.callbackHandler = handler
	if *listen != "127.0.0.1:2021" {
		fmt.Println(node.AddPeer("127.0.0.1:2021"))
	}
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

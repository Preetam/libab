# libab

## Background

There are many ways to implement a failure detector. For example, you may choose to use pings
between pairs of nodes in a cluster or have one node broadcast heartbeats to monitor node
status. There are also different options to implement leader election, such as using a ring,
a chain, or some order determined by randomized timeouts (like in Raft).

This implementation of a failure detector uses an *optimal implementation of ◊S*, the weakest
class of failure detectors for solving consensus. This implementation is described in
[*"Efficient Algorithms to Implement Failure Detectors and Solve Consensus in Distributed Systems"* (PPT)]
(http://www.sc.ehu.es/acwlaalm/sdi/phd-slides.ppt).

![slide](https://cloud.githubusercontent.com/assets/379404/11109137/691dd8b6-88bb-11e5-9a57-bcf1ff42f63c.png)

All of the nodes in a cluster are organized into a chain ordered by ID. The node with the lowest
ID is at the head of the chain. Each node waits for a heartbeat from its *trusted* process or node.
If a heartbeat is not received by some timeout, the node is suspected to have failed. Any node
that suspects the failure of the node immediately before it in the chain assumes the role of
the new leader. The exception is the head of the chain, which always assumes the leadership role
when it is online.

Keep in mind that this is just the background and not a rigorous explanation of the implementation!
It's not completely implemented yet, so this document will be updated as things change.

## Dependencies and Building

You will need a compiler that supports C++14. Only Linux and OS X are supported at the moment,
but things should be fine on other BSDs.

This repository uses submodules. The following will fetch them if you haven't done so already:

```sh
$ git submodule update --init --recursive
```

You will also need CMake version 3.0 or higher.

### Building

```sh
$ mkdir build && cd build
$ cmake ..
$ make
```

## License

BSD (see LICENSE)

# Communication Library
A lightweight library for Packet-based P2P Communication

## Overview

<p align="center">
  <img src="./docs/overview.jpg" width="480" />
</p>

## Directory Structure
```
.
├── docs/
├── include/
│   └── inline
├── src/
├── test/
├── tools/
├── wrapper
│   └── README.md
├── CMakeLists.txt
├── LICENSE
├── README.md
└── setup.sh
```

* `docs`    : documentation
* `include` : libcomm headers
* `src`     : libcomm implementation
* `test`    : unit tests (native & python)
* `tool`    : scripts for generating data for unit testing
* `wrapper` : wrapper for libcomm

## Usage
* Endpoint initialization
  * `UdpPeer`
  ```
  std::unique_ptr<comm::P2P_Endpoint> pEndpoint =
      comm::IP_Endpoint::createUdpPeer(<Local Port>, <Peer/Remote IP Address>, <Peer/Remote Port>);
  ...
  ```

  * TCP Client (created TCP Client shall try to connect to the designated server)
  ```
  std::unique_ptr<comm::P2P_Endpoint> pEndpoint =
      comm::IP_Endpoint::createTcpClient(<Server IP Address>, <Server Port>);
  ...
  ```

  * TCP Server
  ```
  // Construct a TCP Server
  std::unique_ptr<comm::TcpServer> pTcpServer = comm::TcpServer::create(<Local Port>);
  ...

  // Waiting for client
  int errorCode = 0;
  std::unique_ptr<comm::P2P_Endpoint> pEndpoint = pTcpServer->waitForClient(errorCode, <timeout_ms>);
  if (0 != errorCode) {
      // Error handling
  } else if (!pEndpoint) {
      // Timeout handling
  } else {
      // Connected
  }
  ...
  ```

* Send/Receive data via endpoints
```
// Send a packet to Peer
pEndpoint->send(comm::Packet::create(<buffer address>, <buffer size>));
...

// Check Rx queue
std::deque<std::unique_ptr<comm::Packet>> pPackets;
if (pEndpoint->recvAll(pPackets)) {
    for (auto& pPacket : pPackets) {
        // do something
    }
}
...
```

## Compilation
* Ubuntu
```
rm -rf build-linux
mkdir build-linux
cd build-linux
cmake ..
make
```

* Windows (MinGW64)
```
rm -rf build-mingw
mkdir build-mingw
cd build-mingw
cmake -G "MinGW Makefiles" ..
cmake --build .
```

* Note
  * CMAKE Option `-DDEFINE_DEBUG=ON`: to enable debug log
  * CMAKE Option `-DDEFINE_PROFILING=ON`: to enable profiling
  * CMAKE Option `-DBUILD_TESTS=OFF`: to disable unit tests' compilation
  * CMAKE Option `-DDEFINE_USE_RAW_POINTER=ON`: to use Raw Pointers in unit tests

## Dependencies
* Common
  * `CMake` (tested v3.30.4, v4.0.2)
  * [Optional] `clang-format`

* Ubuntu
  * Package: `build-essential`

* Windows
  * `WinLibs` (tested `gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2`)

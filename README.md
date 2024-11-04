# Client-Server Application

This project is a simple client-server application implemented in C++. The server listens for incoming connections from clients and handles requests using multithreading. The communication protocol is based on custom-defined packets.

## Features

- Multithreaded server capable of handling multiple clients
- Custom packet protocol for communication
- CRC32 message verification
- Static linking of libraries

## Requirements

Before you start, make sure you have the following installed:

- **C++ Compiler**: GCC or Clang
- **CMake**: Version 3.10 or higher
- **pthread**: Typically included with the standard C library on Linux systems

## Installing Required Components

### On Ubuntu/Debian

You can install the necessary components using the following commands:

```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential cmake screen git
```

## Cloning the Repository
```bash
git clone https://github.com/Ternick/client-server.git
cd client-server
```

## Building
```bash
cd src
mkdir build && cd build && cmake .. && make
```

## Running
```bash
screen -S own_server
./server
```
### To detach from server screen [CTRL + A + D]
```bash
./client
screen -r
```

#pragma once
#include "packet/packet.h"

#include <fstream>
#include <thread>
#include <vector>
#include <string>

class Server
{
public:
	Server(const char* addr, uint16_t port) : addr_(addr), port_(port), sock_(-1) {
		if (!initialWrite2File()) {
			throw std::runtime_error("Server -> initialWrite2File -> failed");
		}
	};
	bool start();
	void stop();
	~Server();

private:
	const char* addr_;
	const uint16_t port_;
	int sock_;
	std::vector<std::thread> clientThreads_;
	std::fstream fs;

	void acceptLoop();
	void processPacket(const Packet& header, int clientSocket);
	void handleClient(int clientSocket);
	bool initialWrite2File();
	bool readFromFile(std::string& data);
};
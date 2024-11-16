#pragma once
#include "packet/packet.h"
#include "../iio_client/iio_client.h"

#include <thread>
#include <vector>
#include <string>

class Server
{
public:
	Server(const char* addr, uint16_t port, IIOClient* client) : addr_(addr), port_(port), sock_(-1), client_(client) {};
	bool start();
	void stop();
	~Server();

private:
	const char* addr_;
	const uint16_t port_;
	int sock_;
	std::vector<std::thread> clientThreads_;
	IIOClient* client_;

	void acceptLoop();
	void processPacket(const Packet& header, int clientSocket);
	void handleClient(int clientSocket);
};
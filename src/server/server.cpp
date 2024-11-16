#include "server.h"

#include <iostream>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

bool Server::start() {
	sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        std::cerr << "Server -> start[" << __LINE__ << "] -> " << std::strerror(errno) << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(addr_);
    serverAddr.sin_port = htons(port_);

    if (bind(sock_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Server -> start[" << __LINE__ << "] -> " << std::strerror(errno) << std::endl;
        return false;
    }

    if (listen(sock_, 5) < 0) {
		std::cerr << "Server -> start[" << __LINE__ << "] -> " << std::strerror(errno) << std::endl;
        return false;
    }

    try {
        if (client_->WriteParameter("voltage0", "raw", "33")) {
            std::cout << "voltage0 output raw value set to 33" << std::endl;
        }
    }
    catch (std::exception& ex) {
        std::cerr << "Server -> start[" << __LINE__ << "] -> " <<  ex.what() << std::endl;
    }

    std::cout << "Server listening on " << addr_ << ":" << port_ << std::endl;

    acceptLoop();
    return true;
}

void Server::acceptLoop() {
    while (true) {
        sockaddr_in serverAddr;
        socklen_t clientLen = sizeof(serverAddr);
        int clientSocket = accept(sock_, (struct sockaddr*)&serverAddr, &clientLen);

        if (clientSocket < 0) {
            std::cerr << "Connection aborted" << std::endl;
            continue;
        }

        std::cout << "New connection from "
            << inet_ntoa(serverAddr.sin_addr)
            << ":" << ntohs(serverAddr.sin_port) << std::endl;
        
        clientThreads_.emplace_back(&Server::handleClient, this, clientSocket);
    }
}

void Server::processPacket(const Packet& packet, int clientSocket) {
    switch (packet.pHeader.type)
    {
    case (PacketType::PING): {
        std::cout << "<<PING PONG>>" << std::endl;

        Packet pong(PacketType::PONG);
        std::vector<uint8_t> answerSerialized = pong.serialize();
        send(clientSocket, answerSerialized.data(), answerSerialized.size(), 0);
        break;
    }
    case (PacketType::COMMAND): {
        std::cout << "<<COMMAND RESPONSE>>" << std::endl;
        std::unique_ptr<char[]> data = std::make_unique<char[]>(32);

        Packet response = Packet(PacketType::ERROR, "Unable to reply on COMMAND");

        if (client_->ReadParameter("voltage0", "offset", data.get(), 32)) {
            response = Packet(PacketType::RESPONSE, "voltage0 offset: " + std::string(data.get()));
        }

        std::vector<uint8_t> answerSerialized = response.serialize();
        send(clientSocket, answerSerialized.data(), answerSerialized.size(), 0);
        break;
    }
    case (PacketType::OK): {
        std::cout << "<<OK: RESPONSE delivered and approved" << std::endl;
        break;
    }
    case (PacketType::ERROR):
        std::cout << "<<ERROR: " << packet.data.get() << std::endl;
        break;
    default:
        std::cerr << "Unknown packet type " << static_cast<uint16_t>(packet.pHeader.type) << std::endl;
        break;
    }
}

void Server::handleClient(int clientSocket) {
    while (true) {
        PacketHeader header;

        ssize_t headerSize = read(clientSocket, &header, sizeof(header));

        if (headerSize <= 0) {
            std::cout << "Client disconnected." << std::endl;
            close(clientSocket);
            break;
        } else if (headerSize != sizeof(PacketHeader)) {
            std::cerr << "Receiving packet header from server failed." << std::endl;
            continue;
        }

        header.type = static_cast<PacketType>(ntohs(static_cast<uint16_t>(header.type)));
        header.dataLen = ntohl(header.dataLen);

        uint32_t crc32Computed;
        std::unique_ptr<char[]> data = std::make_unique<char[]>(header.dataLen);

        if (header.dataLen) {
            ssize_t data_size = recv(clientSocket, data.get(), header.dataLen, 0);
            if (data_size != header.dataLen) {
                throw std::runtime_error("Receiving body of packet from server failed.");
            }

            ssize_t crc_size = recv(clientSocket, &crc32Computed, sizeof(crc32Computed), 0);
            if (crc_size != sizeof(crc32Computed)) {
                throw std::runtime_error("Receiving crc32 of data from server failed.");
            }
        }

        processPacket(
            Packet(header.type, data.get(), header.dataLen, crc32Computed),
            clientSocket
        );
    }
}

void Server::stop() {
    if (sock_ >= 0) {
        close(sock_);
        sock_ = -1;
    }
}

Server::~Server()
{
    stop();
}
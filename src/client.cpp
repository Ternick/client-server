#include "server/packet/packet.h"
#include "server/packet/crc32.h"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

Packet receivePacket(int sockfd) {
    PacketHeader header;

    ssize_t headerSize = recv(sockfd, &header, sizeof(PacketHeader), 0);
    if (headerSize != sizeof(PacketHeader)) {
        throw std::runtime_error("Receiving packet header from server failed.");
    }

    header.type = static_cast<PacketType>(ntohs(static_cast<uint16_t>(header.type)));
    header.dataLen = ntohl(header.dataLen);

    uint32_t crc32Supposed = 0;
    std::unique_ptr<char[]> data = std::make_unique<char[]>(header.dataLen);

    if (header.dataLen) {
        ssize_t data_size = recv(sockfd, data.get(), header.dataLen, 0);
        if (data_size != header.dataLen) {
            throw std::runtime_error("Receiving body of packet from server failed.");
        }

        ssize_t crcSize = recv(sockfd, &crc32Supposed, sizeof(crc32Supposed), 0);
        if (crcSize != sizeof(crc32Supposed)) {
            throw std::runtime_error("Receiving crc32 of data from server failed.");
        }

        crc32Supposed = ntohl(crc32Supposed);
    }

    return Packet(header.type, data.get(), header.dataLen, crc32Supposed);
}

void processPacket(const Packet& packet, int sockfd) {
    switch(packet.pHeader.type)
    {
    case (PacketType::PONG):
        std::cout << "<<PONG" << std::endl;
        break;
    case (PacketType::RESPONSE): {
        std::cout << "<<RESPONSE: " << packet.data.get() << std::endl;

        Packet answer(PacketType::ERROR, "CRC32 comparison failed");

        if (packet.getCRC32() == packet.crc32Supposed) {
            answer = Packet(PacketType::OK);
            std::cout << "OK>>" << std::endl;
        }
        else {
            std::cout << "ERROR>>" << std::endl;
        }
        
        std::vector<uint8_t> answerSerialized = answer.serialize();
        send(sockfd, answerSerialized.data(), answerSerialized.size(), 0);
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

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Server -> start[" << __LINE__ << "] -> " << std::strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(7335);

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error connecting to the server." << std::endl;
        close(sockfd);
        return 1;
    }

    Packet pingPacket(PacketType::PING);
    std::vector<uint8_t> serializedPacket = pingPacket.serialize();
    send(sockfd, serializedPacket.data(), serializedPacket.size(), 0);
    std::cout << "PING>>" << std::endl;

    try {
        Packet responsePacket = receivePacket(sockfd);
        processPacket(responsePacket, sockfd);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error while receiving packet: " << ex.what() << std::endl;
    }

    Packet packet(PacketType::COMMAND);
    serializedPacket = packet.serialize();
    send(sockfd, serializedPacket.data(), serializedPacket.size(), 0);
    std::cout << "COMMAND>>" << std::endl;

    try {
        Packet responsePacket = receivePacket(sockfd);
        processPacket(responsePacket, sockfd);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error while receiving packet: " << ex.what() << std::endl;
    }

    close(sockfd);
    return 0;
}
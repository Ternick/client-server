#pragma once
#include "crc32.h"

#include <memory>
#include <cstring>
#include <arpa/inet.h>
#include <vector>

enum class PacketType : uint16_t {
    PING = 1,
    PONG = 2,
    COMMAND = 3,
    RESPONSE = 4,
    ERROR = 5,
    OK = 6
};

#pragma pack(push, 1)
struct PacketHeader {
    PacketType type;
    uint32_t dataLen;
};
#pragma pack(pop)

class Packet {
public:
    PacketHeader pHeader;
    std::unique_ptr<char[]> data;
    uint32_t crc32Supposed;

    Packet(PacketType type, const std::string& message = "")
        : pHeader{ type, static_cast<uint32_t>(message.size()) } {
        data = std::make_unique<char[]>(pHeader.dataLen);
        std::memcpy(data.get(), message.c_str(), pHeader.dataLen);
        crc32Computed_ = crc32_.compute(reinterpret_cast<const uint8_t*>(data.get()), pHeader.dataLen);
    };

    Packet(PacketType type, const char* messageData, uint32_t length)
        : pHeader{ type, length } {
        data = std::make_unique<char[]>(pHeader.dataLen);
        std::memcpy(data.get(), messageData, pHeader.dataLen);
        crc32Computed_ = crc32_.compute(reinterpret_cast<const uint8_t*>(data.get()), pHeader.dataLen);
    };

    Packet(PacketType type, const char* messageData, uint32_t length, uint32_t crc32Supposed_)
        : pHeader{ type, length }, crc32Supposed(crc32Supposed_) {
        data = std::make_unique<char[]>(pHeader.dataLen);
        std::memcpy(data.get(), messageData, pHeader.dataLen);
        crc32Computed_ = crc32_.compute(reinterpret_cast<const uint8_t*>(data.get()), pHeader.dataLen);
    };

    uint32_t getCRC32() const {
        return crc32Computed_;
    };

    std::vector<uint8_t> serialize();
    static Packet deserialize(const std::vector<uint8_t>& buffer);
private:
    CRC32 crc32_;
    uint32_t crc32Computed_;
};
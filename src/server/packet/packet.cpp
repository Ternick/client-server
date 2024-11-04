#include "packet.h"
#include <iostream>

std::vector<uint8_t> Packet::serialize() {
    uint64_t extension = pHeader.dataLen > 0 ? pHeader.dataLen + sizeof(crc32Computed_) : 0;
    std::vector<uint8_t> buffer(sizeof(PacketHeader) + extension);

    uint16_t typeNet = htons(static_cast<uint16_t>(pHeader.type));
    uint32_t lenNet = htonl(pHeader.dataLen);
    uint32_t crc32ComputedNet = htonl(crc32Computed_);

    std::memcpy(buffer.data(), &typeNet, sizeof(typeNet));
    std::memcpy(buffer.data() + sizeof(typeNet), &lenNet, sizeof(lenNet));

    if (pHeader.dataLen) {
        std::memcpy(buffer.data() + sizeof(typeNet) + sizeof(lenNet), data.get(), pHeader.dataLen);
        std::memcpy(buffer.data() + sizeof(typeNet) + sizeof(lenNet) + pHeader.dataLen, &crc32ComputedNet, sizeof(crc32ComputedNet));
    }

    return buffer;
}

Packet Packet::deserialize(const std::vector<uint8_t>& buffer) {
    uint16_t typeNet;
    uint32_t lenNet;

    std::memcpy(&typeNet, buffer.data(), sizeof(typeNet));
    std::memcpy(&lenNet, buffer.data() + sizeof(typeNet), sizeof(lenNet));

    PacketType type = static_cast<PacketType>(ntohs(typeNet));
    uint32_t dataLen = ntohl(lenNet);

    if (dataLen > 0) {
        uint32_t crc32Supposed;
        std::memcpy(&crc32Supposed, buffer.data() + sizeof(typeNet) + sizeof(lenNet) + dataLen, sizeof(crc32Supposed));
        crc32Supposed = ntohl(crc32Supposed);

        return Packet(type, reinterpret_cast<const char*>(buffer.data() + sizeof(typeNet) + sizeof(lenNet)), dataLen, crc32Supposed);
    }

    return Packet(type, reinterpret_cast<const char*>(buffer.data() + sizeof(typeNet) + sizeof(lenNet)), dataLen);
}
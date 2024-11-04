#include "crc32.h"

const std::array<uint32_t, 256> CRC32::generateTable() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else {
                crc >>= 1;
            }
        }
        table[i] = crc;
    }
    return table;
}

const std::array<uint32_t, 256> CRC32::table = CRC32::generateTable();

uint32_t CRC32::compute(const uint8_t* data, size_t length) const {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        crc = (crc >> 8) ^ table[(crc & 0xFF) ^ byte];
    }
    return ~crc;
}
#pragma once
#include <array>

class CRC32 {
public:
    CRC32() = default;

    uint32_t compute(const uint8_t* data, size_t length) const;

private:
    static const std::array<uint32_t, 256> generateTable();
    static const std::array<uint32_t, 256> table;
};
#ifndef EVOLUTION_LFSR_H
#define EVOLUTION_LFSR_H

#include <cstdint>

class LFSR {
private:
    uint64_t lfsr_register;

    uint8_t step() {
        /*
         * Taps at [64, 63, 61, 60] taken from
         * https://web.archive.org/web/20161007061934/http://courses.cse.tamu.edu/csce680/walker/lfsr_table.pdf
         * to get a MAXIMUM SIZE cycle (2^64 - 1, zero excluded)
         */
        uint64_t bit = (lfsr_register ^ (lfsr_register >> 1u) ^ (lfsr_register >> 3u) ^ (lfsr_register >> 4u)) << 63u;
        lfsr_register = (lfsr_register >> 1u) | bit;
        return (bit != 0) ? 0x1u : 0x0u;
    }

public:
    LFSR() : lfsr_register(0x1234567812345678u) {}

    explicit LFSR(uint64_t lfsr) : lfsr_register(lfsr) {}

    explicit LFSR(uint32_t seed) {
        lfsr_register = static_cast<uint64_t>(seed);
    }

    void seed(uint32_t s) {
        lfsr_register = static_cast<uint64_t>(s);
    }

    uint64_t getLfsrRegister() {
        return lfsr_register;
    }

    uint32_t getNextInt() {
        uint32_t result = 0x0u;
        for (uint8_t i = 0; i < 32; i++)
            result |= static_cast<uint32_t>(step()) << i;
        return result;
    }

    uint32_t getNextIntBetween(uint32_t from, uint32_t to) {
        return getNextInt() % (to - from) + from;
    }

    float getNextFloatBetween(float from, float to) {
        // Inspired by https://stackoverflow.com/a/686373
        return from + static_cast<float>(getNextInt()) / static_cast<float>(UINT32_MAX / (to - from));
    }

};

#endif //EVOLUTION_LFSR_H

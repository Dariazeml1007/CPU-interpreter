#pragma once
#include <vector>
#include <memory>
#include <cstdint>    //  для uint8_t, uint32_t
#include <algorithm>  //  для std::copy_n
#include <stdexcept>  //  для std::out_of_range

class Memory
{
private:
    std::vector<uint8_t> data;

    void readBlock(uint8_t* dest, size_t size, uint32_t offset) const
    {
        if (offset + size > data.size()) throw std::out_of_range("Memory overflow");
        std::copy_n(data.begin() + offset, size, dest);
    }

    void writeBlock(const uint8_t* src, size_t size, uint32_t offset)
    {
        if (offset + size > data.size()) throw std::out_of_range("Memory overflow");
        std::copy_n(src, size, data.begin() + offset);
    }

public:
    Memory(size_t size_in_bytes) : data(size_in_bytes, 0) {}

    template<typename T>
    T read(uint32_t addr) const
    {
        T result;
        readBlock(reinterpret_cast<uint8_t*>(&result), sizeof(T), addr);
        return result;
    }

    template<typename T>
    void write(uint32_t addr, T value)
    {
        writeBlock(reinterpret_cast<const uint8_t*>(&value), sizeof(T), addr);
    }

    size_t size() const { return data.size(); }
};

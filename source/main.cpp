#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <stdexcept>

#include "memory.hpp"
#include "cpu.hpp"

void load_binary_file(Memory& memory, CPU& cpu, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    uint32_t address = 0x00001000;
    uint32_t instruction;

    std::cout << "=== BINARY FILE LOADING ===" << std::endl;

    while (file.read(reinterpret_cast<char*>(&instruction), 4))
    {
        memory.write<uint32_t>(address, instruction);
        address += 4;
    }

    cpu.set_pc(0x00001000);
    std::cout << "Loaded " << (address - 0x1000) / 4 << " instructions from " << filename << std::endl;
    std::cout << "=== LOADING COMPLETE ===" << std::endl;
}

void print_registers(CPU& cpu)
{
    std::cout << "Registers state:" << std::endl;
    for (int i = 0; i < 8; i++)
    {
        if (cpu.get_register(i) != 0)
        {
            std::cout << "r" << i << ": 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << cpu.get_register(i) << " (" << std::dec << cpu.get_register(i) << ")" << std::endl;
        }
    }
    std::cout << "PC: 0x" << std::hex << cpu.get_pc() << std::endl;
    std::cout << "------------------------" << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "CPU Emulator\n";

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <binary_file>\n";
        return 1;
    }

    Memory memory(64 * 1024);
    CPU cpu;

    load_binary_file(memory, cpu, argv[1]);
    cpu.run(memory);

    return 0;
}

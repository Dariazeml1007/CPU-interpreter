#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <stdexcept>
#include "CLI11.hpp"

#include "machine.hpp"

void load_binary_file(Machine& machine, const std::string& filename, uint32_t load_address = 0x1000)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    uint32_t address = load_address;
    uint32_t instruction;

    std::cout << "=== BINARY FILE LOADING ===" << std::endl;

    while (file.read(reinterpret_cast<char*>(&instruction), 4)) {
        machine.get_memory().write<uint32_t>(address, instruction);
        address += 4;
    }

    machine.set_start_address(load_address);
    std::cout << "Loaded " << (address - load_address) / 4 << " instructions from " << filename << std::endl;
    std::cout << "=== LOADING COMPLETE ===" << std::endl;
}

void print_registers(Machine& machine)
{
    std::cout << "Registers state:" << std::endl;
    for (int i = 0; i < 8; i++) {
        if (machine.get_cpu().get_register(i) != 0) {
            std::cout << "r" << i << ": 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << machine.get_cpu().get_register(i) << " (" << std::dec
                      << machine.get_cpu().get_register(i) << ")" << std::endl;
        }
    }
    std::cout << "PC: 0x" << std::hex << machine.get_pc() << std::endl;
    std::cout << "------------------------" << std::endl;
}

int main(int argc, char** argv)
{

    CLI::App app{"CPU Emulator with Machine class"};


    std::string binary_file;
    bool verbose = false;
    uint32_t load_address = 0x1000;


    app.add_option("binary_file", binary_file, "Binary file to execute")
        ->required()
        ->check(CLI::ExistingFile);

    app.add_flag("-v,--verbose", verbose, "Enable verbose output");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }


    if (verbose)
    {
        std::cout << "CPU Emulator starting..." << std::endl;
        std::cout << "Binary file: " << binary_file << std::endl;
        std::cout << "Load address: 0x" << std::hex << load_address << std::endl;

    }

    Machine machine;

    load_binary_file(machine, binary_file, load_address);


    machine.run();


    return 0;
}

#pragma once
#include "memory.hpp"
#include "cpu.hpp"

class Machine
{
private:
    Memory memory;
    CPU cpu;

    static constexpr size_t DEFAULT_SIZE = 64 * 1024;
public:
    Machine(size_t memory_size = DEFAULT_SIZE) : memory(memory_size), cpu() {}

    Memory& get_memory() { return memory; }
    CPU& get_cpu() { return cpu; }
    const Memory& get_memory() const { return memory; }
    const CPU& get_cpu() const { return cpu; }

    void run()
    {
        cpu.run(memory);
    }

    void step()
    {
        cpu.step(memory);
    }

    void reset()
    {
        cpu.reset();
    }

    void set_start_address(uint32_t address)
    {
        cpu.set_pc(address);
    }

    bool is_halted() const {

        return cpu.is_halted();
    }

    uint32_t get_pc() const { return cpu.get_pc(); }

};

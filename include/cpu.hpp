#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <iostream>

class Memory
{
private:
    std::vector<uint8_t> data;

public:
    Memory(size_t size_in_bytes) : data(size_in_bytes, 0) {}

    void write32(uint32_t address, uint32_t value)
    {
    // Little-endian запись
    write8(address,     (value >> 0) & 0xFF);
    write8(address + 1, (value >> 8) & 0xFF);
    write8(address + 2, (value >> 16) & 0xFF);
    write8(address + 3, (value >> 24) & 0xFF);
    }

uint32_t read32(uint32_t address)
{
    // Little-endian чтение
    return (read8(address) << 0) |
           (read8(address + 1) << 8) |
           (read8(address + 2) << 16) |
           (read8(address + 3) << 24);
}
    uint8_t read8(uint32_t address) const { return data[address]; }
    void write8(uint32_t address, uint8_t value) { data[address] = value; }

    size_t size() const { return data.size(); }
};

class CPU
{
private:
    std::array<uint32_t, 32> gpr;
    uint32_t pc;
    std::unique_ptr<Memory> memory;
    bool should_halt = false;

public:

    CPU(size_t memory_size = 64 * 1024) : pc(0), memory(std::make_unique<Memory>(memory_size))
    {
        reset();
    }

    void reset()
    {
        pc = 0;
        std::fill(gpr.begin(), gpr.end(), 0);
    }


    uint32_t get_register(uint8_t index) const
    {
        return gpr[index];
    }

    void set_register(uint8_t index, uint32_t value)
    {
        if (index != 0)
        {
            gpr[index] = value;
        }
    }

    uint32_t get_pc() const { return pc; }
    void set_pc(uint32_t value) { pc = value; }

    Memory* get_memory() { return memory.get(); }
    const Memory* get_memory() const { return memory.get(); }


    uint32_t get_sp() const { return gpr[2]; }
    uint32_t get_ra() const { return gpr[1]; }
    uint32_t get_a0() const { return gpr[10]; }


    void step()
    {
       uint32_t current_pc = pc;
       uint32_t instr = memory->read32(current_pc);

       execute_instruction( instr);


       if (pc == current_pc)
       {
           pc = current_pc + 4;
       }
    }


    void run(size_t max_steps = 1000)
    {
          std::cout << "Starting execution, max steps: " << max_steps << std::endl;

          for (size_t i = 0; i < max_steps; i++)
          {
              if (should_halt)
              {
                  std::cout << "Program halted normally" << std::endl;
                  return;
              }

              if (pc >= memory->size())
              {
                  std::cout << "PC out of memory bounds" << std::endl;
                  return;
              }

              step();
          }

          std::cout << "Step limit reached (" << max_steps << "), stopping" << std::endl;
      }

private:

    static void execute_LD     (CPU& cpu, uint32_t instr);
    static void execute_ST     (CPU& cpu, uint32_t instr);
    static void execute_STP    (CPU& cpu, uint32_t instr);
    static void execute_BNE    (CPU& cpu, uint32_t instr);
    static void execute_BEQ    (CPU& cpu, uint32_t instr);
    static void execute_ADD    (CPU& cpu, uint32_t instr);
    static void execute_SUB    (CPU& cpu, uint32_t instr);
    static void execute_ADDI   (CPU& cpu, uint32_t instr);
    static void execute_SBIT   (CPU& cpu, uint32_t instr);
    static void execute_SSAT   (CPU& cpu, uint32_t instr);
    static void execute_CLS    (CPU& cpu, uint32_t instr);
    static void execute_BEXT   (CPU& cpu, uint32_t instr);
    static void execute_J      (CPU& cpu, uint32_t instr);
    static void execute_SYSCALL(CPU& cpu, uint32_t instr);



    void execute_instruction(uint32_t instr)
    {
        uint8_t opcode = (instr >> 26) & 0x3F;
        uint8_t funct = instr & 0x3F;


        if (opcode == 0b000000)
        {
           switch (funct)
           {
                case 0b001010:
                    std::cout << "Executing CLS" << std::endl;
                    execute_CLS(*this, instr);
                    break;
                case 0b010010:
                    std::cout << "Executing ADD" << std::endl;
                    execute_ADD(*this, instr);
                    break;
                case 0b010100:
                    std::cout << "Executing BEXT" << std::endl;
                    execute_BEXT(*this, instr);
                    break;
                case 0b101000:
                    std::cout << "Executing SYSCALL" << std::endl;
                    execute_SYSCALL(*this, instr);
                    break;
                case 0b110110:
                    std::cout << "Executing SUB" << std::endl;
                    execute_SUB(*this, instr);
                    break;
                default:
                    std::cout << "Unknown R-format funct: 0x" << std::hex << (int)funct << std::endl;
            }
            return;
        }


        switch (opcode)
        {
            case 0b001101: execute_SSAT(*this, instr); break;    // SSAT
            case 0b010101: execute_STP(*this, instr); break;     // STP
            case 0b011000: execute_BNE(*this, instr); break;     // BNE
            case 0b011010: execute_BEQ(*this, instr); break;     // BEQ
            case 0b011100: execute_SBIT(*this, instr); break;    // SBIT
            case 0b011111: execute_J(*this, instr); break;       // J
            case 0b101101: execute_ADDI(*this, instr); break;    // ADDI
            case 0b110111: execute_ST(*this, instr); break;      // ST
            case 0b111001: execute_LD(*this, instr); break;      // LD
        //    default:
        //        std::cout << "Unknown instruction opcode: 0x" << std::hex << (int)opcode << std::endl;
        }
    }
};

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

    uint32_t read32(uint32_t address) const
    {
        // TODO: добавить проверку границ
        return *reinterpret_cast<const uint32_t*>(&data[address]);
    }

    void write32(uint32_t address, uint32_t value)
    {
        // TODO: добавить проверку границ
        *reinterpret_cast<uint32_t*>(&data[address]) = value;
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

    using InstructionHandler = void(*)(CPU& cpu, uint32_t instruction);
    struct InstructionInfo
    {
        const char         *name;
        const char         *format;
        InstructionHandler handler;

    };

    InstructionInfo instruction_table[64] = {};

    void initialize_instruction_table()
    {
    instruction_table[0b000000] = {"ADD", "rd, rs, rt", &CPU::execute_ADD};
    instruction_table[0b000001] = {"SUB", "rd, rs, rt", &CPU::execute_SUB};
    instruction_table[0b000010] = {"BEXT", "rd, rs1, rs2", &CPU::execute_BEXT};
    instruction_table[0b000011] = {"CLS", "rd, rs", &CPU::execute_CLS};
    instruction_table[0b000100] = {"SYSCALL", "", &CPU::execute_SYSCALL};
    instruction_table[0b001101] = {"SSAT", "rd, rs, imm5", &CPU::execute_SSAT};
    instruction_table[0b010101] = {"STP", "rt1, rt2, offset(base)", &CPU::execute_STP};
    instruction_table[0b011000] = {"BNE", "rs, rt, offset", &CPU::execute_BNE};
    instruction_table[0b011010] = {"BEQ", "rs, rt, offset", &CPU::execute_BEQ};
    instruction_table[0b011100] = {"SBIT", "rd, rs, imm5", &CPU::execute_SBIT};
    instruction_table[0b011111] = {"J", "target", &CPU::execute_J};
    instruction_table[0b101101] = {"ADDI", "rt, rs, imm", &CPU::execute_ADDI};
    instruction_table[0b110111] = {"ST", "rt, offset(base)", &CPU::execute_ST};
    instruction_table[0b111001] = {"LD", "rt, offset(base)", &CPU::execute_LD};
    }


public:
    // Конструктор с размером памяти (по умолчанию 64KB)
    CPU(size_t memory_size = 64 * 1024) : pc(0), memory(std::make_unique<Memory>(memory_size))
    {
        initialize_instruction_table();  // ← Добавь этот вызов
        reset();
    }

    void reset()
    {
        pc = 0;
        std::fill(gpr.begin(), gpr.end(), 0);
    }

    // Регистры
    uint32_t get_register(uint8_t index) const
    {
        return gpr[index];
    }

    void set_register(uint8_t index, uint32_t value)
    {
        if (index != 0)
        {  // x0 ===== 0!
            gpr[index] = value;
        }
    }

    // Program Counter
    uint32_t get_pc() const { return pc; }
    void set_pc(uint32_t value) { pc = value; }

    // Память
    Memory* get_memory() { return memory.get(); }
    const Memory* get_memory() const { return memory.get(); }

    // Специальные регистры (для отладки)
    uint32_t get_sp() const { return gpr[2]; }
    uint32_t get_ra() const { return gpr[1]; }
    uint32_t get_a0() const { return gpr[10]; }

    // Выполнение
    void step()
    {
        uint32_t instruction = memory->read32(pc);
        pc += 4;
        execute_instruction(instruction);
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

    uint8_t decode_opcode(uint32_t instruction) const
    {
        return (instruction >> 26) & 0x3F;
    }

    void execute_instruction(uint32_t instr)
    {
        uint8_t opcode = decode_opcode(instr);
        if (instruction_table[opcode].handler)
        {
            instruction_table[opcode].handler(*this, instr);
        }
        else
        {
            std::cout << "Unknown instruction: 0x" << std::hex << opcode << std::endl;
        }
    }
};

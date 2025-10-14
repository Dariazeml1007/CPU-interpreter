#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <iostream>

#include "memory.hpp"

class CPU
{
private:
    std::array<uint32_t, 32> gpr;
    uint32_t pc;
    bool should_halt = false;
    bool branch_flag = false;

public:

     CPU() : pc(0) {
        reset();
    }

    void reset() {
        pc = 0;
        std::fill(gpr.begin(), gpr.end(), 0);
        should_halt = false;
    }

    void step(Memory& memory)
    {

        uint32_t raw_instr = memory.read<uint32_t>(pc);
        Instruction instr_obj(raw_instr);
        branch_flag = false;
        execute_instruction(instr_obj, memory);

        if (!branch_flag)
        {
            pc += 4;
        }
    }

    void run(Memory& memory, size_t max_steps = 1000)
    {
        std::cout << "Starting execution, max steps: " << max_steps << std::endl;

        for (size_t i = 0; i < max_steps; i++)
        {
            if (should_halt)
            {
                std::cout << "Program halted normally" << std::endl;
                return;
            }

            step(memory);
        }

        std::cout << "Step limit reached (" << max_steps << "), stopping" << std::endl;
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

private:

    static int32_t sign_extend(uint32_t value, uint8_t bits)
    {
    int32_t x = static_cast<int32_t>(value);
    int32_t mask = 1u << (bits - 1);
    return (x ^ mask) - mask;
    }

    struct Instruction
    {
        uint8_t opcode;
        uint8_t funct;

        uint8_t rs1, rs2, rd, rt;
        int32_t imm;
        uint32_t target, offset;

        Instruction(uint32_t raw);
    };


    enum Opcode : uint8_t
    {
        OP_R_FORMAT = 0b000000,
        OP_SSAT     = 0b001101,
        OP_STP      = 0b010101,
        OP_BNE      = 0b011000,
        OP_BEQ      = 0b011010,
        OP_SBIT     = 0b011100,
        OP_J        = 0b011111,
        OP_ADDI     = 0b101101,
        OP_ST       = 0b110111,
        OP_LD       = 0b111001
    };

    enum Funct : uint8_t
    {
        F_CLS     = 0b001010,
        F_ADD     = 0b010010,
        F_BEXT    = 0b010100,
        F_SYSCALL = 0b101000,
        F_SUB     = 0b110110
    };

    static void execute_LD     (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_ST     (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_STP    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_BNE    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_BEQ    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_ADD    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_SUB    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_ADDI   (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_SBIT   (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_SSAT   (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_CLS    (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_BEXT   (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_J      (CPU& cpu, Instruction instr, Memory& memory);
    static void execute_SYSCALL(CPU& cpu, Instruction instr, Memory& memory);



   void execute_instruction( Instruction instr_obj, Memory& memory)
    {

        uint8_t opcode = instr_obj.opcode;
        uint8_t funct = instr_obj.funct;

        if (opcode == OP_R_FORMAT)
        {
            switch (funct)
            {
                case F_CLS:     execute_CLS(*this, instr_obj, memory);     break;
                case F_ADD:     execute_ADD(*this, instr_obj, memory);     break;
                case F_BEXT:    execute_BEXT(*this, instr_obj, memory);    break;
                case F_SYSCALL: execute_SYSCALL(*this, instr_obj, memory); break;
                case F_SUB:     execute_SUB(*this, instr_obj, memory);     break;
                default:
                    std::cerr << "Unknown R-format funct: 0x" << std::hex << (int)funct << std::endl;
            }
            return;
        }

        switch (opcode)
        {
            case OP_SSAT: execute_SSAT(*this, instr_obj, memory);   break;
            case OP_STP:  execute_STP(*this, instr_obj, memory);    break;
            case OP_BNE:  execute_BNE(*this, instr_obj, memory);    break;
            case OP_BEQ:  execute_BEQ(*this, instr_obj, memory);    break;
            case OP_SBIT: execute_SBIT(*this, instr_obj, memory);   break;
            case OP_J:    execute_J(*this, instr_obj, memory);      break;
            case OP_ADDI: execute_ADDI(*this, instr_obj, memory);   break;
            case OP_ST:   execute_ST(*this, instr_obj, memory);     break;
            case OP_LD:   execute_LD(*this, instr_obj, memory);     break;
            default:
                std::cerr << "Unknown instruction opcode: 0x" << std::hex << (int)opcode << std::endl;
        }
    }

};


inline CPU::Instruction::Instruction(uint32_t raw)
{
    opcode = (raw >> 26) & 0x3F;                        // [31:26]
    funct  = raw & 0x3F;                                // [5:0]

    rd = (raw >> 21) & 0x1F;                            // [25:21]
    rs1 = (raw >> 16) & 0x1F;                           // [20:16]
    rs2  = (raw >> 11) & 0x1F;                          // [15:11]
    rt  = rs1;

    imm =  sign_extend(raw & 0xFFFF, 16);              // [15:0], sign-extended

    target = raw & 0x3FFFFFF;                           // [25:0] для J
    offset = sign_extend(raw & 0x7FF, 11);              // [0:10] stp
}

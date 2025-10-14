#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <iostream>

#include "cpu.hpp"
#include "memory.hpp"

enum Syscalls_
{
    SYS_EXIT        = 0,
    SYS_PRINT_INT   = 1,
    SYS_PRINT_STR   = 2,
    SYS_READ_INT    = 3,
    SYS_READ_STR    = 4,
};

void CPU::execute_LD(CPU& cpu, Instruction instr, Memory& memory)
{
    uint8_t base  = instr.rd;
    uint8_t rt = instr.rs1;
    int16_t offset = instr.imm;

    uint32_t address = cpu.gpr[base] + offset;

    cpu.gpr[rt] = memory.read<uint32_t>(address);
}

void CPU::execute_CLS(CPU& cpu, Instruction instr)
{
    uint8_t rd = instr.rd;
    uint8_t rs = instr.rs1;

    uint32_t value = cpu.gpr[rs];
    uint32_t sign_bit = (value >> 31) & 1;

    int count = 0;
    for (int i = 31; i >= 0; i--)
    {
        bool bit = (value >> i) & 1;

        if (bit == sign_bit)
        {
            count++;

        }
        else
        {

            break;
        }

    }

    cpu.gpr[rd] = count;

}

void CPU::execute_SYSCALL(CPU& cpu)
{
    uint32_t syscall_num = cpu.gpr[8];

    switch(syscall_num)
    {
        case SYS_EXIT:

            cpu.should_halt = true;
            break;

        case SYS_PRINT_INT:
            std::cout << "=========================================================\n";
            std::cout << "Output: " << std::dec << cpu.gpr[3] << std::endl;
             std::cout << "=========================================================\n";
            cpu.gpr[0] = 0;
            break;


       case SYS_READ_INT:

            std::cout << "Input: ";
            std::cin >> cpu.gpr[3];
            std::cout << std::endl;

            break;

        default:
            std::cerr << "Unknown syscall: " << syscall_num << std::endl;
            cpu.should_halt = true;
            break;
    }
}

void CPU::execute_BNE(CPU& cpu, Instruction instr)
{
    uint8_t rs = instr.rd;
    uint8_t rt = instr.rt;
    int16_t offset = (instr.imm);


    if (cpu.gpr[rs] != cpu.gpr[rt])
    {
        int32_t target = (offset) << 2;
        cpu.pc += target;
        cpu.branch_flag = true;
    }
}

void CPU::execute_BEQ(CPU& cpu, Instruction instr)
{
    uint8_t rs  = instr.rd ;
    uint8_t rt    = instr.rt;
    int16_t offset = instr.imm;

    if (cpu.gpr[rs] == cpu.gpr[rt])
    {
        int32_t target = (offset) << 2;
        cpu.pc += target;
        cpu.branch_flag = true;
    }

}

void CPU::execute_SBIT(CPU& cpu, Instruction instr)
{
    uint8_t rd = instr.rd;
    uint8_t imm5 = instr.rs2;

    cpu.gpr[rd] = (1 << imm5);
}
void CPU::execute_BEXT(CPU& cpu, Instruction instr)
{
    uint8_t rd = instr.rd;
    uint8_t rs1 = instr.rs1;
    uint8_t rs2 = instr.rs2;

    uint32_t result = 0;
    uint32_t mask = cpu.gpr[rs2];

    for (int i = 0, count = 0; i < 32; i++)
    {
        if (mask & (1u << i))
        {
            bool bit = (cpu.gpr[rs1] & (1u << i));

            if (bit)
            {
                result |= (1u << count);
            }
            count++;
        }
    }

    cpu.gpr[rd] = result;
}

void CPU::execute_SUB(CPU& cpu, Instruction instr)
{
    uint8_t rs = instr.rd;
    uint8_t rt = instr.rs1;
    uint8_t rd = instr.rs2;

    cpu.gpr[rd] = cpu.gpr[rs] - cpu.gpr[rt];

}

void CPU::execute_ADDI(CPU& cpu, Instruction instr)
{
    uint8_t rs = instr.rd;
    uint8_t rt = instr.rs1;
    int32_t imm = (instr.imm);

    cpu.gpr[rt] = cpu.gpr[rs] + imm;
}

void CPU::execute_ADD(CPU& cpu, Instruction instr)
{
    uint8_t rs = instr.rd;
    uint8_t rt = instr.rs1;
    uint8_t rd = instr.rs2;

    cpu.gpr[rd] = cpu.gpr[rs] + cpu.gpr[rt];

}

void CPU::execute_J(CPU& cpu, Instruction instr)
{
    uint32_t index = instr.target;
    uint32_t base = cpu.pc & 0xFFFFF000;
    uint32_t offset = index << 2;

    cpu.pc = base | offset;
    cpu.branch_flag = true;

}

void CPU::execute_SSAT(CPU& cpu, Instruction instr)
{
    uint8_t rd = instr.rd;
    uint8_t rs =  instr.rs1;
    uint8_t imm5 = instr.rs2;

    int32_t value = static_cast<int32_t>(cpu.gpr[rs]);

    int32_t result;
    if (imm5 == 0 || imm5 > 31)
    {
        result = 0;


    } else
    {
        int32_t max_positive = (1 << (imm5 - 1)) - 1;
        int32_t min_negative = -(1 << (imm5 - 1));

        if (value > max_positive)
        {
            result = max_positive;


        } else if (value < min_negative)
        {
            result = min_negative;

        } else
        {
            result = value;
        }
    }

    cpu.gpr[rd] = static_cast<uint32_t>(result);
}

void CPU::execute_ST(CPU& cpu, Instruction instr, Memory& memory)
{
    uint8_t base = instr.rd;
    uint8_t rt = instr.rs1;
    int16_t offset = instr.imm;

    int32_t sign_extended_offset = static_cast<int32_t>(offset);
    uint32_t address = cpu.gpr[base] + sign_extended_offset;

    if ((address & 0x3) != 0)
    {
        return;
    }
    memory.write<uint32_t>(address, cpu.gpr[rt]);
}

void CPU::execute_STP(CPU& cpu, Instruction instr, Memory& memory)
{
    uint8_t base = instr.rd;
    uint8_t rt1 = instr.rs1;
    uint8_t rt2 = instr.rs2;
    int16_t offset = instr.offset;

    uint32_t addr = cpu.gpr[base] + offset;

    memory.write<uint32_t>(addr, cpu.gpr[rt1]);
    memory.write<uint32_t>(addr + 4, cpu.gpr[rt2]);

}

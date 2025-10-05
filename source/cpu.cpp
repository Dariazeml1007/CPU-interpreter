#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <iostream>

#include "cpu.hpp"

enum Syscalls_
{
    SYS_EXIT        = 0,   // завершение программы
    SYS_PRINT_INT   = 1,  // вывод числа
    SYS_PRINT_STR   = 2,  // вывод строки
    SYS_READ_INT    = 3,   // чтение числа
    SYS_READ_STR    = 4,   // чтение строки
};

void CPU::execute_LD(CPU& cpu, uint32_t instr)
{
    uint8_t base  = (instr >> 21) & 0x1F;
    uint8_t rt    = (instr >> 16) & 0x1F;
    int16_t offset = instr & 0xFFFF;

    uint32_t address = cpu.gpr[base] + offset;

    cpu.gpr[rt] = cpu.memory->read32(address);
}

void CPU::execute_CLS(CPU& cpu, uint32_t instr)
{
    uint8_t rd = (instr >> 21) & 0x1F;
    uint8_t rs = (instr >> 16) & 0x1F;

    uint32_t value = cpu.gpr[rs];
    uint32_t sign_bit = (value >> 31) & 1;  // знаковый бит

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

void CPU::execute_SYSCALL(CPU& cpu, uint32_t instr)
{
    uint32_t syscall_num = cpu.gpr[8];

    switch(syscall_num)
    {
        case SYS_EXIT:

            cpu.should_halt = true;
            cpu.pc = 0;
            break;

        case SYS_PRINT_INT:

            std::cout << "Output: " << std::dec << cpu.gpr[0] << std::endl;
            cpu.gpr[0] = 0;
            break;


       case SYS_READ_INT:
            std::cout << "Input: ";
            std::cin >> cpu.gpr[0];
            std::cout << std::endl;

            break;

        default:
            break;
    }
}

void CPU::execute_BNE(CPU& cpu, uint32_t instr)
{
    uint8_t rs  = (instr >> 21) & 0x1F;
    uint8_t rt    = (instr >> 16) & 0x1F;
    int16_t offset = instr & 0xFFFF;

    int32_t target = static_cast<int32_t>(offset) << 2;
    if (cpu.gpr[rs] != cpu.gpr[rt])
        cpu.pc += target;

}

void CPU::execute_BEQ(CPU& cpu, uint32_t instr)
{
    uint8_t rs  = (instr >> 21) & 0x1F;
    uint8_t rt    = (instr >> 16) & 0x1F;
    int16_t offset = instr & 0xFFFF;

    int32_t target = static_cast<int32_t>(offset) << 2;
    if (cpu.gpr[rs] == cpu.gpr[rt])
        cpu.pc += target;

}

void CPU::execute_SBIT(CPU& cpu, uint32_t instr)
{
    uint8_t rd = (instr >> 21) & 0x1F;
    uint8_t rs = (instr >> 16) & 0x1F;
    uint8_t imm5 = (instr >> 11) & 0x1F;


    cpu.gpr[rd] = (1 << imm5);
}
void CPU::execute_BEXT(CPU& cpu, uint32_t instr)
{
    uint8_t rd = (instr >> 21) & 0x1F;
    uint8_t rs1 = (instr >> 16) & 0x1F;
    uint8_t rs2 = (instr >> 11) & 0x1F;

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

void CPU::execute_SUB(CPU& cpu, uint32_t instr)
{
    uint8_t rs = (instr >> 21) & 0x1F;
    uint8_t rt = (instr >> 16) & 0x1F;
    uint8_t rd = (instr >> 11) & 0x1F;

    cpu.gpr[rd] = cpu.gpr[rs] - cpu.gpr[rt];

}

void CPU::execute_ADDI(CPU& cpu, uint32_t instr)
{
    uint8_t rs = (instr >> 21) & 0x1F;
    uint8_t rt = (instr >> 16) & 0x1F;
    int32_t imm = static_cast<int16_t>(instr & 0xFFFF);

    cpu.gpr[rt] = cpu.gpr[rs] + imm;
}

void CPU::execute_ADD(CPU& cpu, uint32_t instr)
{
    uint8_t rs = (instr >> 21) & 0x1F;
    uint8_t rt = (instr >> 16) & 0x1F;
    uint8_t rd = (instr >> 11) & 0x1F;

    cpu.gpr[rd] = cpu.gpr[rs] + cpu.gpr[rt];

}

void CPU::execute_J(CPU& cpu, uint32_t instr)
{
    uint32_t index = instr & 0x3FFFFFF;
    uint32_t return_addr = cpu.pc - 4;
    cpu.pc = return_addr + (index << 2);
}

void CPU::execute_SSAT(CPU& cpu, uint32_t instr)
{
    uint8_t rd = (instr >> 21) & 0x1F;
    uint8_t rs = (instr >> 16) & 0x1F;
    uint8_t imm5 = (instr >> 11) & 0x1F;

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

void CPU::execute_ST(CPU& cpu, uint32_t instr)
{
    uint8_t base = (instr >> 21) & 0x1F;
    uint8_t rt = (instr >> 16) & 0x1F;
    int16_t offset = static_cast<int16_t>(instr & 0xFFFF);

    int32_t sign_extended_offset = static_cast<int32_t>(offset);
    uint32_t address = cpu.gpr[base] + sign_extended_offset;

    if ((address & 0x3) != 0)
    {
        return;
    }
    cpu.memory->write32(address, cpu.gpr[rt]);
}

void CPU::execute_STP(CPU& cpu, uint32_t instr)
{
    uint8_t base = (instr >> 21) & 0x1F;
    uint8_t rt1 = (instr >> 16) & 0x1F;
    uint8_t rt2 = (instr >> 11) & 0x1F;
    int16_t offset = static_cast<int16_t>(instr & 0x7FF);

    uint32_t addr = cpu.gpr[base] + offset;

    cpu.memory->write32(addr, cpu.gpr[rt1]);
    cpu.memory->write32(addr + 4, cpu.gpr[rt2]);
}

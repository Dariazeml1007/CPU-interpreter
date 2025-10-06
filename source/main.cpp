#include "cpu.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <stdexcept>

void tests();

void load_binary_file(CPU& cpu, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    // Начальный адрес кода — 0x1000
    uint32_t address = 0x00001000;

    uint32_t instruction;
    while (file.read(reinterpret_cast<char*>(&instruction), 4))
    {
        cpu.get_memory()->write32(address, instruction);
        address += 4;
    }

    // Устанавливаем PC на начало программы
    cpu.set_pc(0x00001000);

    std::cout << "Loaded " << (address - 0x1000) / 4 << " instructions from " << filename << std::endl;
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


void write_code_to_memory(const std::vector<uint32_t>& program, CPU& cpu)
{
    uint32_t code_start_address = 0x00001000;  // Код с адреса 0x1000
    uint32_t data_start_address = 0x00002000;  // Данные с адреса 0x2000
    uint32_t stack_start_address = 0x00003000; // Стек с адреса 0x3000


    for (size_t i = 0; i < program.size(); i++)
    {
        uint32_t address = code_start_address + i * 4;
        uint32_t instruction = program[i];

        cpu.get_memory()->write8(address,     (instruction >> 0)  & 0xFF);
        cpu.get_memory()->write8(address + 1, (instruction >> 8)  & 0xFF);
        cpu.get_memory()->write8(address + 2, (instruction >> 16) & 0xFF);
        cpu.get_memory()->write8(address + 3, (instruction >> 24) & 0xFF);
    }

    // Инициализируем данные нулями - ИСПРАВЛЕНО
    for (uint32_t addr = data_start_address; addr < data_start_address + 0x1000; addr += 4) {
        cpu.get_memory()->write32(addr, 0);  // ← используй get_memory()
    }

    // Инициализируем стек
    cpu.set_register(2, stack_start_address); // $sp = 0x3000

    // Устанавливаем PC на начало кода
    cpu.set_pc(code_start_address);
}


void test_(const std::vector<uint32_t>& program, int expected_result, const std::string& test_name)
{
    std::cout << "=== " << test_name << " ===" << std::endl;
    CPU cpu;

    write_code_to_memory(program, cpu);
    cpu.run(10);

    uint32_t result = cpu.get_register(3); // r3
    std::cout << "Result: r3 = " << result << std::endl;

    if (result == expected_result)
    {
        std::cout << "✓ TEST PASSED" << std::endl;
    }
    else
    {
        std::cout << "✗ TEST FAILED - Expected " << expected_result << ", got " << result << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}



int main()
{
    std::cout << "CPU Emulator Test Suite" << std::endl;
    std::cout << "=======================" << std::endl;

    #ifdef RUN_TESTS

        tests();
    #else
        CPU cpu(64 * 1024);  // 64 KB памяти

        // Загружаем программу из файла
        load_binary_file(cpu, "./ruby/output.bin");

        // Запускаем до 1000 шагов
        cpu.run(1000);
    #endif

    return 0;
}

void tests()
{
    // Тест 1: ADDI + ADD (базовая арифметика)
    test_(
        {
            UINT32_C(0b10110100000000010000000000000101), // ADDI r1, r0, 5
            UINT32_C(0b10110100000000100000000000000011), // ADDI r2, r0, 3
            UINT32_C(0b00000000001000100001100000010010), // ADD r3, r1, r2
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        8,
        "ADDI + ADD: 5 + 3 = 8"
    );

    // Тест 2: ADDI + SUB
    test_(
        {
            UINT32_C(0b10110100000000010000000000001000), // ADDI r1, r0, 8
            UINT32_C(0b10110100000000100000000000000011), // ADDI r2, r0, 3
            UINT32_C(0b00000000001000100001100000110110), // SUB r3, r1, r2
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        5,
        "ADDI + SUB: 8 - 3 = 5"
    );

    // Тест 3: SBIT
    test_(
        {
            UINT32_C(0b01110000011000000010100000000000), // SBIT r3, r0, 5
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        32,
        "SBIT: set bit 5 = 32"
    );

    // Тест 4: BEXT
    test_(
        {
            UINT32_C(0b10110100000000010000000011101010), // ADDI r1, r0, 0xEA
            UINT32_C(0b10110100000000100000000000110100), // ADDI r2, r0, 0x34
            UINT32_C(0b00000000011000010001000000010100), // BEXT r3, r1, r2
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        4,
        "BEXT: extract bits"
    );

    test_(
    {
        UINT32_C(0b10110100000000010000000011110000), // ADDI r1, r0, 0xF0
        UINT32_C(0b00000000011000010000000000001010), // CLS r3, r1
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    24,

    "CLS: count leading signs of 0xF0"
     );
    // Тест 6: SSAT
    test_(
        {
            UINT32_C(0b10110100000000010000000000011111), // ADDI r1, r0, 31
            UINT32_C(0b00110100011000010001100000000000), // SSAT r3, r1, 3
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        3,
        "SSAT: saturate 31 to 3 bits"
    );

    // Тест 7: LD
    test_(
        {
            UINT32_C(0b10110100000000010000000000010000), // ADDI r1, r0, 0x1000
            UINT32_C(0b11100100001000110000000000000000), // LD r3, 0(r1)
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        0,
        "LD: load from data segment"
    );

    // Тест 8: BEQ (branch if equal)
    test_(
        {
            UINT32_C(0b10110100000000010000000000000101), // ADDI r1, r0, 5
            UINT32_C(0b10110100000000100000000000000101), // ADDI r2, r0, 5
            UINT32_C(0b01101000001000100000000000000001), // BEQ r1, r2, 1
            UINT32_C(0b10110100000000110000000000000000), // ADDI r3, r0, 0 (skipped)
            UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, 15 (target)
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        15,
        "BEQ: branch if equal"
    );

    // Тест 9: BNE (branch if not equal)
    test_(
        {
            UINT32_C(0b10110100000000010000000000000101), // ADDI r1, r0, 5
            UINT32_C(0b10110100000000100000000000000011), // ADDI r2, r0, 3
            UINT32_C(0b01100000001000100000000000000001), // BNE r1, r2, 1
            UINT32_C(0b10110100000000110000000000000000), // ADDI r3, r0, 0 (skipped)
            UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, 15 (target)
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        15,
        "BNE: branch if not equal"
    );

   // Тест 11: J (jump)
test_(
    {
        UINT32_C(0b10110100000000110000000000000000), // ADDI r3, r0, #0   -> r3 = 0
        UINT32_C(0b01111100000000000000000000000010), // J 2               -> jump to addr = PC_base | (2 << 2) = +8 bytes
        UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, #15  -> skipped
        UINT32_C(0b10110100000000110000000000101010), // ADDI r3, r0, #42  -> executed, r3 = 42
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    42,
    "J: jump instruction"
);

    // Тест 11: STP (store pair)
    test_(
        {
            UINT32_C(0b10110100000000010000000000101010), // ADDI r1, r0, 0x2A
            UINT32_C(0b10110100000000100000000000011111), // ADDI r2, r0, 0x1F
            UINT32_C(0b10110100000001000000000000100000), // ADDI r4, r0, 0x20
            UINT32_C(0b01010100100000010001000000000000), // STP r1, r2, 0(r4)
            UINT32_C(0b11100100100000110000000000000000), // LD r3, 0(r4)
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        0x2A,
        "STP: store pair in data segment"
    );

    // Тест 13: SYSCALL
    test_(
        {
            UINT32_C(0b10110100000010000000000000000001), // ADDI r8, r0, 1 (PRINT_INT)
            UINT32_C(0b10110100000000110000000000000111), // ADDI r3, r0, 7 (continue)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        7,
        "SYSCALL: system call execution"
    );
}

#include "../include/cpu.hpp"
#include "../include/memory.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

void write_code_to_memory(const std::vector<uint32_t>& program, Memory& memory, CPU& cpu)
{
    uint32_t code_start_address = 0x00001000;

    for (size_t i = 0; i < program.size(); i++)
    {
        uint32_t address = code_start_address + i * 4;
        memory.write<uint32_t>(address, program[i]);
    }

    cpu.set_pc(code_start_address);
}


void test_(const std::vector<uint32_t>& program, uint32_t expected_result, const std::string& test_name)
{
    std::cout << "=== " << test_name << " ===" << std::endl;
    Memory memory(64 * 1024);
    CPU cpu;

    write_code_to_memory(program, memory, cpu);
    cpu.run(memory);

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

   test_(
    {
        // Подготовка данных
        UINT32_C(0b10110100000000010000000000100000), // ADDI r1, r0, 0x20 (32) - адрес
        UINT32_C(0b10110100000000100000000000101010), // ADDI r2, r0, 0x2A (42) - значение
        UINT32_C(0b11011100001000100000000000000000), // ST r2, 0(r1) - сохраняем 42 по адресу 32

        // Тест LD с нулевым offset
        UINT32_C(0b10110100000000110000000000100000), // ADDI r3, r0, 0x20 (32) - base
        UINT32_C(0b11100100011001000000000000000000), // LD r4, 0(r3) - загружаем с offset=0

        // Завершение
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    42,
    "LD: basic 16-bit offset test"
);

    // Тест 8: BEQ (branch if equal)
    test_(
        {
            UINT32_C(0b10110100000000010000000000000101), // ADDI r1, r0, 5
            UINT32_C(0b10110100000000100000000000000101), // ADDI r2, r0, 5
            UINT32_C(0b01101000001000100000000000000010), // BEQ r1, r2, 1
            UINT32_C(0b10110100000000110000000000000000), // ADDI r3, r0, 0 (skipped)
            UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, 15 (target)
            UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
            UINT32_C(0b00000000000000000000000000101000), // SYSCALL
        },
        15,
        "BEQ: branch if equal"
    );

test_(
    {
        // instruction 0-1: Инициализация
        UINT32_C(0b10110100000000010000000000000011), // ADDI r1, r0, 3
        UINT32_C(0b10110100000000100000000000000011), // ADDI r2, r0, 3

        // instruction 2: BEQ - должен прыгать (3 == 3)
        UINT32_C(0b01101000001000100000000000000011), // BNE r1, r2, 3

        // instruction 3-4: Путь если НЕ прыгнули
        UINT32_C(0b10110100000000110000000000000100), // ADDI r3, r0, 4
        UINT32_C(0b01111100000000000000000000000110), // J 6 (прыжок на выход)

        // instruction 5: Путь если прыгнули
        UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, 15

        // instruction 6-7: Выход
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    15,
    "BNE: branch if not equal"
);

test_(
    {
        // instruction 0: Просто прыжок на выход
        UINT32_C(0b01111100000000000000000000000010), // J 2

        // instruction 1: Эта инструкция должна быть пропущена
        UINT32_C(0b10110100000000110000000000001111), // ADDI r3, r0, 15

        // instruction 2-3: Выход
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    0,  // Ожидаем 0, т.к. r3 не менялся
    "J: simple jump to exit"
);
test_(
    {
        UINT32_C(0b10110100000000010000000000101010), // ADDI r1, r0, 0x2A (42) ← ИСПРАВЛЕНО!
        UINT32_C(0b10110100000000100000000000011111), // ADDI r2, r0, 0x1F (31)
        UINT32_C(0b10110100000001000000100000000000), // ADDI r4, r0, 0x800 (2048)
        UINT32_C(0b01010100100000010001000000000000), // STP r1, r2, 0(r4)
        UINT32_C(0b11100100100000110000000000000000), // LD r3, 0(r4)
        UINT32_C(0b10110100000010000000000000000000), // ADDI r8, r0, 0 (EXIT)
        UINT32_C(0b00000000000000000000000000101000), // SYSCALL
    },
    0x2A,
    "STP+LD: with safe high address"
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

#ifdef RUN_TESTS
int main() {
    tests();
    return 0;
}
#endif

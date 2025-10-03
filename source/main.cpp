#include "cpu.hpp"
#include <iostream>
#include <iomanip>

void print_registers(CPU& cpu)
 {
    std::cout << "Registers state:" << std::endl;
    for (int i = 0; i < 8; i++)
    { // Покажем только первые 8 регистров для краткости
        if (cpu.get_register(i) != 0)
        {
            std::cout << "r" << i << ": 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << cpu.get_register(i) << " (" << std::dec << cpu.get_register(i) << ")" << std::endl;
        }
    }
    std::cout << "PC: 0x" << std::hex << cpu.get_pc() << std::endl;
    std::cout << "------------------------" << std::endl;
}

void test_arithmetic_and_syscall()
{
    std::cout << "=== TEST: Arithmetic Operations + SYSCALL ===" << std::endl;
    CPU cpu;

    // Программа:
    // ADDI r1, r0, 15     (r1 = 15)
    // ADDI r2, r0, 25     (r2 = 25)
    // ADD r3, r1, r2      (r3 = 40)
    // SUB r4, r2, r1      (r4 = 10)
    // ADDI r8, r0, 1      (r8 = 1 - syscall PRINT_INT)
    // ADDI r0, r0, 123    (r0 = 123 - число для вывода)
    // SYSCALL             (вывод числа)
    // ADDI r8, r0, 0      (r8 = 0 - syscall EXIT)
    // SYSCALL             (завершение)
    uint32_t program[] = {
        0x2020000F,  // ADDI r1, r0, 15      (0x20 0x20 0x00 0x0F)
        0x20400019,  // ADDI r2, r0, 25      (0x20 0x40 0x00 0x19)
        0x00221820,  // ADD  r3, r1, r2      (0x00 0x22 0x18 0x20)
        0x00412022,  // SUB  r4, r2, r1      (0x00 0x41 0x20 0x22)
        0x21080001,  // ADDI r8, r0, 1       (0x21 0x08 0x00 0x01) - syscall PRINT_INT
        0x2000007B,  // ADDI r0, r0, 123     (0x20 0x00 0x00 0x7B) - число для вывода
        0x00000010,  // SYSCALL              (0x00 0x00 0x00 0x10)
        0x21080000,  // ADDI r8, r0, 0       (0x21 0x08 0x00 0x00) - syscall EXIT
        0x00000010   // SYSCALL              (0x00 0x00 0x00 0x10)
    };

    // Загружаем программу в память
    for (size_t i = 0; i < sizeof(program)/4; i++)
    {
        cpu.get_memory()->write32(i * 4, program[i]);
    }

    std::cout << "Starting program execution..." << std::endl;
    std::cout << "Program should: 15 + 25 = 40, 25 - 15 = 10, then print 123 and exit" << std::endl;
    std::cout << "------------------------" << std::endl;

    cpu.run(20);

    std::cout << "------------------------" << std::endl;
    std::cout << "Final registers:" << std::endl;
    print_registers(cpu);

    // Проверяем результаты
    std::cout << "Expected results:" << std::endl;
    std::cout << "r1 = 15, got: " << cpu.get_register(1) << " - "
              << (cpu.get_register(1) == 15 ? "✓ PASS" : "✗ FAIL") << std::endl;
    std::cout << "r2 = 25, got: " << cpu.get_register(2) << " - "
              << (cpu.get_register(2) == 25 ? "✓ PASS" : "✗ FAIL") << std::endl;
    std::cout << "r3 = 40, got: " << cpu.get_register(3) << " - "
              << (cpu.get_register(3) == 40 ? "✓ PASS" : "✗ FAIL") << std::endl;
    std::cout << "r4 = 10, got: " << cpu.get_register(4) << " - "
              << (cpu.get_register(4) == 10 ? "✓ PASS" : "✗ FAIL") << std::endl;
}


int main()
{
    std::cout << "CPU Emulator Test Suite" << std::endl;
    std::cout << "=======================" << std::endl;

    // Основной тест
    test_arithmetic_and_syscall();



    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}

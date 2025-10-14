cmake_minimum_required(VERSION 3.16)
project(CPU_Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RUN_TESTS "Build test executable" OFF)

# ОБЩИЕ настройки компиляции
set(COMMON_COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Основной эмулятор
set(EMULATOR_SOURCES
    source/cpu.cpp
    source/main.cpp
)

add_executable(cpu_emulator ${EMULATOR_SOURCES})
target_include_directories(cpu_emulator PRIVATE include)
target_compile_options(cpu_emulator PRIVATE ${COMMON_COMPILE_OPTIONS})

# Тестовый исполняемый файл
if(RUN_TESTS)
    set(TEST_SOURCES
        source/cpu.cpp
        tests/test.cpp
    )

    add_executable(cpu_emulator_tests ${TEST_SOURCES})
    target_include_directories(cpu_emulator_tests PRIVATE include)
    target_compile_options(cpu_emulator_tests PRIVATE ${COMMON_COMPILE_OPTIONS})
    target_compile_definitions(cpu_emulator_tests PRIVATE RUN_TESTS)
    message(STATUS "Building test executable: cpu_emulator_tests")
else()
    message(STATUS "Building main executable only: cpu_emulator")
endif()

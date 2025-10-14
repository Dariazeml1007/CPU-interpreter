cmake_minimum_required(VERSION 3.16)
project(CPU_Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RUN_TESTS "Build test executable" OFF)

include_directories(include)

# Основной эмулятор
set(EMULATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
)

# Проверка существования файлов
foreach(source ${EMULATOR_SOURCES})
    if(NOT EXISTS ${source})
        message(FATAL_ERROR "Source file not found: ${source}")
    endif()
endforeach()

# Основной исполняемый файл
add_executable(cpu_emulator ${EMULATOR_SOURCES})
target_compile_options(cpu_emulator PRIVATE -Wall -Wextra)
target_include_directories(cpu_emulator PRIVATE include)

# Тестовый исполняемый файл (только если включена опция)
if(RUN_TESTS)
    # Тестовые исходники
    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu.cpp    # общий код CPU
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp    # тесты со своим main()
    )

    foreach(source ${TEST_SOURCES})
        if(NOT EXISTS ${source})
            message(FATAL_ERROR "Test source file not found: ${source}")
        endif()
    endforeach()

    add_executable(cpu_emulator_tests ${TEST_SOURCES})
    target_compile_options(cpu_emulator_tests PRIVATE -Wall -Wextra)
    target_include_directories(cpu_emulator_tests PRIVATE include)
    target_compile_definitions(cpu_emulator_tests PRIVATE RUN_TESTS)
    message(STATUS "Building test executable: cpu_emulator_tests")
else()
    message(STATUS "Building main executable only: cpu_emulator")
endif()

find_package(Ruby REQUIRED)
if(Ruby_FOUND)
    message(STATUS "Found Ruby: ${RUBY_EXECUTABLE}")

    add_custom_target(assemble
        COMMAND ${RUBY_EXECUTABLE} ${CMAKE_SOURCE_DIR}/ruby/assembler.rb
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ruby
        COMMENT "Running Ruby assembler..."
    )

    add_custom_target(run
        COMMAND ${CMAKE_SOURCE_DIR}/build/cpu_emulator
        DEPENDS cpu_emulator assemble
        COMMENT "Running emulator with assembled program..."
    )

else()
    message(WARNING "Ruby not found - assembler will not work")
endif()

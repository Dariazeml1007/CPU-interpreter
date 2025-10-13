cmake_minimum_required(VERSION 3.16)
project(CPU_Emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RUN_TESTS "Run automated tests instead of user input" OFF)


include_directories(include)


set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
)


foreach(source ${SOURCES})
    if(NOT EXISTS ${source})
        message(FATAL_ERROR "Source file not found: ${source}")
    endif()
endforeach()

add_executable(cpu_emulator ${SOURCES})
target_compile_options(cpu_emulator PRIVATE -Wall -Wextra)
target_include_directories(cpu_emulator PRIVATE include)

# Передаем флаг RUN_TESTS в код
if(RUN_TESTS)
    target_compile_definitions(cpu_emulator PRIVATE RUN_TESTS)
    message(STATUS "Building in TEST mode - will run automated tests")
else()
    message(STATUS "Building in NORMAL mode - will execute program from binary file")
endif()

find_package(Ruby REQUIRED)
if(Ruby_FOUND)
    message(STATUS "Found Ruby: ${RUBY_EXECUTABLE}")

    # Цель для ассемблера
    add_custom_target(assemble
        COMMAND ${RUBY_EXECUTABLE} ${CMAKE_SOURCE_DIR}/ruby/assembler.rb
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ruby
        COMMENT "Running Ruby assembler..."
    )

    # Цель которая сначала ассемблирует, потом запускает эмулятор
    add_custom_target(run
        COMMAND ${CMAKE_SOURCE_DIR}/build/cpu_emulator
        DEPENDS cpu_emulator assemble
        COMMENT "Running emulator with assembled program..."
    )

else()
    message(WARNING "Ruby not found - assembler will not work")
endif()

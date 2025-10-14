require 'fileutils'
require_relative 'assembler'

if ARGV.empty?
  puts "Usage: ruby assemble.rb <output_path>"
  puts "Example:"
  puts "  ruby assemble.rb program.bin"
  exit 1
end

output_path = ARGV[0]

puts " Assembling program..."


assembler = Assembler.new(output_path)
program = assembler.assemble do
  addi r8, r0, 3      # SYS_READ_INT
  syscall
  add r1, r0, r3    # r1 = n (из r3)


  addi r2, r0, 0      # F(n-2) = 0
  addi r3, r0, 1      # F(n-1) = 1
  addi r4, r0, 1      # i = 1


  addi r5, r0, 0
  beq r1, r5, 12       # if n == 0, jump to output_0 (instruction 19)

  addi r5, r0, 1
  beq r1, r5, 12       # if n == 1, jump to output_1 (instruction 21)

  # instruction 11:
  addi r4, r4, 1      # i++
  add r6, r2, r3    # temp = a + b
  add r2, r0, r3    # a = b
  add r3, r0, r6    # b = temp
  bne r4, r1, -5     # loop


  addi r8, r0, 1      # SYS_PRINT_INT
  # instruction 16:
  syscall
  # instruction 17:
  addi r8, r0, 0      # SYS_EXIT
  # instruction 18:
  syscall

  # instruction 19: (n=0)
  addi r3, r0, 0      # F(0) = 0
  # instruction 20:
  j 15                   # jump to output (instruction 13)

  # instruction 21: (n=1)
  addi r3, r0, 1      # F(1) = 1
  # instruction 22:
  j 15                   # jump to output (instruction 13)

  puts " Assembly completed. Output: #{output_path}"
end

class Assembler
  def initialize
    @code = []
    @labels = {}
    @pc = 0
  end

  def assemble(&block)
    puts "Assembler is beginning: "
    instance_eval(&block)
    generate_binary
    return @code
  end

  def addi(rt, rs, imm)
    opcode = 0b101101
    instruction = (opcode << 26) | (reg_num(rs) << 21) | (reg_num(rt) << 16) | (imm & 0xFFFF)
    emit(instruction)
    puts "ADDI #{rt}, #{rs}, #{imm} -> opcode: #{opcode}"
  end

  def add(rd, rs, rt)
    opcode = 0b000000
    funct = 0b010010

    instruction = (opcode << 26) |
                  (reg_num(rs) << 21) |
                  (reg_num(rt) << 16) |
                  (reg_num(rd) << 11) |
                  (0 << 6) |
                  funct

    emit(instruction)
    puts "ADD #{rd}, #{rs}, #{rt} -> opcode: #{opcode}, funct: #{funct}"
  end

  def sub(rd, rs, rt)
    opcode = 0b000000
    funct = 0b110110

    instruction = (opcode << 26) |
                  (reg_num(rs) << 21) |
                  (reg_num(rt) << 16) |
                  (reg_num(rd) << 11) |
                  (0 << 6) |
                  funct

    emit(instruction)
    puts "SUB #{rd}, #{rs}, #{rt} -> opcode: #{opcode}, funct: #{funct}"
  end

  def syscall
    opcode = 0b000000
    funct = 0b101000
    code = 0

    instruction = (opcode << 26) | (code << 6) | funct
    emit(instruction)
    puts "SYSCALL (номер в X8)"
  end

  def ld(rt, offset_base)
    opcode = 0b111001

    #  "offset(base)"
    offset_str, base_str = offset_base.split('(')
    base_str = base_str.chomp(')')  # Убираем закрывающую скобку

    offset = offset_str.empty? ? 0 : offset_str.to_i
    base = base_str

    # Собираем инструкцию
    instruction = (opcode << 26) | (reg_num(base) << 21) | (reg_num(rt) << 16) | (offset & 0xFFFF)
    emit(instruction)

    puts "LD #{rt}, #{offset}(#{base})"
  end

  def cls(rd, rs)
    opcode = 0b000000
    funct = 0b001010

    instruction = (opcode << 26) |
                  (reg_num(rd) << 21) |
                  (reg_num(rs) << 16) |
                  funct

    emit(instruction)
    puts "CLS #{rd}, #{rs}"
  end

  def bne(rs, rt, instruction_offset)
    opcode = 0b011000
    instruction = (opcode << 26) | (reg_num(rs) << 21) | (reg_num(rt) << 16) | (instruction_offset & 0xFFFF)
    emit(instruction)
    puts "BNE #{rs}, #{rt}, #{instruction_offset} (jump #{instruction_offset} instructions)"
  end

  def beq(rs, rt, instruction_offset)
    opcode = 0b011010
    instruction = (opcode << 26) | (reg_num(rs) << 21) | (reg_num(rt) << 16) | (instruction_offset & 0xFFFF)
    emit(instruction)
    puts "BEQ #{rs}, #{rt}, #{instruction_offset} (jump #{instruction_offset} instructions)"
  end

  def sbit(rd, rs, imm5)

    if imm5.is_a?(String) && imm5.start_with?('#')
      imm5 = imm5[1..-1].to_i  # Убираем '#' и преобразуем в число
    else
      imm5 = imm5.to_i
    end

    opcode = 0b011100
    zeros_10_0 = 0b00000000000

    instruction = (opcode << 26) |
                (reg_num(rd) << 21) |
                (reg_num(rs) << 16) |
                (imm5 << 11) |
                zeros_10_0

    emit(instruction)
    puts "SBIT #{rd}, #{rs}, ##{imm5}"
  end

  def bext(rd, rs1, rs2)
    opcode = 0b000000
    zero_10_6 = 0b00000
    funct = 0b010100

    instruction = (opcode << 26) |
                  (reg_num(rd) << 21) |
                  (reg_num(rs1) << 16) |
                  (reg_num(rs2) << 11) |
                  (zero_10_6 << 6) |
                  funct

    emit(instruction)
    puts "BEXT #{rd}, #{rs1}, #{rs2}"
  end

  def j(instruction_index)
    opcode = 0b011111
    instruction = (opcode << 26) | (instruction_index & 0x3FFFFFF)
    emit(instruction)
    puts "J #{instruction_index} (jump to instruction ##{instruction_index})"
  end

  def ssat(rd, rs, imm5)
    if imm5.is_a?(String) && imm5.start_with?('#')
      imm5 = imm5[1..-1].to_i  # Убираем '#' и преобразуем в число
    else
      imm5 = imm5.to_i
    end

    opcode = 0b001101
    zeros_10_0 = 0b00000000000

    instruction = (opcode << 26) |
                  (reg_num(rd) << 21) |
                  (reg_num(rs) << 16) |
                  (imm5 << 11) |
                  zeros_10_0

    emit(instruction)
    puts "SSAT #{rd}, #{rs}, ##{imm5}"
  end

  def st(rt, offset_base)
    opcode = 0b110111

    #  "offset(base)"
    offset_str, base_str = offset_base.split('(')
    base_str = base_str.chomp(')')

    offset = offset_str.empty? ? 0 : offset_str.to_i
    base = base_str

    instruction = (opcode << 26) | (reg_num(base) << 21) | (reg_num(rt) << 16) | (offset & 0xFFFF)
    emit(instruction)

    puts "ST #{rt}, #{offset}(#{base})"
  end

  def stp(rt1, rt2, offset_base)
    opcode = 0b010101

    offset_str, base_str = offset_base.split('(')
    base_str = base_str.chomp(')')

    offset = offset_str.empty? ? 0 : offset_str.to_i
    base = base_str

    instruction = (opcode << 26) |
                  (reg_num(base) << 21) |
                  (reg_num(rt1) << 16) |
                  (reg_num(rt2) << 11) |
                  (offset & 0x7FF)

    emit(instruction)
    puts "STP #{rt1}, #{rt2}, #{offset}(#{base})"
  end


  private

  def emit(instruction)
    @code << instruction
    @pc += 4
  end

  def reg_num(reg)
    case reg.to_s.downcase
    when 'r0', 'zero' then 0
    when 'r1' then 1
    when 'r2' then 2
    when 'r3' then 3
    when 'r4' then 4
    when 'r5' then 5
    when 'r6' then 6
    when 'r7' then 7
    when 'r8' then 8
    when 'r9' then 9
    when 'r10' then 10
    else reg.to_i
    end
  end

  def generate_binary
    puts "\nGenerating bin file"

     File.open('output.bin', 'wb') do |file|
    @code.each do |instruction|
      # little-endian
      file.write([instruction].pack('V'))  # 'V' = little-endian 32-bit
    end
  end

    puts "File created #{@code.size} instructions"


    puts "\n Generate :"
    @code.each_with_index do |instr, i|
      opcode = (instr >> 26) & 0x3F
      funct = instr & 0x3F
      printf "  %2d: 0x%08X (opcode: 0x%02X, funct: 0x%02X)\n", i, instr, opcode, funct
    end
  end
end

assembler = Assembler.new
program = assembler.assemble do
  addi 'r8', 'r0', 3      # SYS_READ_INT
  syscall
  add 'r1', 'r0', 'r3'    # r1 = n (из r3)


  addi 'r2', 'r0', 0      # F(n-2) = 0
  addi 'r3', 'r0', 1      # F(n-1) = 1
  addi 'r4', 'r0', 1      # i = 1


  addi 'r5', 'r0', 0
  beq 'r1', 'r5', 12       # if n == 0, jump to output_0 (instruction 16)

  addi 'r5', 'r0', 1
  beq 'r1', 'r5', 12       # if n == 1, jump to output_1 (instruction 17)

  # instruction 11:
  addi 'r4', 'r4', 1      # i++
  add 'r6', 'r2', 'r3'    # temp = a + b
  add 'r2', 'r0', 'r3'    # a = b
  add 'r3', 'r0', 'r6'    # b = temp
  bne 'r4', 'r1', -5      # loop


  addi 'r8', 'r0', 1      # SYS_PRINT_INT
  # instruction 16:
  syscall
  # instruction 17:
  addi 'r8', 'r0', 0      # SYS_EXIT
  # instruction 18:
  syscall

  # instruction 19: (n=0)
  addi 'r3', 'r0', 0      # F(0) = 0
  # instruction 20:
  j 15                   # jump to output (instruction 13)

  # instruction 21: (n=1)
  addi 'r3', 'r0', 1      # F(1) = 1
  # instruction 22:
  j 15                   # jump to output (instruction 13)
end

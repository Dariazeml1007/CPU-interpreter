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

  # ADDI rt, rs, #imm - I-формат
  def addi(rt, rs, imm)
    opcode = 0b101101  #
    instruction = (opcode << 26) | (reg_num(rs) << 21) | (reg_num(rt) << 16) | (imm & 0xFFFF)
    emit(instruction)
    puts "ADDI #{rt}, #{rs}, #{imm} -> opcode: #{opcode}"
  end

  # ADD rd, rs, rt - R-формат
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

  # SUB rd, rs, rt - R-формат
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
  addi 'r1', 'r0', 5    # r1 = 5
  addi 'r2', 'r0', 3    # r2 = 3
  add 'r3', 'r1', 'r2'  # r3 = r1 + r2 = 8
  sub 'r4', 'r1', 'r2'  # r4 = r1 - r2 = 2
  addi 'r8', 'r0', 1    # r8 = 1
  syscall
  addi 'r8', 'r0', 0    # r8 = 1
  syscall

end

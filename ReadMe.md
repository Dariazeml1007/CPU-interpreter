# CPU Emulator with Custom ISA and Ruby-based Micro Assembler

This project implements a custom CPU emulator supporting a unique ISA (Instruction Set Architecture) designed by a creative architect, featuring 14 distinct instructions with varying encodings across different instruction sets, including several unconventional RISC-style operations. The emulator is complemented by a Ruby-based micro-assembler that allows writing machine code using Ruby method calls resembling assembly syntax, which then generates binary instruction files for execution in the emulator. The system provides a complete toolchain from assembly-like code creation to binary execution in a simulated hardware environment.

## 🛠️ Build and Run
 1. Create a build directory and generate build files:
  ```c
cmake -B build -D RUN_TESTS=OFF
  ```
 2. Build the project:
  ```c
cmake --build build
  ```
3. Run the executables:
  ```c
./cpu_emulator

  ```
---

##  Run Built-in Tests
To run automated tests instead  - rebuild the project with RUN_TESTS=ON
  ```c
cmake -B build -D RUN_TESTS=ON
  ```

---

## Instruction Formats (32 bits)
- **R-type**: `opcode(6) rs(5) rt(5) rd(5) shamt(5) funct(6)`
- **I-type**: `opcode(6) rs(5) rt(5) imm(16)`
- **J-type**: `opcode(6) index(26)`

---


### 1. LD — Load Word

| 31:26 | 25:21 | 20:16 | 15:0     |
|-------|-------|-------|----------|
| 111001| base  | rt    | offset   |

**Assembler:** `LD rt, offset(base)`
**Operation:** Load word from memory into register.
**Notes:** `offset` must be multiple of 4. Misaligned access → undefined behavior.
---

### 2. CLS — Count Leading Signs

| 31:26 | 25:21 | 20:16 | 15:6        | 5:0   |
|-------|-------|-------|-------------|-------|
| 000000| rd    | rs    | 0000000000  | 001010|

**Assembler:** `CLS rd, rs`
**Operation:** Count leading bits equal to the sign bit of `X[rs]`.
**Notes:** Result is in range 0–32.

---

### 3. SYSCALL — System Call

| 31:26 | 25:6         | 5:0   |
|-------|--------------|-------|
| 000000| code         | 101000|

**Assembler:** `SYSCALL`
**Operation:** Trigger system call exception.
**Notes:** `X[8]` = syscall number; args in `X[3]`; result in `X[3]`.

---

### 4. BNE — Branch if Not Equal

| 31:26 | 25:21 | 20:16 | 15:0     |
|-------|-------|-------|----------|
| 011000| rs    | rt    | offset   |

**Assembler:** `BNE rs, rt, #offset`
**Operation:** Branch if `X[rs] != X[rt]`.
**Notes:** Target = `PC + sign_extend(offset << 2)`. Offset ×4, word-aligned.

---

### 5. BEQ — Branch if Equal

| 31:26 | 25:21 | 20:16 | 15:0     |
|-------|-------|-------|----------|
| 011010| rs    | rt    | offset   |

**Assembler:** `BEQ rs, rt, #offset`
**Operation:** Branch if `X[rs] == X[rt]`.
**Notes:** Target = `PC + sign_extend(offset << 2)`. Offset ×4.

---

### 6. SBIT — Set Bit

| 31:26 | 25:21 | 20:16 | 15:11 | 10:0        |
|-------|-------|-------|-------|-------------|
| 011100| rd    | rs    | imm5  | 00000000000 |

**Assembler:** `SBIT rd, rs, #imm5`
**Operation:** Set bit `imm5`, clear all others.
**Notes:** `X[rd] = 1 << imm5`. `imm5` in [0, 31].

---

### 7. BEXT — Bit Extract

| 31:26 | 25:21 | 20:16 | 15:11 | 10:6 | 5:0   |
|-------|-------|-------|-------|------|-------|
| 000000| rd    | rs1   | rs2   | 00000| 010100|

**Assembler:** `BEXT rd, rs1, rs2`
**Operation:** Extract bits from `X[rs1]` where `X[rs2]` has 1s.
**Notes:** Packed into LSBs of `X[rd]` in increasing bit order.

---

### 8. SUB — Subtract

| 31:26 | 25:21 | 20:16 | 15:11 | 10:6 | 5:0   |
|-------|-------|-------|-------|------|-------|
| 000000| rs    | rt    | rd    | 00000| 110110|

**Assembler:** `SUB rd, rs, rt`
**Operation:** `X[rd] = X[rs] - X[rt]`.
**Notes:** Standard two’s complement subtraction.

---

### 9. ADDI — Add Immediate

| 31:26 | 25:21 | 20:16 | 15:0     |
|-------|-------|-------|----------|
| 101101| rs    | rt    | imm      |

**Assembler:** `ADDI rt, rs, #imm`
**Operation:** `X[rt] = X[rs] + sign_extend(imm)`.
**Notes:** Immediate is sign-extended.

---

### 10. ADD — Add

| 31:26 | 25:21 | 20:16 | 15:11 | 10:6 | 5:0   |
|-------|-------|-------|-------|------|-------|
| 000000| rs    | rt    | rd    | 00000| 010010|

**Assembler:** `ADD rd, rs, rt`
**Operation:** `X[rd] = X[rs] + X[rt]`.
**Notes:** Standard two’s complement addition.

---

### 11. J — Jump

| 31:26 | 25:0     |
|-------|----------|
| 011111| index    |

**Assembler:** `J target`
**Operation:** Unconditional jump to computed address.
**Notes:** `PC = (PC & 0xF0000000) | (index << 2)`. Word-aligned.

---

### 12. SSAT — Signed Saturation

| 31:26 | 25:21 | 20:16 | 15:11 | 10:0        |
|-------|-------|-------|-------|-------------|
| 001101| rd    | rs    | imm5  | 00000000000 |

**Assembler:** `SSAT rd, rs, #imm5`
**Operation:** Saturate `X[rs]` to `imm5`-bit signed range.
**Notes:** Range: `[-2^(N-1), 2^(N-1)-1]`, where N = `imm5`.

---

### 13. ST — Store Word

| 31:26 | 25:21 | 20:16 | 15:0     |
|-------|-------|-------|----------|
| 110111| base  | rt    | offset   |

**Assembler:** `ST rt, offset(base)`
**Operation:** Store register to memory.
**Notes:** `offset` must be multiple of 4. Misaligned access → undefined behavior.

---

### 14. STP — Store Pair

| 31:26 | 25:21 | 20:16 | 15:11 | 10:0     |
|-------|-------|-------|-------|----------|
| 010101| base  | rt1   | rt2   | offset   |

**Assembler:** `STP rt1, rt2, offset(base)`
**Operation:** Store two registers to consecutive memory words.
**Notes:** `offset` must be word-aligned. Misaligned access → undefined behavior.

---






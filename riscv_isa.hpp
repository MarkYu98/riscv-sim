#ifndef RISCV_ISA_HPP
#define RISCV_ISA_HPP

#include <string.h>

// Integer registers
#define R_ZERO 0
#define R_RA   1
#define R_SP   2
#define R_GP   3
#define R_TP   4
#define R_T0   5
#define R_T1   6
#define R_T2   7
#define R_S0   8
#define R_FP   8
#define R_S1   9
#define R_A0   10
#define R_A1   11
#define R_A2   12
#define R_A3   13
#define R_A4   14
#define R_A5   15
#define R_A6   16
#define R_A7   17
#define R_S2   18
#define R_S3   19
#define R_S4   20
#define R_S5   21
#define R_S6   22
#define R_S7   23
#define R_S8   24
#define R_S9   25
#define R_S10  26
#define R_S11  27
#define R_T3   28
#define R_T4   29
#define R_T5   30
#define R_T6   31

const std::string R_NAMES[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", 
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", 
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", 
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

typedef int raw_inst_t;

// Instruction masks
#define OPCODE_MASK 0x7f
#define OPCODE_SHFT 0
#define OPCODE(x)   ((x & OPCODE_MASK) >> OPCODE_SHFT)
#define RD_MASK     0xf80
#define RD_SHFT     7
#define RD(x)       ((x & RD_MASK) >> RD_SHFT)
#define RS1_MASK    0xf8000
#define RS1_SHFT    15
#define RS1(x)      ((x & RS1_MASK) >> RS1_SHFT)
#define RS2_MASK    0x1f00000
#define RS2_SHFT    20
#define RS2(x)      ((x & RS2_MASK) >> RS2_SHFT)
#define FUNCT7_MASK 0xfe000000
#define FUNCT7_SHFT 25
#define FUNCT7(x)   ((x & FUNCT7_MASK) >> FUNCT7_SHFT)
#define FUNCT3_MASK 0x7000
#define FUNCT3_SHFT 12
#define FUNCT3(x)   ((x & FUNCT3_MASK) >> FUNCT3_SHFT)

#define I_IMM(x)    (raw_inst_t(x & 0xfff00000) >> 20)
#define S_IMM(x)    ((raw_inst_t(x & 0xfe000000) >> 20) | (raw_inst_t(x & 0xf80) >> 7))
#define SB_IMM(x)   ((raw_inst_t(x & 0x80000000) >> 19) | (raw_inst_t(x & 0x80) << 4) | \
                    (raw_inst_t(x & 0x7e000000) >> 20) | (raw_inst_t(x & 0xf00) >> 7)) 
#define U_IMM(x)    raw_inst_t(x & 0xfffff000)
#define UJ_IMM(x)   ((raw_inst_t(x & 0x80000000) >> 11) | (raw_inst_t(x & 0x7fe00000) >> 20) | \
                    (raw_inst_t(x & 0x100000) >> 9) | raw_inst_t(x & 0xff000))

// Instruction types
enum INST_TYPE 
{
    IT_R, IT_I, IT_S, IT_SB, IT_U, IT_UJ
};

enum R_INST_TYPE
{
    R_ADD, R_SUB, R_SLL, R_SLT, R_SLTU, R_XOR,
    R_SRL, R_SRA, R_OR, R_AND, R_ADDW, R_SUBW, 
    R_SLLW, R_SRLW, R_SRAW, R_MUL, R_MULH, R_MULHSU, 
    R_MULHU, R_DIV, R_DIVU, R_REM, R_REMU, R_MULW,
    R_DIVW, R_DIVUW, R_REMW, R_REMUW, R_UNIMP
};
const std::string R_INST_NAME[] = 
{
    "add", "sub", "sll", "slt", "sltu", "xor",
    "srl", "sra", "or", "and", "addw", "subw", 
    "sllw", "srlw", "sraw", "mul", "mulh", "mulhsu",
    "mulhu", "div", "divu", "rem", "remu", "mulw",
    "divw", "divuw", "remw", "remuw", "unimp"
};

enum I_INST_TYPE
{
    I_LB, I_LH, I_LW, I_LD, I_LBU, I_LHU, I_LWU, I_ADDI, 
    I_SLLI, I_SLTI, I_SLTIU, I_XORI, I_SRLI, I_SRAI, 
    I_ORI, I_ANDI, I_JALR, I_ECALL, I_EBREAK, I_ADDIW, 
    I_SLLIW, I_SRLIW, I_SRAIW, I_UNIMP
};
const std::string I_INST_NAME[] = 
{
    "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu", "addi", 
    "slli", "slti", "sltiu", "xori", "srli", "srai", 
    "ori", "andi", "jalr", "ecall", "ebreak", "addiw", 
    "slliw", "srliw", "sraiw", "unimp"
};

enum S_INST_TYPE { S_SB, S_SH, S_SW, S_SD, S_UNIMP };
const std::string S_INST_NAME[] = { "sb", "sh", "sw", "sd", "unimp" };

enum SB_INST_TYPE { SB_BEQ, SB_BNE, SB_BLT, SB_BGE, SB_BLTU, SB_BGEU, SB_UNIMP };
const std::string SB_INST_NAME[] = { "beq", "bne", "blt", "bge", "bltu", "bgeu", "unimp" };

enum U_INST_TYPE { U_AUPIC, U_LUI, U_UNIMP };
const std::string U_INST_NAME[] = { "aupic", "lui", "unimp" };

enum UJ_INST_TYPE { UJ_JAL, UJ_UNIMP };
const std::string UJ_INST_NAME[] = { "jal", "unimp" };

struct RISCV_inst 
{
    INST_TYPE type;
    unsigned raw_inst;
    union {
        R_INST_TYPE type_r;
        I_INST_TYPE type_i;
        S_INST_TYPE type_s;
        SB_INST_TYPE type_sb;
        U_INST_TYPE type_u;
        UJ_INST_TYPE type_uj;
    } subtype;
    union {
        struct { uint8_t funct7, funct3, rs2, rs1, rd, opcode; } inst_r;
        struct { uint8_t funct3, rs1, rd, opcode; short imm; } inst_i;
        struct { uint8_t funct3, rs1, rs2, opcode; short imm; } inst_s;
        struct { uint8_t funct3, rs1, rs2, opcode; short imm; } inst_sb;
        struct { uint8_t rd, opcode; int imm; } inst_u;
        struct { uint8_t rd, opcode; int imm; } inst_uj;
    } inst;

    static INST_TYPE get_inst_type(raw_inst_t inst) 
    {
        if (OPCODE(inst) == 0x33 || OPCODE(inst) == 0x3b)
            return IT_R;
        if (OPCODE(inst) == 0x23)
            return IT_S;
        if (OPCODE(inst) == 0x63)
            return IT_SB;
        if (OPCODE(inst) == 0x6f)
            return IT_UJ;
        if (OPCODE(inst) == 0x17 || OPCODE(inst) == 0x37)
            return IT_U;
        return IT_I;
    }

    RISCV_inst(raw_inst_t raw_inst) : raw_inst(raw_inst) 
    {
        type = get_inst_type(raw_inst);
        switch (type) {
        case IT_R:
            inst.inst_r.funct7 = FUNCT7(raw_inst);
            inst.inst_r.funct3 = FUNCT3(raw_inst);
            inst.inst_r.rs2 = RS2(raw_inst);
            inst.inst_r.rs1 = RS1(raw_inst);
            inst.inst_r.rd = RD(raw_inst);
            inst.inst_r.opcode = OPCODE(raw_inst);
            if (inst.inst_r.funct3 == 0x0) {
                if (inst.inst_r.funct7 == 0x00) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_ADD;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_ADDW;
                }
                else if (inst.inst_r.funct7 == 0x01) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_MUL;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_MULW;
                }
                else if (inst.inst_r.funct7 == 0x20) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_SUB;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_SUBW;
                }
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x1) {
                if (inst.inst_r.funct7 == 0x00) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_SLL;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_SLLW;
                }
                else if (inst.inst_r.funct7 == 0x01) subtype.type_r = R_MULH;
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x2) {
                if (inst.inst_r.funct7 == 0x00) subtype.type_r = R_SLT;
                else if (inst.inst_r.funct7 == 0x01) subtype.type_r = R_MULHSU;
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x3) {
                if (inst.inst_r.funct7 == 0x00) subtype.type_r = R_SLTU;
                else if (inst.inst_r.funct7 == 0x01) subtype.type_r = R_MULHU;
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x4) {
                if (inst.inst_r.funct7 == 0x00) subtype.type_r = R_XOR;
                else if (inst.inst_r.funct7 == 0x01) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_DIV;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_DIVW;
                }
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x5) {
                if (inst.inst_r.funct7 == 0x00) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_SRL;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_SRLW;
                }
                else if (inst.inst_r.funct7 == 0x01) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_DIVU;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_DIVUW;
                }
                else if (inst.inst_r.funct7 == 0x20) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_SRA;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_SRAW;
                }
                else subtype.type_r = R_UNIMP;
            } 
            else if (inst.inst_r.funct3 == 0x6) {
                if (inst.inst_r.funct7 == 0x00) subtype.type_r = R_OR;
                else if (inst.inst_r.funct7 == 0x01) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_REM;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_REMW;
                }
                else subtype.type_r = R_UNIMP;
            }
            else if (inst.inst_r.funct3 == 0x7) {
                if (inst.inst_r.funct7 == 0x00) subtype.type_r = R_AND;
                else if (inst.inst_r.opcode == 0x01) {
                    if (inst.inst_r.opcode == 0x33) subtype.type_r = R_REMU;
                    else if (inst.inst_r.opcode == 0x3b) subtype.type_r = R_REMUW;
                }
                else subtype.type_r = R_UNIMP;
            }
            else subtype.type_r = R_UNIMP;
            break;
        case IT_I:
            inst.inst_i.funct3 = FUNCT3(raw_inst);
            inst.inst_i.rs1 = RS1(raw_inst);
            inst.inst_i.rd = RD(raw_inst);
            inst.inst_i.opcode = OPCODE(raw_inst);
            inst.inst_i.imm = I_IMM(raw_inst);
            if (inst.inst_i.opcode == 0x03) {
                if (inst.inst_i.funct3 == 0x0) subtype.type_i = I_LB;
                else if (inst.inst_i.funct3 == 0x1) subtype.type_i = I_LH;
                else if (inst.inst_i.funct3 == 0x2) subtype.type_i = I_LW;
                else if (inst.inst_i.funct3 == 0x3) subtype.type_i = I_LD;
                else if (inst.inst_i.funct3 == 0x4) subtype.type_i = I_LBU;
                else if (inst.inst_i.funct3 == 0x5) subtype.type_i = I_LHU;
                else if (inst.inst_i.funct3 == 0x6) subtype.type_i = I_LWU;
                else subtype.type_i = I_UNIMP;
            }
            else if (inst.inst_i.opcode == 0x13) {
                if (inst.inst_i.funct3 == 0x0) subtype.type_i = I_ADDI;
                else if (inst.inst_i.funct3 == 0x1) subtype.type_i = I_SLLI;
                else if (inst.inst_i.funct3 == 0x2) subtype.type_i = I_SLTI;
                else if (inst.inst_i.funct3 == 0x3) subtype.type_i = I_SLTIU;
                else if (inst.inst_i.funct3 == 0x4) subtype.type_i = I_XORI;
                else if (inst.inst_i.funct3 == 0x5) {
                    if (inst.inst_i.imm & 0x400) subtype.type_i = I_SRAI;
                    else subtype.type_i = I_SRLI;
                }
                else if (inst.inst_i.funct3 == 0x6) subtype.type_i = I_ORI;
                else if (inst.inst_i.funct3 == 0x7) subtype.type_i = I_ANDI;
                else subtype.type_i = I_UNIMP;
            }
            else if (inst.inst_i.opcode == 0x1b) {
                if (inst.inst_i.funct3 == 0x0) subtype.type_i = I_ADDIW;
                else if (inst.inst_i.funct3 == 0x1) subtype.type_i = I_SLLIW;
                else if (inst.inst_i.funct3 == 0x5) {
                    if (inst.inst_i.imm & 0x400) subtype.type_i = I_SRAIW;
                    else subtype.type_i = I_SRLIW;
                }
                else subtype.type_i = I_UNIMP;
            }
            else if (inst.inst_i.opcode == 0x67) {
                if (inst.inst_i.funct3 == 0x0) subtype.type_i = I_JALR;
                else subtype.type_i = I_UNIMP;
            }
            else if (inst.inst_i.opcode == 0x73) {
                if (inst.inst_i.funct3 == 0x0) subtype.type_i = I_ECALL;
                else if (inst.inst_i.funct3 == 0x1) subtype.type_i = I_EBREAK;
                else subtype.type_i = I_UNIMP;
            }
            else subtype.type_i = I_UNIMP;
            break;
        case IT_S:
            inst.inst_s.funct3 = FUNCT3(raw_inst);
            inst.inst_s.rs2 = RS2(raw_inst);
            inst.inst_s.rs1 = RS1(raw_inst);
            inst.inst_s.opcode = OPCODE(raw_inst);
            inst.inst_s.imm = S_IMM(raw_inst);
            if (inst.inst_s.funct3 == 0x0) subtype.type_s = S_SB;
            else if (inst.inst_s.funct3 == 0x1) subtype.type_s = S_SH;
            else if (inst.inst_s.funct3 == 0x2) subtype.type_s = S_SW;
            else if (inst.inst_s.funct3 == 0x3) subtype.type_s = S_SD;
            else subtype.type_s = S_UNIMP;
            break;
        case IT_SB:
            inst.inst_sb.funct3 = FUNCT3(raw_inst);
            inst.inst_sb.rs2 = RS2(raw_inst);
            inst.inst_sb.rs1 = RS1(raw_inst);
            inst.inst_sb.opcode = OPCODE(raw_inst);
            inst.inst_sb.imm = SB_IMM(raw_inst);
            if (inst.inst_sb.funct3 == 0x0) subtype.type_sb = SB_BEQ;
            else if (inst.inst_sb.funct3 == 0x1) subtype.type_sb = SB_BNE;
            else if (inst.inst_sb.funct3 == 0x4) subtype.type_sb = SB_BLT;
            else if (inst.inst_sb.funct3 == 0x5) subtype.type_sb = SB_BGE;
            else if (inst.inst_sb.funct3 == 0x6) subtype.type_sb = SB_BLTU;
            else if (inst.inst_sb.funct3 == 0x7) subtype.type_sb = SB_BGEU;
            else subtype.type_sb = SB_UNIMP;
            break;
        case IT_U:
            inst.inst_u.rd = RD(raw_inst);
            inst.inst_u.opcode = OPCODE(raw_inst);
            inst.inst_u.imm = U_IMM(raw_inst);
            if (inst.inst_u.opcode == 0x17) subtype.type_u = U_AUPIC;
            else if (inst.inst_u.opcode == 0x37) subtype.type_u = U_LUI;
            else subtype.type_u = U_UNIMP;
            break;
        case IT_UJ:
            inst.inst_uj.rd = RD(raw_inst);
            inst.inst_uj.opcode = OPCODE(raw_inst);
            inst.inst_uj.imm = UJ_IMM(raw_inst);
            if (inst.inst_uj.opcode == 0x6f) subtype.type_uj = UJ_JAL;
            else subtype.type_uj = UJ_UNIMP;
            break;
        }
    }
};

#endif // RISCV_ISA_HPP
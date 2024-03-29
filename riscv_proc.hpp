#ifndef RISCV_PROC_HPP
#define RISCV_PROC_HPP

#include <elfio/elfio.hpp>
#include <cache/cache.h>
#include <cache/memory.h>
#include <riscv_isa.hpp>
#include <riscv_config.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#define PGSIZE      4096
#define PTE_P       0x1
#define PTE_W       0x2
#define PTE_X       0x4
#define PG_ALLOC(pte) (pte.flags & PTE_P)
#define PG_WRITE(pte) (pte.flags & PTE_W)
#define PG_EXEC(pte)  (pte.flags & PTE_X)
#define PAGE(vaddr)   ROUND_DOWN(vaddr, PGSIZE)

#define STACK_ALIGN 1024

#define ALU_ADD     0
#define ALU_SUB     1
#define ALU_SLL     2
#define ALU_SLT     3
#define ALU_SLTU    4
#define ALU_XOR     5
#define ALU_SRL     6
#define ALU_SRA     7
#define ALU_OR      8
#define ALU_AND     9
#define ALU_ADDW    10
#define ALU_SUBW    11
#define ALU_SLLW    12
#define ALU_SRLW    13
#define ALU_SRAW    14
#define ALU_MUL     15
#define ALU_MULH    16
#define ALU_MULHSU  17
#define ALU_MULHU   18
#define ALU_DIV     19
#define ALU_DIVU    20
#define ALU_REM     21
#define ALU_REMU    22
#define ALU_MULW    23
#define ALU_DIVW    24
#define ALU_DIVUW   25
#define ALU_REMW    26
#define ALU_REMUW   27
#define ALU_NOP     28

#define SYS_PRINT_I 1
#define SYS_PRINT_C 2
#define SYS_PRINT_S 3
#define SYS_READ_I  4
#define SYS_READ_C  5
#define SYS_SBRK    6       // Extend heap for malloc
#define SYS_HEAP_LO 7
#define SYS_HEAP_HI 8
#define SYS_EXIT    93

#ifdef PIPE
#define DATA_FORWARD(x) ((x.opcode == 0x67 || x.opcode == 0x6f) ? x.val : x.res)
#define CLOCK_TICK(x, X) if (ctrl_##X != PCTRL_STALL) reg_##X = (ctrl_##X == PCTRL_NORMAL) ? reg_##x : PIPE_REG_##X();
#endif

typedef unsigned long long REG;
typedef long long SREG;

typedef struct {
    uint8_t flags = 0;
    size_t paddr = 0;
} pte_t;
typedef std::map<size_t, pte_t> pgtb_t;    // PageTable

enum PipeControl { PCTRL_NORMAL, PCTRL_STALL, PCTRL_BUBBLE };

static size_t ROUND_UP(size_t bytes, size_t ALIGN)
{ 
    return (((bytes) + ALIGN - 1) & ~(ALIGN - 1));
}

static size_t ROUND_DOWN(size_t bytes, size_t ALIGN)
{ 
    return (bytes) & ~(ALIGN - 1);
}

std::string dec2hex(size_t i);
std::string dec2hex(size_t i, size_t bytes);
REG alu_calc(REG src1, REG src2, unsigned ALU_FUNC);

class CachedStorage {
public:
    CachedStorage() {
        memory.SetPGSize(PGSIZE);
        L1.SetLower(&L2);
        L2.SetLower(&L3);
        L3.SetLower(&memory);
        ClearStats();
    }
    ~CachedStorage() {}
    void ClearStats();
    void PrintStats();
    void flush();
    void reset_memory() { memory.reset(); }
    void SetConfig(CacheConfig cc1, CacheConfig cc2, CacheConfig cc3);
    void SetLatency(StorageLatency ltc1, StorageLatency ltc2, StorageLatency ltc3, StorageLatency ltcm);
    void free_page(size_t addr) { memory.free_page(addr); }
    size_t alloc_page() { return memory.alloc_page(); }
    void HandleRequest(size_t addr, int bytes, int read,
                       char *content, int &time);
private:
    void StatsInfo(const StorageStats &s, bool ismem);
    Cache L1, L2, L3;
    Memory memory;
};

struct PIPE_REG_F {
    REG PC;
    PIPE_REG_F() { PC = 0; }
};

struct PIPE_REG_D {
    raw_inst_t inst;
    REG PC;
    PIPE_REG_D() { inst = PC = 0; }
};

struct PIPE_REG_E {
    uint8_t alu_func, rd, opcode, funct3;
    bool cond;
    REG src1, src2, val;
#ifdef PIPE
    REG PC;
    PIPE_REG_E() { alu_func = ALU_NOP; rd = opcode = funct3 = 0; cond = false; src1 = src2 = val = PC = 0; }
#endif
};

struct PIPE_REG_M {
    uint8_t rd, opcode, funct3;
    bool cond;
    REG val, res;
#ifdef PIPE
    REG PC;
    PIPE_REG_M() { rd = opcode = funct3 = 0; cond = false; res = val = PC = 0; }
#endif
};

struct PIPE_REG_W {
    uint8_t rd, opcode;
    bool cond;
    REG val, res;
#ifdef PIPE
    REG PC;
    PIPE_REG_W() { rd = opcode = 0; cond = false; res = val = PC = 0; }
#endif
};

struct ELF_SYMBOL {
    uint8_t bind, type, other;
    unsigned idx;
    ELFIO::Elf64_Addr value;
    ELFIO::Elf_Xword size;
    ELFIO::Elf_Half section_index;
    std::string name;

    ELF_SYMBOL(uint8_t bind, uint8_t type, uint8_t other,
               unsigned idx, ELFIO::Elf64_Addr value, ELFIO::Elf_Xword size, 
               ELFIO::Elf_Half section_index, std::string name);
};

struct Breakpoint {
    bool activated;
    size_t addr;
    std::string literal;

    void enable() { activated = true; }
    void disable() { activated = false; }
    Breakpoint(size_t addr, std::string literal) : addr(addr), literal(literal) 
    { activated = true; }
};

class RISCV_proc {
public:
    void load_prog();
    bool set_entry_symbol(const std::string &symbol);
    bool set_entry_addr(size_t addr);

    RISCV_proc(const ELFIO::elfio &reader, const Config &config);
    ~RISCV_proc();

    void start();

private:
    const Config &config;
    const ELFIO::elfio &elf_reader;
    ELFIO::section *text_sec, *symtab_sec;
    std::vector<ELF_SYMBOL> symtab;

    Breakpoint* curbp;
    std::vector<Breakpoint> breakpoints;
    bool flag_finished, flag_break;
    size_t entry_addr, entry_offset;
    size_t heap, heap_base;
    std::string entry_literal;
    std::stringstream inputstream;

    size_t inst_count, pipe_cycle_count;

    pgtb_t pg_table;
    REG reg_ulong[32];
    CachedStorage storage; // Contains 3-level caches and memory

#ifdef F_EXT
    REG reg_float[32];
#endif

    PIPE_REG_F reg_F;
    PIPE_REG_D reg_D;
    PIPE_REG_E reg_E;
    PIPE_REG_M reg_M;
    PIPE_REG_W reg_W;
#ifdef PIPE
    PIPE_REG_F reg_w;
    PIPE_REG_D reg_f;
    PIPE_REG_E reg_d;
    PIPE_REG_M reg_e;
    PIPE_REG_W reg_m;
    PipeControl ctrl_F, ctrl_D, ctrl_E, ctrl_M, ctrl_W;

    uint8_t mispred;
    void clock_tick();
    void set_pipe_control();
    REG predict_PC(REG thisPC, int imm);
    PIPE_REG_F select_PC();
#endif 
    void reset_cache();
    void print_config();

    void read_memory(char *buf, size_t vaddr, size_t len);
    void write_memory(char *buf, size_t vaddr, size_t len, uint8_t flags); 
    void alloc_page(size_t vaddr, uint8_t flags);

    template<typename T> T memread(size_t vaddr);
    template<typename T> void memwrite(size_t vaddr, T val);
    std::string read_string(size_t addr);

    void load_memory();
    bool get_symbol(const std::string &symbol, ELF_SYMBOL** psym);
    void execute(size_t steps);
    void clear_pg_table();
    void clear_regs();

    void fetch();
    void decode();
    void exec();
    void mem();
    void writeback();

    PIPE_REG_D calc_reg_D();
    PIPE_REG_E calc_reg_E();
    PIPE_REG_M calc_reg_M();
    PIPE_REG_W calc_reg_W();

    void run_simulator();
    void set_breakpoint(const std::string& cmd);
    void status(const std::string& cmd);
    void summary(bool finished = false);
    void shell();
    void exit();
};

#ifndef PIPE    // SEQ
#endif

#endif // RISCV_PROC_HPP

#include <riscv_proc.hpp>
#include <riscv_config.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>
using namespace std;

REG alu_calc(REG src1, REG src2, unsigned ALU_FUNC)
{
    switch (ALU_FUNC) {
    case ALU_ADD:
        return (SREG)src1 + (SREG)src2;
    case ALU_SUB:
        return (SREG)src1 - (SREG)src2;
    case ALU_SLL:
        return src1 << (src2 & 0x3f);
    case ALU_SLT:
        return ((SREG)src1 < (SREG)src2) ? 1 : 0;
    case ALU_SLTU:
        return (src1 < src2) ? 1 : 0;
    case ALU_XOR:
        return src1 ^ src2;
    case ALU_SRL:
        return src1 >> (src2 & 0x3f);
    case ALU_SRA:
        return REG((SREG(src1)) >> (src2 & 0x3f));
    case ALU_OR:
        return src1 | src2;
    case ALU_AND:
        return src1 & src2;
    case ALU_ADDW:
        return REG(SREG(int(src1 + src2)));
    case ALU_SUBW:
        return REG(SREG(int(src1 - src2)));
    case ALU_SLLW:
        return REG(SREG(int(src1 << (src2 & 0x3f))));
    case ALU_SRLW:
        return REG(SREG(int(src1 >> (src2 & 0x3f))));
    case ALU_SRAW:
        return REG(SREG(int((SREG(src1)) >> (src2 & 0x3f))));
    case ALU_MUL:
        // return REG(__int128_t(src1 * src2) & (((__int128_t)1<<(sizeof(REG)*8))-1));
        return REG(SREG(src1) * SREG(src2));
    case ALU_MULH:
        return REG((__int128_t(SREG(src1)) * __int128_t(SREG(src2))) >> (sizeof(REG)*8));
    case ALU_MULHSU:
        return REG((__int128_t(SREG(src1)) * __int128_t(src2)) >> (sizeof(REG)*8));
    case ALU_MULHU:
        return REG((__int128_t(src1) * __int128_t(src2)) >> (sizeof(REG)*8));
    case ALU_MULW:
        return REG(SREG(int(src1) * int(src2)));
    case ALU_DIV:
        if (src2 == 0) return REG(-1);
        return (SREG)src1 / (SREG)src2;
    case ALU_DIVU:
        if (src2 == 0) return REG(-1);
        return src1 / src2;
    case ALU_DIVW:
        if (src2 == 0) return REG(-1);
        return REG(SREG(int(src1) / int(src2)));
    case ALU_DIVUW:
        if (src2 == 0) return REG(-1);
        return REG(SREG(unsigned(src1) / unsigned(src2)));
    case ALU_REM:
        if (src2 == 0) return src1;
        return (SREG)src1 % (SREG)src2;
    case ALU_REMU:
        if (src2 == 0) return src1;
        return src1 % src2;
    case ALU_REMW:
        if (src2 == 0) return (int)src1;
        return REG(SREG(int(src1) % int(src2)));
    case ALU_REMUW:
        if (src2 == 0) return (unsigned)src1;
        return REG(SREG(unsigned(src1) % unsigned(src2)));
    default:
        return 0;
    }
}

string dec2hex(size_t i)
{
    return dec2hex(i, 0);
}

string dec2hex(size_t i, size_t bytes)
{
    stringstream ioss; 
    string s_temp; 
    ioss << hex << i; 
    ioss >> s_temp;
    if (bytes == 0) return s_temp;

    string s(bytes * 2 - s_temp.size(), '0'); 
    s += s_temp; 
    return s;
}

static string reg_format(REG num, size_t bytes)
{
    string full = dec2hex(num, sizeof(REG));
    string res = "";
    if (bytes <= 4) {
        res += full.substr(full.size() - bytes * 2, full.size()) + "\t";
        if (bytes == 1) 
            res += to_string(int(char(num)));
        else if (bytes == 2)
            res += to_string(short(num));
        else if (bytes == 4) 
            res += to_string(int(num));
        return res;
    }
    
    for (int i = bytes / 4; i > 0; i --) {
        size_t start = full.size() - i * 8;
        res += full.substr(start, start+8) + ' ';
    }
    res += "\t" + to_string(SREG(num));
    return res;
}

string& trim(string &s)
{
    if (s.empty()) return s;
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

ostream & operator << (ostream &os, const RISCV_inst &inst) 
{
    os << dec << dec2hex(inst.raw_inst, sizeof(inst.raw_inst)) << "\t";
    switch (inst.type) {
    case IT_R:
        os << R_INST_NAME[inst.subtype.type_r] << " " << R_NAMES[inst.inst.inst_r.rd]
           << "," << R_NAMES[inst.inst.inst_r.rs1] << "," << R_NAMES[inst.inst.inst_r.rs2];
        break;
    case IT_I:
        os << I_INST_NAME[inst.subtype.type_i];
        if (inst.inst.inst_i.opcode == 0x73)
            break;
        os << " " << R_NAMES[inst.inst.inst_i.rd] << ",";
        if (inst.inst.inst_i.opcode == 0x03) 
            os << inst.inst.inst_i.imm << "(" << R_NAMES[inst.inst.inst_i.rs1] << ")";
        else
            os << R_NAMES[inst.inst.inst_i.rs1] << "," << inst.inst.inst_i.imm;
        break;
    case IT_S:
        os << S_INST_NAME[inst.subtype.type_s] << " " << R_NAMES[inst.inst.inst_s.rs2]
           << "," << inst.inst.inst_s.imm << "(" << R_NAMES[inst.inst.inst_s.rs1] << ")";
        break;
    case IT_SB:
        os << SB_INST_NAME[inst.subtype.type_sb] << " " << R_NAMES[inst.inst.inst_sb.rs1]
           << "," << R_NAMES[inst.inst.inst_sb.rs2] << ",";
        if (inst.inst.inst_sb.imm < 0) os << "-0x" << hex << -inst.inst.inst_sb.imm;
        else os << "0x" << hex << inst.inst.inst_sb.imm;
        break;
    case IT_U:
        os << U_INST_NAME[inst.subtype.type_u] << " " << R_NAMES[inst.inst.inst_u.rd]
           << ",0x" << hex << (inst.inst.inst_u.imm >> 12);
        break;
    case IT_UJ:
        os << UJ_INST_NAME[inst.subtype.type_uj] << " " << R_NAMES[inst.inst.inst_uj.rd]
           << ",";
        if (inst.inst.inst_uj.imm < 0) os << "-0x" << hex << -inst.inst.inst_uj.imm;
        else os << "0x" << hex << inst.inst.inst_uj.imm; 
        break;
    default:
        os << "unimp";
    }
}

ELF_SYMBOL::ELF_SYMBOL(uint8_t bind, uint8_t type, uint8_t other,
                       unsigned idx, ELFIO::Elf64_Addr value, ELFIO::Elf_Xword size, 
                       ELFIO::Elf_Half section_index, string name) :
                       bind(bind), type(type), other(other), idx(idx), value(value), 
                       size(size), section_index(section_index), name(name) {}

bool RISCV_proc::get_symbol(const string &symbol, ELF_SYMBOL** psym) 
{
    for (auto &sym : symtab) {
        if (sym.name.compare(symbol) == 0) {
            *psym = &sym;
            return true;
        }
    }
    return false;
}

void RISCV_proc::load_prog() 
{
    cout << "Loading ELF program into RISC-V simulator memory..." << endl;
    ELFIO::Elf_Half sec_num = elf_reader.sections.size();
    ELFIO::section *psec;
    cout << "Reading ELF sections : ";

    for (int i = 0; i < sec_num; i++) {
        psec = elf_reader.sections[i];
        cout << " [" << i << "] " << psec->get_name() << "\tflags: " << psec->get_flags() << "\t";
        cout << "Addr: " << hex << "0x" << psec->get_address() << "\tSize: " << dec << psec->get_size() << endl;
        if (psec->get_name().compare(".text") == 0)
            text_sec = psec;
        if (psec->get_type() == SHT_SYMTAB)
            symtab_sec = psec;
    }

    cout << "Loading .symtab ... ";
    const ELFIO::symbol_section_accessor symbols(elf_reader, symtab_sec);
    symtab.clear();
    symtab.reserve(symbols.get_symbols_num());
    for (unsigned int j = 0; j < symbols.get_symbols_num(); j++) {
        string name;
        ELFIO::Elf64_Addr value;
        ELFIO::Elf_Xword size;
        ELFIO::Elf_Half section_index;
        uint8_t bind, type, other;
        symbols.get_symbol(j, name, value, size, bind,
                           type, section_index, other);
        symtab.push_back(ELF_SYMBOL(bind, type, other, j, value, size, section_index, name));
    }
    cout << "Loaded." << endl;

    // load_memory();
    bool entry_setted = false;
    if (config.set_entry_symbol || config.set_entry_addr) {
        if (config.set_entry_symbol) entry_setted = set_entry_symbol(config.entry_symbol);
        else entry_setted = set_entry_addr(config.entry_addr);
        if (!entry_setted) cout << "Entry point cannot be overwrite! Using default." << endl;
    }
    else {
        entry_addr = elf_reader.get_entry();
        entry_literal = "<";
        for (auto &sym : symtab) 
            if (sym.type == STT_FUNC && sym.value == entry_addr) {
                entry_literal += sym.name + ", ";
                break;
            }
        entry_literal += "0x" + dec2hex(entry_addr) + ">";
    }
    cout << "Entry point setted at " << entry_literal << "." << endl;
}

void RISCV_proc::load_memory()
{
    ELFIO::Elf_Half seg_num = elf_reader.segments.size();
    ELFIO::segment *pseg;
    cout << "Loading ELF segments into user memory space ...";
    for (int i = 0; i < seg_num; i++) {
        pseg = elf_reader.segments[i];
        if (pseg->get_type() == PT_LOAD) { // Alloc: memcpy
            uint8_t flags = 0;
            if (pseg->get_flags() & PF_X) flags |= PTE_X;
            if (pseg->get_flags() & PF_W) flags |= PTE_W;
            write_memory(pseg->get_data(), pseg->get_virtual_address(), pseg->get_file_size(), flags);
            write_memory(NULL, pseg->get_virtual_address() + pseg->get_file_size(), 
                         pseg->get_memory_size() - pseg->get_file_size(), flags);
            if (pseg->get_virtual_address() + pseg->get_memory_size() >= heap)
                heap = ROUND_UP(pseg->get_virtual_address() + pseg->get_memory_size(), PGSIZE);
        }
    }
    cout << "Loaded." << endl;
}

bool RISCV_proc::set_entry_symbol(const string &symbol) 
{
    bool setted = false;
    ELF_SYMBOL* psym = NULL;
    cout << "Finding " << symbol << " in .symtab : ";

    if (get_symbol(symbol, &psym)) {
        setted = true;
        cout << "[" << psym->idx << "] " << psym->name << "\tAddr: 0x" << hex << psym->value;
        entry_addr = psym->value;
        entry_offset = psym->value - text_sec->get_address();
        cout << "\tOffset: 0x" << entry_offset << endl;

        entry_literal = symbol;
        entry_literal += ", 0x" + dec2hex(entry_addr);
        return true;
    }
    return false;
}

bool RISCV_proc::set_entry_addr(size_t addr) 
{
    if (addr >= config.max_memory_addr) return false;
    if (!PG_ALLOC(pg_table[PAGE(addr)]) || !PG_EXEC(pg_table[PAGE(addr)])) return false;
    entry_addr = addr;
    entry_literal = "0x" + dec2hex(addr);
    return true;
}

RISCV_proc::RISCV_proc(const ELFIO::elfio &reader, const Config &config) 
    : elf_reader(reader), config(config)
{
    memset(reg_ulong, 32, sizeof(reg_ulong));
}

RISCV_proc::~RISCV_proc() 
{
    clear_pg_table();
}

void RISCV_proc::read_memory(char *buf, size_t vaddr, size_t len)
{
    size_t curpg, start = vaddr, end = vaddr + len;
    while (vaddr < end) {
        curpg = PAGE(vaddr);
        pte_t &pte = pg_table[curpg];
        if (!PG_ALLOC(pte)) cout << dec2hex(vaddr) << endl;
        assert(PG_ALLOC(pte));

        size_t copy_size = min(curpg + PGSIZE - vaddr, end - vaddr);
        memcpy(buf + vaddr - start, (char *)(pte.pg) + (vaddr - curpg), copy_size);
        vaddr += copy_size;
    }
}

void RISCV_proc::write_memory(const char *buf, size_t vaddr, size_t len, uint8_t flags = 0)
{
    size_t curpg, start = vaddr, end = vaddr + len;
    while (vaddr < end) {
        curpg = PAGE(vaddr);
        pte_t &pte = pg_table[curpg];
        if (flags) alloc_page(curpg, flags);
        else assert(PG_ALLOC(pte) && PG_WRITE(pte));

        size_t copy_size = min(curpg + PGSIZE - vaddr, end - vaddr);
        if (buf)
            memcpy((char *)(pte.pg) + (vaddr - curpg), buf + vaddr - start, copy_size);
        else
            memset((char *)(pte.pg) + (vaddr - curpg), 0, copy_size);
        vaddr += copy_size;
    }
}

void RISCV_proc::alloc_page(size_t vaddr, uint8_t flags)
{
    pte_t &pte = pg_table[PAGE(vaddr)];
    if (PG_ALLOC(pte)) return;
    pte.pg = malloc(PGSIZE);
    pte.flags = (flags | PTE_P);
}

template<typename T> T RISCV_proc::memread(size_t vaddr)
{
    if (!vaddr) return 0;
    T res;
    read_memory((char *)&res, vaddr, sizeof(T));
    return res;
}

template<typename T> void RISCV_proc::memwrite(size_t vaddr, T val)
{
    write_memory((char *)&val, vaddr, sizeof(T));
}

string RISCV_proc::read_string(size_t addr)
{
    string res("");
    char ch;
    while ((ch = memread<char>(addr)) != '\0') {
        res += ch;
        addr += sizeof(char);
    }
    return res;
}


void RISCV_proc::fetch()
{
#ifndef PIPE
    reg_D = calc_reg_D();
#else 
    reg_f = calc_reg_D();
#endif
}

void RISCV_proc::decode()
{
#ifndef PIPE
    reg_E = calc_reg_E();
#else 
    reg_d = calc_reg_E();
#endif
}

void RISCV_proc::exec()
{
#ifndef PIPE
    reg_M = calc_reg_M();
#else 
    reg_e = calc_reg_M();
#endif
}

void RISCV_proc::mem()
{
#ifndef PIPE
    reg_W = calc_reg_W();
#else 
    reg_m = calc_reg_W();
#endif
}

void RISCV_proc::writeback()
{
    bool setPC = false;
    uint8_t opcode = reg_W.opcode;
    if (opcode == 0x33 || opcode == 0x03 || opcode == 0x13 || opcode == 0x3b ||
        opcode == 0x17 || opcode == 0x37 || opcode == 0x1b) {
        if (reg_W.rd != R_ZERO) reg_ulong[reg_W.rd] = reg_W.res;
        if (reg_W.rd == R_SP) {
            if (reg_W.res < config.heap_max) {
                cout << dec2hex(reg_W.res) << endl;
                cout << config.heap_max << endl;
                cout << dec2hex(reg_F.PC) << endl;
            }
            assert(reg_W.res >= config.heap_max);     // stack overflow
            alloc_page(reg_W.res, PTE_W);
        }
    }     
    else if (opcode == 0x67 || opcode == 0x6f) { 
        if (reg_W.rd != R_ZERO) reg_ulong[reg_W.rd] = reg_W.val;
        if (reg_W.rd == R_SP) {
            assert(reg_W.res >= config.heap_max);     // stack overflow
            alloc_page(reg_W.val, PTE_W);
        }
        setPC = true;
    }
    else if (opcode == 0x63 && reg_W.cond)
        setPC = true;
    else if (opcode == 0x73) {  // SYSCALL
        reg_ulong[reg_W.rd] = 0;
        pipe_cycle_count += config.latency.ecall;
        switch (reg_W.val) {
        case SYS_PRINT_I:
            cout << "<stdout> " << dec << (long long) reg_W.res << endl;
            break;
        case SYS_PRINT_C:
            cout << "<stdout> " << (char) reg_W.res << endl;
            break;
        case SYS_PRINT_S: {
            string s = read_string(reg_W.res);
            cout << "<stdout> " << s << endl;
        }   break;
        case SYS_READ_I: {
            long long x;
            if (inputstream.rdbuf()->in_avail() == 0) {
                cout << "<stdin> Waiting input: " << endl;
                inputstream.clear();
                inputstream.str();
                string s;
                getline(cin, s);
                inputstream << s;
            }
            inputstream >> x;
            reg_ulong[reg_W.rd] = REG(x);
        }   break;
        case SYS_READ_C: {
            char c;
            if (inputstream.rdbuf()->in_avail() == 0) {
                cout << "<stdin> Waiting input: " << endl;
                inputstream.clear();
                inputstream.str();
                string s;
                getline(cin, s);
                inputstream << s;
            }
            inputstream >> c;
            reg_ulong[reg_W.rd] = REG(SREG(c));
        }   break;
        case SYS_SBRK: {
            size_t old_heap = heap;
            while (heap + PGSIZE < old_heap + reg_W.res) {
                heap += PGSIZE;
                assert(heap < config.heap_max);     // heap overflow
                alloc_page(heap, PTE_W);
            }
            heap = old_heap + reg_W.res;
            assert(heap < config.heap_max);     // heap overflow
            alloc_page(heap, PTE_W);
            reg_ulong[reg_W.rd] = REG(old_heap);
        }   break;
        case SYS_HEAP_LO: 
            reg_ulong[reg_W.rd] = config.heap_base;
            break;
        case SYS_HEAP_HI:
            reg_ulong[reg_W.rd] = config.heap_max;
            break;
        case SYS_EXIT:
            flag_finished = true;
            reg_ulong[reg_W.rd] = reg_W.res;
            break;
        }
    }
#ifndef PIPE
    if (setPC)
        reg_F.PC = reg_W.res;
#endif
}   

#ifdef PIPE
REG RISCV_proc::predict_PC(REG thisPC, int imm)
{
    if (!config.branch_prediction.compare("always")) {
        return thisPC + imm;
    }
    else if (!config.branch_prediction.compare("ftbnt")) {
        if (imm > 0) return thisPC + imm;
        else return thisPC + sizeof(raw_inst_t);
    }
    else if (!config.branch_prediction.compare("btfnt")) {
        if (imm < 0) return thisPC + imm;
        else return thisPC + sizeof(raw_inst_t);
    }
    return thisPC + sizeof(raw_inst_t);
}

void RISCV_proc::clock_tick()
{
    CLOCK_TICK(w, F);
    CLOCK_TICK(f, D);
    CLOCK_TICK(d, E);
    CLOCK_TICK(e, M);
    CLOCK_TICK(m, W);
    pipe_cycle_count++;
}

void RISCV_proc::set_pipe_control()
{
    ctrl_F = ctrl_D = ctrl_E = ctrl_M = ctrl_W = PCTRL_NORMAL;
    mispred = 0;
    if (reg_E.opcode == 0x73 || reg_M.opcode == 0x73 || reg_W.opcode == 0x73) {     // ECALL
        ctrl_E = PCTRL_BUBBLE;
        ctrl_D = PCTRL_STALL;
        ctrl_F = PCTRL_STALL;
    }
    if (reg_E.opcode == 0x03 && (RS1(reg_D.inst) == reg_E.rd || RS2(reg_D.inst) == reg_E.rd)) {   // LXX
        ctrl_E = PCTRL_BUBBLE;
        ctrl_D = PCTRL_STALL;
        ctrl_F = PCTRL_STALL;
    } 
    else if (OPCODE(reg_D.inst) == 0x67) {   // JALR
        ctrl_F = PCTRL_NORMAL;
        ctrl_D = PCTRL_BUBBLE;
    }
    if (reg_E.opcode == 0x63) {             // BXX
        if ((reg_E.cond && (predict_PC(reg_E.src1, reg_E.src2) != (SREG)reg_E.src1 + (SREG)reg_E.src2)) || 
            (!reg_E.cond && (predict_PC(reg_E.src1, reg_E.src2) == (SREG)reg_E.src1 + (SREG)reg_E.src2))) {
                ctrl_E = PCTRL_BUBBLE;
                ctrl_D = PCTRL_BUBBLE;
                ctrl_F = PCTRL_NORMAL;
                if (reg_E.cond) mispred = 1;
                else mispred = 2;
            }
    }
    
}

PIPE_REG_F RISCV_proc::select_PC()
{
    PIPE_REG_F result;
    if (mispred) {
        if (mispred == 1) result.PC = (SREG)reg_E.src1 + (SREG)reg_E.src2;
        else result.PC = reg_e.PC + sizeof(raw_inst_t);
        return result;
    }
    if (reg_d.opcode == 0x67) {   // JALR in D
        result.PC = reg_d.src1 + reg_d.src2;
        return result;
    }
    result.PC = reg_F.PC;
    switch (OPCODE(reg_f.inst)) {
    case 0x6f:  // JAL
        result.PC = reg_F.PC + UJ_IMM(reg_f.inst);
        break;
    case 0x63:  // BXX
        result.PC = predict_PC(reg_F.PC, SB_IMM(reg_f.inst));
        break;
    case 0x67:  // JALR
    default:
        result.PC = reg_F.PC + sizeof(raw_inst_t);
        break;
    }
    return result;
}

void RISCV_proc::execute(size_t steps) 
{
    size_t s = 0;
    do {
        s++;
        for (auto &bp : breakpoints) 
            if (bp.addr == reg_W.PC && bp.activated) {
                cout << "Breakpoint at " << bp.literal << endl;
                curbp = &bp; 
                bp.disable();
                return;
            }
        if (curbp != nullptr) {
            curbp->enable();
            curbp = nullptr;
        }

        // Should in fact happen in parallel
        // backward for data-forwarding to work correctly
        writeback();
        mem();
        exec();
        decode();
        fetch();
        reg_w = select_PC();

        // Update the PIPE regs
        clock_tick();
        if (!reg_W.PC) s--;

        // Set PIPE controls
        set_pipe_control();

        if (s == steps) break;
    } while (!flag_finished);
    inst_count += s;
}
#else 
void RISCV_proc::execute(size_t steps) 
{
    size_t s = 0;
    do {
        s++;
        for (auto &bp : breakpoints) 
            if (bp.addr == reg_F.PC && bp.activated) {
                cout << "Breakpoint at " << bp.literal << endl;
                curbp = &bp; 
                bp.disable();
                return;
            }
        if (curbp != nullptr) {
            curbp->enable();
            curbp = nullptr;
        }
        fetch();
        decode();
        exec();
        mem();
        writeback();
        pipe_cycle_count += 5;
        if (s == steps) break;
    } while (!flag_finished);
    inst_count += s;
}
#endif

void RISCV_proc::run_simulator()
{
    string shell_prompt = "> ";
    string cmd, tmp;
    int steps;

    clear_regs();
    inputstream.str("");
    inputstream.clear();
    breakpoints.clear();
    curbp = nullptr;

    clear_pg_table();
    reg_F.PC = entry_addr;
    reg_ulong[R_SP] = config.max_memory_addr - 8;   // stack
    heap = config.heap_base;                        // heap
    load_memory();
    heap_base = heap;
    alloc_page(reg_ulong[R_SP], PTE_W);
    alloc_page(heap, PTE_W);

#ifdef PIPE
    pipe_cycle_count = 0;
#endif
    inst_count = 0;
    flag_finished = flag_break = false;
    
    cout << "Started at " << entry_literal << ": ";
    cout << RISCV_inst(memread<raw_inst_t>(reg_F.PC)) << endl;
#ifdef PIPE
    // Warmup
    set_pipe_control();
    execute(1);
#endif
    while (1) {
        if (flag_finished) {
            cout << "Program exited with code: " << reg_ulong[R_A5] << endl;
            break;
        }
#ifdef PIPE
        REG pc = reg_W.PC;
        if (!PG_EXEC(pg_table[PAGE(pc)])) {
            cout << "Fatal: PC Encountered unexecutable memory address! Execution stopped." << endl;
            break;
        }
        cout << "next inst.: <0x" << dec2hex(pc) << ">\t" << RISCV_inst(memread<raw_inst_t>(pc)) << endl;
#else
        if (!PG_EXEC(pg_table[PAGE(reg_F.PC)])) {
            cout << "Fatal: PC Encountered unexecutable memory address! Execution stopped." << endl;
            break;
        }
        cout << "next inst.: <0x" << dec2hex(reg_F.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_F.PC)) << endl;
#endif
        cout << shell_prompt;

        tmp = cmd;
        getline(cin, cmd);
        if (trim(cmd).empty()) {
            cmd = tmp;
            cout << "Repeating command: " << cmd << endl;
        }
            
        if (cmd[0] == 's') {
            if (cmd[1] >= '0' && cmd[1] <= '9')
                steps = stoi(cmd.substr(1, cmd.size()));
            else steps = 1;
            execute(steps);
        }
#ifdef PIPE
        else if (cmd[0] == 'p') {
            cout << "Pipeline Status: " << endl;
            cout << "Fetch: <0x" << dec2hex(reg_F.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_F.PC)) << endl;
            cout << "Decode:<0x" << dec2hex(reg_D.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_D.PC)) << endl;
            cout << "Exec:  <0x" << dec2hex(reg_E.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_E.PC)) << endl;
            cout << "Mem:   <0x" << dec2hex(reg_M.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_M.PC)) << endl;
            cout << "WB:    <0x" << dec2hex(reg_W.PC) << ">\t" << RISCV_inst(memread<raw_inst_t>(reg_W.PC)) << endl;
        }
#endif
        else if (cmd[0] == 'c') execute(0);
        else if (cmd[0] == 'k') {
            cout << "Program killed!" << endl;
            break;
        }
        else if (cmd[0] == 'b') set_breakpoint(cmd);
        else if (cmd[0] == 'i') summary();
        else if (cmd[0] == 'd' || cmd[0] == 'e') {
            int bpid;
            try {
                bpid = stoi(cmd.substr(1, cmd.size()));
            } catch (invalid_argument) {
                cout << "Error: Unidentified breakpoint id." << endl;
                return;
            }
            if (bpid <= 0 || bpid > breakpoints.size()) {
                cout << dec << "No breakpoint with id: " << bpid << endl;
                return;
            }
            if (cmd[0] == 'e') {
                breakpoints[bpid-1].enable();
                cout << "Breakpoint [" << bpid << "] at " << breakpoints[bpid-1].literal;
                cout << " enabled." << endl;
            }
            else if (cmd[0] == 'd') {
                breakpoints[bpid-1].disable();
                if (curbp == &breakpoints[bpid-1]) curbp = nullptr;
                cout << "Breakpoint [" << bpid << "] at " << breakpoints[bpid-1].literal;
                cout << " disabled." << endl;
            }
        }
        else status(cmd);
    }
    summary(true);
}

void RISCV_proc::clear_pg_table() 
{
    pgtb_t::iterator it;
    for (it = pg_table.begin(); it != pg_table.end(); it++) {
        if (it->second.flags & PTE_P)
            free(it->second.pg);
    }
    pg_table.clear();
}

void RISCV_proc::clear_regs()
{
    memset(reg_ulong, 0, sizeof(reg_ulong));
    reg_F = PIPE_REG_F();
    reg_D = PIPE_REG_D();
    reg_E = PIPE_REG_E();
    reg_M = PIPE_REG_M();
    reg_W = PIPE_REG_W(); 
}

void RISCV_proc::start()
{
    load_prog();
    cout << endl;
    cout << "Welcome to MarkYu's RISC-V simulator!" << endl;
    shell();
    exit();
}

void RISCV_proc::shell()
{   
    string shell_prompt = "RISCV-sim > ";
    string cmd;
    cout << "Enter 'r' to run the loaded program, enter 'q' to quit" << endl;
    while (1) {
        cout << shell_prompt;
        getline(cin, cmd);
        if (cmd[0] == 'q') break;
        else if (cmd[0] == 'r') run_simulator();
        else status(cmd);
    }
}

void RISCV_proc::set_breakpoint(const string& cmd)
{
    if (cmd.size() <= 2) {
        cout << "Set breakpoint: \"b [address]\" or \"b *[symbol]\"." << endl;
        return;
    }
    size_t addr;
    stringstream ss;
    string addr_s;
    
    if (cmd[2] == '*') {
        ELF_SYMBOL* psym;
        ss << cmd.substr(3, cmd.size());
        ss >> addr_s;

        if (get_symbol(addr_s, &psym)) {
            if (psym->type != STT_FUNC) {
                cout << "Error: symbol type not allowed: " << addr_s << endl;
                return;
            }
            addr = psym->value;
        }
        else {
            cout << "Error: symbol not found: " << addr_s << endl;
            return;
        }
        Breakpoint bp = Breakpoint(addr, "<"+addr_s+", 0x"+ dec2hex(addr)+">");
        breakpoints.push_back(bp);
        cout << "Breakpoint [" << dec << breakpoints.size() << "] set at " << bp.literal << endl;
    }
    else {
        ss << cmd.substr(2, cmd.size());
        ss >> addr_s;
        try {
            addr = ROUND_UP(stol(addr_s, 0, 0), sizeof(raw_inst_t));
        } catch (invalid_argument) {
            cout << "Error: Unidentified breakpoint address." << endl;
            return;
        }
        if (addr < 0 || addr >= config.max_memory_addr) 
            cout << "Breakpoint: address out of range." << endl;
        else {
            if (!PG_ALLOC(pg_table[PAGE(addr)]) || !PG_EXEC(pg_table[PAGE(addr)]))
                cout << "Breakpoint: address not executable, cannot set there." << endl;
            else {
                Breakpoint bp = Breakpoint(addr, "<0x"+ dec2hex(addr)+">");
                breakpoints.push_back(bp);
                cout << "Breakpoint [" << dec << breakpoints.size() << "] set at " << bp.literal << endl;
            }
        }
    }
}

void RISCV_proc::status(const string& cmd)
{
    int bytes = sizeof(long long);
    if (cmd.size() <= 3) {
        cout << "Unrecognized command: " << cmd << endl;
        return;
    }
    if (cmd[3] == 'b') bytes = sizeof(char);
    else if (cmd[3] == 'h') bytes = sizeof(short);
    else if (cmd[3] == 'w') bytes = sizeof(int);
    else if (cmd[3] == 'i') bytes = 0;

    if (cmd[0] == 'x' && cmd[1] == 'r' && cmd[2] == '/') {  
        if (cmd.size() > 5 && cmd[4] == ' ' && cmd[5] >= '0' && cmd[5] <= '9') {
            int reg_num;
            try {
                reg_num = stoi(cmd.substr(5, cmd.size()), 0, 0);
            } catch (invalid_argument) {
                cout << "Error: Unidentified register number." << endl;
            }
            if (reg_num < 0 || reg_num > 31)
                cout << "Error: Wrong register number." << endl;
            else {
                cout << dec << reg_num << "\t" << R_NAMES[reg_num] << ":\t";
                cout << reg_format(reg_ulong[reg_num], bytes) << endl;   
            }
        }
        else {
            for (int i = 0; i < 32; i++) {
                cout << i << "\t" << R_NAMES[i] << ":\t";
                cout << reg_format(reg_ulong[i], bytes) << endl;  
            }
        }
    }
    else if (cmd[0] == 'x' && (cmd[1] == 'm' || cmd[1] == 's') && cmd[2] == '/') {  
        if (cmd.size() > 5 && cmd[4] == ' ') {
            stringstream ss;
            string addr_s;
            size_t addr, count = 1;
            REG dword;

            if (cmd[1] == 'm' && cmd[5] == '*') {    // addr in register
                int reg_num;
                ss << cmd.substr(6, cmd.size());
                ss >> addr_s >> count;
                try {
                    reg_num = stol(addr_s, 0, 0);
                } catch (invalid_argument) {
                    cout << "Error: Unidentified register number." << endl;
                    return;
                }
                if (reg_num < 0 || reg_num > 31) {
                    cout << "Error: Wrong register number." << endl;
                    return;
                }
                addr = reg_ulong[reg_num];
            }
            else if (cmd[1] == 'm') {
                ss << cmd.substr(5, cmd.size());
                ss >> addr_s >> count;
                try {
                    addr = ROUND_DOWN(stol(addr_s, 0, 0), bytes ? bytes : sizeof(raw_inst_t));
                } catch (invalid_argument) {
                    cout << "Error: Unidentified memory address." << endl;
                    return;
                }
            }
            else if (cmd[1] == 's') {   // symbol
                ELF_SYMBOL* psym;
                ss << cmd.substr(5, cmd.size());
                ss >> addr_s >> count;

                if (get_symbol(addr_s, &psym)) {
                    if (psym->type != STT_OBJECT) {
                        cout << "Error: Symbol type not allowed: " << addr_s << endl;
                        return;
                    }
                    addr = psym->value;
                }
                else {
                    cout << "Error: Symbol not found: " << addr_s << endl;
                    return;
                }
            }
            for (size_t i = 0; i < count; i += (bytes ? bytes : 1)) {
                if (addr < 0 || addr > config.max_memory_addr - bytes || 
                    (bytes == 0 && addr > config.max_memory_addr - sizeof(raw_inst_t))) {
                    cout << "Error: Memory address out of range." << endl;
                    break;
                }
                else if (!PG_ALLOC(pg_table[PAGE(addr)])) {
                    cout << "Error: Address in unallocated page." << endl;
                    break;
                }
                else {
                    cout << dec2hex(addr, sizeof(size_t)) << ":\t";
                    if (bytes == 0) {   // 'i'
                        if (!PG_EXEC(pg_table[PAGE(addr)])) {
                            cout << "Error: Address not executable." << endl;
                            break;
                        }
                        cout << RISCV_inst(memread<raw_inst_t>(addr)) << endl;
                        addr += sizeof(raw_inst_t);
                        continue;
                    }
                    if (bytes == sizeof(char)) dword = memread<char>(addr);
                    else if (bytes == sizeof(short)) dword = memread<short>(addr);
                    else if (bytes == sizeof(int)) dword = memread<int>(addr);
                    else if (bytes == sizeof(int64_t)) dword = memread<int64_t>(addr);
                    cout << reg_format(dword, bytes) << endl;   
                }
                addr += bytes;
            }
        }
    }
    else cout << "Unrecognized command: " << cmd << endl;
    return;
}

void RISCV_proc::summary(bool finished)
{
    if (finished)
        cout << endl << "================SUMMARY================" << endl;
    cout << "Total instructions executed: " << dec << inst_count << endl;
#ifdef PIPE
    cout << "Total Pipeline cycle: " << dec << pipe_cycle_count << endl;
#endif
}

void RISCV_proc::exit()
{
    cout << "Exiting MarkYu's RISC-V simulator, bye!" << endl;
}

PIPE_REG_D RISCV_proc::calc_reg_D()
{
    PIPE_REG_D result;
    raw_inst_t inst = memread<raw_inst_t>(reg_F.PC);
    result.inst = inst;
    result.PC = reg_F.PC;
#ifndef PIPE
    reg_F.PC += sizeof(raw_inst_t);
#endif
    return result;
}

PIPE_REG_E RISCV_proc::calc_reg_E()
{
    PIPE_REG_E result;
    raw_inst_t raw_inst = reg_D.inst;
    RISCV_inst riscv_inst = RISCV_inst(raw_inst);
#ifdef PIPE
    result.PC = reg_D.PC;
#endif
    result.opcode = OPCODE(riscv_inst.raw_inst);
    result.funct3 = FUNCT3(riscv_inst.raw_inst);
    switch (riscv_inst.type) {
    case IT_R: {
        const uint8_t &rs1 = riscv_inst.inst.inst_r.rs1;
        const uint8_t &rs2 = riscv_inst.inst.inst_r.rs2;
        const uint8_t &rd = riscv_inst.inst.inst_r.rd;
        result.alu_func = (uint8_t)(riscv_inst.subtype.type_r);
        result.rd = rd;
        result.src1 = reg_ulong[rs1];
        result.src2 = reg_ulong[rs2];
#ifdef PIPE
        if (rs1 && rs1 == reg_e.rd) result.src1 = DATA_FORWARD(reg_e); 
        else if (rs1 && rs1 == reg_m.rd) result.src1 = DATA_FORWARD(reg_m);
        if (rs2 && rs2 == reg_e.rd) result.src2 = DATA_FORWARD(reg_e); 
        else if (rs2 && rs2 == reg_m.rd) result.src2 = DATA_FORWARD(reg_m);
#endif
    }   break;
    case IT_I: {
        const uint8_t &opcode = riscv_inst.inst.inst_i.opcode;
        const uint8_t &rs1 = riscv_inst.inst.inst_i.rs1;
        const I_INST_TYPE &type_i = riscv_inst.subtype.type_i;
        result.rd = riscv_inst.inst.inst_i.rd;
        result.src1 = reg_ulong[rs1];
        result.src2 = riscv_inst.inst.inst_i.imm;
        if (opcode == 0x03) result.alu_func = ALU_ADD;
        else if (type_i == I_ADDI) result.alu_func = ALU_ADD;
        else if (type_i == I_SLLI) result.alu_func = ALU_SLL;
        else if (type_i == I_SLTI) result.alu_func = ALU_SLT;
        else if (type_i == I_SLTIU) result.alu_func = ALU_SLTU;
        else if (type_i == I_XORI) result.alu_func = ALU_XOR;
        else if (type_i == I_SRLI) result.alu_func = ALU_SRL;
        else if (type_i == I_SRAI) result.alu_func = ALU_SRA;
        else if (type_i == I_ORI)  result.alu_func = ALU_OR;
        else if (type_i == I_ANDI) result.alu_func = ALU_AND;
        else if (type_i == I_ADDIW) result.alu_func = ALU_ADDW;
        else if (type_i == I_SLLIW) result.alu_func = ALU_SLLW;
        else if (type_i == I_SRLIW) result.alu_func = ALU_SRLW;
        else if (type_i == I_SRAIW) result.alu_func = ALU_SRAW;
        else if (type_i == I_JALR) { result.alu_func = ALU_ADD; result.val = reg_D.PC + sizeof(raw_inst_t); }
        else if (type_i == I_ECALL) { 
            result.src1 = reg_ulong[R_A0]; result.src2 = reg_ulong[R_A7]; 
            result.alu_func = ALU_NOP; result.rd = R_A5;
        }
        else result.alu_func = ALU_NOP;
#ifdef PIPE
        if (rs1 && rs1 == reg_e.rd) result.src1 = DATA_FORWARD(reg_e); 
        else if (rs1 && rs1 == reg_m.rd) result.src1 = DATA_FORWARD(reg_m);
        if (type_i == I_ECALL) {
            if (reg_e.rd == R_A0) result.src1 = DATA_FORWARD(reg_e); 
            else if (reg_m.rd == R_A0) result.src1 = DATA_FORWARD(reg_m); 
            if (reg_e.rd == R_A7) result.src2 = DATA_FORWARD(reg_e); 
            else if (reg_m.rd == R_A7) result.src2 = DATA_FORWARD(reg_m); 
        }
#endif
    }   break;
    case IT_S: {
        const uint8_t &rs1 = riscv_inst.inst.inst_s.rs1;
        const uint8_t &rs2 = riscv_inst.inst.inst_s.rs2;
        result.val = reg_ulong[rs2];
        result.src1 = reg_ulong[rs1];
        result.src2 = riscv_inst.inst.inst_s.imm;
#ifdef PIPE
        if (rs1 && rs1 == reg_e.rd) result.src1 = DATA_FORWARD(reg_e); 
        else if (rs1 && rs1 == reg_m.rd) result.src1 = DATA_FORWARD(reg_m);
        if (rs2 && rs2 == reg_e.rd) result.val = DATA_FORWARD(reg_e); 
        else if (rs2 && rs2 == reg_m.rd) result.val = DATA_FORWARD(reg_m);
#endif
        if (riscv_inst.subtype.type_s != S_UNIMP) result.alu_func = ALU_ADD;
        else result.alu_func = ALU_NOP;
    }   break;
    case IT_SB: {
        const SB_INST_TYPE &type_sb = riscv_inst.subtype.type_sb;
        const uint8_t &rs1 = riscv_inst.inst.inst_sb.rs1;
        const uint8_t &rs2 = riscv_inst.inst.inst_sb.rs2;
        result.src1 = reg_D.PC;
        result.src2 = riscv_inst.inst.inst_sb.imm;
        result.alu_func = ALU_ADD;
        REG r1 = reg_ulong[rs1], r2 = reg_ulong[rs2];
#ifdef PIPE
        if (rs1 && rs1 == reg_e.rd) r1 = DATA_FORWARD(reg_e); 
        else if (rs1 && rs1 == reg_m.rd) r1 = DATA_FORWARD(reg_m);
        if (rs2 && rs2 == reg_e.rd) r2 = DATA_FORWARD(reg_e); 
        else if (rs2 && rs2 == reg_m.rd) r2 = DATA_FORWARD(reg_m);
#endif
        if (type_sb == SB_BEQ) result.cond = (r1 == r2);
        else if (type_sb == SB_BNE) result.cond = (r1 != r2);
        else if (type_sb == SB_BLT) result.cond = ((SREG)r1 < (SREG)r2);
        else if (type_sb == SB_BGE) result.cond = ((SREG)r1 >= (SREG)r2);
        else if (type_sb == SB_BLTU) result.cond = (r1 < r2);
        else if (type_sb == SB_BGEU) result.cond = (r1 >= r2);
    }   break;
    case IT_U: {
        const U_INST_TYPE &type_u = riscv_inst.subtype.type_u;
        result.rd = riscv_inst.inst.inst_u.rd;
        result.alu_func = ALU_ADD;
        result.src1 = (type_u == U_AUPIC) ? reg_D.PC : 0;
        result.src2 = riscv_inst.inst.inst_u.imm;
    }   break;
    case IT_UJ: {
        result.rd = riscv_inst.inst.inst_u.rd;
        result.alu_func = ALU_ADD;
        result.src1 = reg_D.PC;
        result.src2 = riscv_inst.inst.inst_u.imm;
        result.val = reg_D.PC + sizeof(raw_inst_t);
    }   break;
    }
    return result;
}

PIPE_REG_M RISCV_proc::calc_reg_M()
{
    PIPE_REG_M result;
#ifdef PIPE
    result.PC = reg_E.PC;
#endif
    result.opcode = reg_E.opcode;
    result.funct3 = reg_E.funct3;
    if (reg_E.opcode == 0x73) { // ECALL
        // cout << "SYSCALL" << endl;
        result.res = reg_E.src1;
        result.val = reg_E.src2;
    }
    else {
        result.res = alu_calc(reg_E.src1, reg_E.src2, reg_E.alu_func);
        result.val = reg_E.val;
        if (reg_E.alu_func >= ALU_MUL && reg_E.alu_func <= ALU_MULHU)
            pipe_cycle_count += config.latency.mul - 1;
        else if (reg_E.alu_func >= ALU_DIV && reg_E.alu_func <= ALU_REMU)
            pipe_cycle_count += config.latency.div - 1;
        else if (reg_E.alu_func == ALU_MULW)
            pipe_cycle_count += config.latency.mulw - 1;
        else if (reg_E.alu_func >= ALU_DIVW && reg_E.alu_func <= ALU_REMUW)
            pipe_cycle_count += config.latency.divw - 1;
    }
    result.rd = reg_E.rd;
    result.cond = reg_E.cond;
    return result;
}

PIPE_REG_W RISCV_proc::calc_reg_W()
{
    PIPE_REG_W result;
#ifdef PIPE
    result.PC = reg_M.PC;
#endif
    result.opcode = reg_M.opcode;
    REG res = reg_M.res;
    REG val = reg_M.val;
    result.rd = reg_M.rd;
    result.cond = reg_M.cond;
    if (reg_M.opcode == 0x03) {
        if (reg_M.funct3 == 0x0) result.res = REG(SREG(memread<char>(res)));
        else if (reg_M.funct3 == 0x1) result.res = REG(SREG(memread<short>(res)));
        else if (reg_M.funct3 == 0x2) result.res = REG(SREG(memread<int>(res)));
        else if (reg_M.funct3 == 0x3) result.res = REG(SREG(memread<int64_t>(res)));
        else if (reg_M.funct3 == 0x4) result.res = REG(memread<uint8_t>(res));
        else if (reg_M.funct3 == 0x5) result.res = REG(memread<uint16_t>(res));
        else if (reg_M.funct3 == 0x6) result.res = REG(memread<uint32_t>(res));
    }
    else if (reg_M.opcode == 0x23) {
        if (reg_M.funct3 == 0x0) memwrite(res, char(val & 0xff));
        else if (reg_M.funct3 == 0x1) memwrite(res, short(val & 0xffff));
        else if (reg_M.funct3 == 0x2) memwrite(res, int(val & 0xffffffff));
        else if (reg_M.funct3 == 0x3) memwrite(res, SREG(val));    
    }
    else {
        result.val = val;
        result.res = res;
    }
    return result;
}
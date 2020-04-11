#ifndef RISCV_CONFIG_HPP
#define RISCV_CONFIG_HPP

#include <string.h>

// Branch prediction
#define PRED_ALWAYS    0    // Always taken
#define PRED_NEVER     1    // Never taken
#define PRED_FTBNT     2    // Forward taken back not taken
#define PRED_RESERVED  3    

class Config {
public:
    bool set_entry_symbol, set_entry_addr;
    uint8_t branch_prediction;
    size_t entry_addr;   
    size_t max_memory_addr;
    size_t heap_base, heap_max;
    std::string entry_symbol;
};

#endif // RISCV_CONFIG_HPP
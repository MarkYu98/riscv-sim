#ifndef RISCV_CONFIG_HPP
#define RISCV_CONFIG_HPP

#include <string.h>
#include <fstream>
#include <riscv_proc.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>

extern std::string dec2hex(size_t i);
#define CEREAL_HEX_NVP(T)   ::cereal::make_nvp(#T, "0x" + dec2hex(T))
#define CEREAL_HEX_STR(T)   ::cereal::make_nvp(#T, T##_s)

class Config {
public:
    bool set_entry_symbol, set_entry_addr;      // Overwrite default entry point in ELF
    size_t entry_addr;                          // Not used when set_entry_addr is false
    size_t max_memory_addr;                     // Max reachable address for RV64 program
    size_t heap_base, heap_max;                 // Heap address and size (max address)
    std::string entry_symbol;                   // Not used when set_entry_symbol is false
#ifdef PIPE
    std::string branch_prediction;              // Options: always, never, btfnt, ftbnt
#endif
    struct {
        size_t mul, mulw, div, divw, ecall;
        size_t l1_bus, l1_hit, l2_bus, l2_hit, l3_bus, l3_hit, memory_bus, memory_hit;
        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                CEREAL_NVP(mul), CEREAL_NVP(mulw), 
                CEREAL_NVP(div), CEREAL_NVP(divw), CEREAL_NVP(ecall),
                CEREAL_NVP(l1_hit), CEREAL_NVP(l1_bus),
                CEREAL_NVP(l2_hit), CEREAL_NVP(l2_bus), 
                CEREAL_NVP(l3_hit), CEREAL_NVP(l3_bus), 
                CEREAL_NVP(memory_hit), CEREAL_NVP(memory_bus)
            ); 
        }
    } latency;

    struct {
        struct {
            size_t size, associativity, block_size;
            bool writeback;
            template<class Archive>
            void serialize(Archive & archive) {
                archive(
                    CEREAL_NVP(size), CEREAL_NVP(associativity), 
                    CEREAL_NVP(block_size), CEREAL_NVP(writeback)
                ); 
            }
        } l1, l2, l3;
        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                CEREAL_NVP(l1), CEREAL_NVP(l2), CEREAL_NVP(l3)
            ); 
        }
    } cache;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(
            CEREAL_NVP(set_entry_symbol), CEREAL_NVP(set_entry_addr), 
            CEREAL_HEX_NVP(entry_addr), CEREAL_HEX_NVP(max_memory_addr),
            CEREAL_HEX_NVP(heap_base), CEREAL_HEX_NVP(heap_max),
            CEREAL_NVP(entry_symbol), 
#ifdef PIPE
            CEREAL_NVP(branch_prediction),
#endif
            CEREAL_NVP(latency), CEREAL_NVP(cache)
        ); 
    }

    void save(const char *filename)
    {
        std::ofstream fout(filename);
        cereal::JSONOutputArchive ar(fout);
        ar(
            CEREAL_NVP(set_entry_symbol), CEREAL_NVP(set_entry_addr), 
            CEREAL_HEX_NVP(entry_addr), CEREAL_HEX_NVP(max_memory_addr),
            CEREAL_HEX_NVP(heap_base), CEREAL_HEX_NVP(heap_max),
            CEREAL_NVP(entry_symbol), 
#ifdef PIPE
            CEREAL_NVP(branch_prediction),
#endif
            CEREAL_NVP(latency), CEREAL_NVP(cache)
        ); 
    }

    void load(const char *filename)
    {
        std::ifstream fin(filename);
        cereal::JSONInputArchive ar(fin);
        std::string entry_addr_s, max_memory_addr_s, heap_base_s, heap_max_s;
        
        ar( 
            CEREAL_NVP(set_entry_symbol), CEREAL_NVP(set_entry_addr),
            CEREAL_HEX_STR(entry_addr), CEREAL_HEX_STR(max_memory_addr),
            CEREAL_HEX_STR(heap_base), CEREAL_HEX_STR(heap_max),
            CEREAL_NVP(entry_symbol), 
#ifdef PIPE
            CEREAL_NVP(branch_prediction),
#endif
            CEREAL_NVP(latency), CEREAL_NVP(cache)
        );
        entry_addr = std::stol(entry_addr_s, 0, 0);
        max_memory_addr = std::stol(max_memory_addr_s, 0, 0);
        heap_base = std::stol(heap_base_s, 0, 0);
        heap_max = std::stol(heap_max_s, 0, 0);
    }
};

#endif // RISCV_CONFIG_HPP
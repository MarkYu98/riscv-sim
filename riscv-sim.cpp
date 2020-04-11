#include <iostream>
#include <fstream>
#include <elfio/elfio.hpp>
#include <riscv_proc.hpp>
#include <riscv_config.hpp>
using namespace std;

void load_config(Config &config, const char *filename)
{
    ifstream fin(filename);
    cereal::JSONInputArchive iarchive(fin);
    iarchive(config);
}

int main(int argc, char** argv)
{
    ios::sync_with_stdio(false);
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <elf_file>" << endl;
        return -1;
    }

    // Create elfio reader
    ELFIO::elfio reader;
    Config config;
    config.load("config.json");

    // Load ELF data
    if (!reader.load(argv[1])) {
        cout << "Can't find or process ELF file " << argv[1] << endl;
        return -2;
    }

    // Check ELF file properties
    cout << "Checking file class : ";
    if ( reader.get_class() == ELFCLASS32 ) {
        cout << "ELF32" << endl;
        cout << "Not a 64-bit ELF format file, abort!" << endl;
        return -3;        
    }
    else
        cout << "ELF64, OK!" << endl;

    cout << "Checking ELF file encoding : ";
    if ( reader.get_encoding() == ELFDATA2LSB )
        cout << "Little endian, OK!" << endl;
    else {
        cout << "Big endian" << endl;
        cout << "ELF is not little endian encoded, abort!" << endl;
        return -4;
    }

    cout << "Checking ELF machine type : ";
    if ( reader.get_machine() == EM_RISCV )
        cout << "RISC-V, OK!" << endl;
    else {
        cout << "Not a RISC-V ELF file, abort!" << endl;
        return -5;
    }

    RISCV_proc simulator(reader, config);
    simulator.start();
    
    return 0;
}
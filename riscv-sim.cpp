#include <iostream>
#include <fstream>
#include <argparse.h>
#include <elfio/elfio.hpp>
#include <riscv_proc.hpp>
#include <riscv_config.hpp>
using namespace std;

int main(int argc, const char** argv)
{
    ios::sync_with_stdio(false);
#ifndef PIPE
    argparse::ArgumentParser program("RISCV Simulator (instruction)");
#else
    argparse::ArgumentParser program("RISCV Simulator (pipeline)");
#endif
    program.add_argument().names({"-p", "--program"}).description("The ELF program to run.").required(true);
    program.add_argument().names({"-c", "--config"}).description("Config file (JSON)").required(true);
    program.enable_help();

    auto err = program.parse(argc, argv);
    if (err) {
        cout << err.what() << endl;
        program.print_help();
        return -1;
    }

    if (program.exists("help")) {
        program.print_help();
        return 0;
    }

    // Create elfio reader
    ELFIO::elfio reader;
    Config config;
    config.load(program.get<string>("c").c_str());

    // Load ELF data
    if (!reader.load(program.get<string>("p").c_str())) {
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
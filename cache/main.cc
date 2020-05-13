#include "cache.h"
#include "memory.h"
#include <argparse.h>
#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

const double cpu_freq_GHz = 2.0;

int main(int argc, const char **argv) {
  argparse::ArgumentParser program("Cache Simulator");
  program.add_argument().names({"-f", "--file"}).description("The trace file").required(true);
  program.add_argument().names({"--size"}).description("Cache Size").required(true);
  program.add_argument().names({"-B", "--blocksize"}).description("Cache block size").required(true);
  program.add_argument().names({"-E", "--associativity"}).description("Cache associativity").required(true);
  program.add_argument().names({"--wb", "--writeback"}).description("Use Writeback/Writealloc policy");
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

  Memory m;
  Cache l1;
  l1.SetLower(&m);

  StorageStats s;
  s.access_time = 0;
  m.SetStats(s);
  l1.SetStats(s);

  StorageLatency ml;
  ml.bus_latency = 6;
  ml.hit_latency = 100;
  m.SetLatency(ml);

  StorageLatency ll;
  ll.bus_latency = 3;
  ll.hit_latency = 10;
  l1.SetLatency(ll);

  size_t block_size = program.get<size_t>("B");
  size_t associativity = program.get<size_t>("E");
  size_t cache_size = program.get<size_t>("size");
  bool writeback = program.exists("writeback");

  CacheConfig cc(cache_size, associativity, cache_size / (block_size * associativity), writeback);
  l1.SetConfig(cc);
  cout << "L1 Cache Config:\n"
          "  Cache Size: " << cc.size << "\n"
          "  Set number: " << cc.set_num << "\n"
          "  Associativity: " << cc.associativity << "\n"
          "  Write Policy: " << (writeback ? "Write Back/Write Alloc\n" : "Write Through / No Write Alloc\n") << endl;
  ifstream fin(program.get<string>("file").c_str());
  char ch; 
  string addr_s;
  size_t addr;
  char content[64];
  int hit, time, read;
  size_t total_time = 0;
  while (fin >> ch >> addr_s) {
    addr = stol(addr_s, 0, 0);
    read = (ch == 'r') ? 1 : 0;
    l1.HandleRequest(addr, 0, read, content, hit, time);
    total_time += time;
  }
  
  cout << "Total Access Time (CPU cycles): " << dec << total_time << endl;
  cout << "Total Access Time (Nanoseconds): " << fixed << setprecision(2)
       << (double)total_time / cpu_freq_GHz << endl;
  l1.GetStats(s);
  cout << "L1 Cache access time (CPU cycles): " << dec << s.access_time << endl;
  cout << "L1 Cache access time (Nanoseconds): " << fixed << setprecision(2) 
       << (double)s.access_time / cpu_freq_GHz << endl;
  cout << "L1 Cache access count: " << s.access_counter << endl;
  cout << "L1 Cache miss count: " << s.miss_num << endl;
  cout << "  Miss rate: " << fixed << setprecision(2) << 100 * (double)s.miss_num / s.access_counter << "%" << endl;
  cout << "L1 Cache replace count: " << s.replace_num << endl; 
  cout << "L1 Cache fetch count: " << s.fetch_num << endl;
  cout << endl;
  m.GetStats(s);
  cout << "Memory access time (CPU cycles): " << s.access_time << endl;
  cout << "Memory access time (Nanoseconds): " << fixed << setprecision(2) 
       << (double)s.access_time / cpu_freq_GHz << endl;
  cout << "Memory access count: " << s.access_counter << endl;
  return 0;
}

#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <string.h>
#include <map>
#include "storage.h"

typedef struct CacheConfig_ {
  int size;
  int associativity;
  int set_num; // Number of cache sets
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  CacheConfig_() {}
  CacheConfig_(int size, int associativity, int set_num, bool write_back) : 
      size(size), associativity(associativity), set_num(set_num) {
        write_through = !write_back;
        write_allocate = write_back;
    }
} CacheConfig;

class CacheLine {
public:
  CacheLine() {}
  void init(int block_size) { data = new char[block_size]; sz = block_size; dirty = false; valid = false; }
  ~CacheLine() { delete[] data; }
  bool isValid() const { return valid; }
  bool isDirty() const { return dirty; }
  size_t getTag() const { return tag; }
  char *getContent() const { return data; }
  void place(size_t tag, char *content) { this->tag = tag; memcpy(data, content, sz); valid = true; dirty = false; }
  void write(size_t offset, size_t bytes, char *content);
  void read(size_t offset, size_t bytes, char *content) { memcpy(content, data+offset, bytes); }
  CacheLine *prev = nullptr, *next = nullptr;
private:
  bool valid = false, dirty = false;
  size_t sz = 0, tag = 0;
  char *data;
};

class CacheSet;
class Cache: public Storage {
 public:
  Cache() {}
  ~Cache() {}

  // Sets & Gets
  void SetConfig(CacheConfig cc);
  void GetConfig(CacheConfig &cc);
  void SetLower(Storage *ll) { lower_ = ll; }
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);
  void flush();

 private:
  // Bypassing
  bool BypassDecision();
  // Prefetching
  bool PrefetchDecision();
  void PrefetchAlgorithm();

  int tbits, sbits, bbits;
  CacheConfig config_;
  CacheSet *cachesets = nullptr;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

class CacheSet {
public:
  CacheSet() {}
  void init(int associativity, int block_size);
  bool hit(size_t tag);
  bool full();
  void write(size_t tag, size_t offset, size_t bytes, char *content);
  void read(size_t tag, size_t offset, size_t bytes, char *content);
  void replace(CacheLine *victim, size_t tag, char *content);
  void add(size_t tag, char *content);
  CacheLine* getVictim() { return full() ? end : nullptr; }
  ~CacheSet() { delete[] lines; }
private:
  friend void Cache::flush();
  void moveToHead(CacheLine *line);
  std::map<size_t, int> tagmap;
  int e = 0;
  CacheLine *lines;
  CacheLine *head, *end;
};

#endif //CACHE_CACHE_H_ 

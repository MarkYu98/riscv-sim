#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include <map>
#include "storage.h"

class Memory: public Storage {
 public:
  Memory() : pgsize(0), nextpg(0) { pages.clear(); }
  ~Memory() {}
  void SetPGSize(size_t pgsize) { this->pgsize = pgsize; pages.clear(); }
  void free_page(size_t addr);
  void reset();
  size_t alloc_page();

  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

 private:
  // Memory implement
  size_t pgsize, nextpg;
  std::map<size_t, void *> pages;
  DISALLOW_COPY_AND_ASSIGN(Memory);
};

#endif //CACHE_MEMORY_H_ 

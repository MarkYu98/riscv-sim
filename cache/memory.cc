#include "memory.h"
#include <assert.h>
#include <string.h>

static size_t ROUND_DOWN(size_t bytes, size_t ALIGN)
{
  return ((bytes) & ~(ALIGN - 1));
}

void Memory::reset()
{
  for (auto &i : pages) 
    free(i.second);
  pages.clear();
  nextpg = 0;
}

void Memory::free_page(size_t addr) {
  assert(pages.find(addr) != pages.end());
  free(pages[addr]);
  pages.erase(addr);
}

size_t Memory::alloc_page() {
  void *newpage = malloc(this->pgsize);
  size_t ret = nextpg;
  pages[nextpg] = newpage;
  nextpg += this->pgsize;
  return ret;
}

void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  hit = 1;
  time = latency_.hit_latency + latency_.bus_latency;
  stats_.access_time += time;
  stats_.access_counter++;
  if (pgsize == 0) return;  // Simple simulation, no data

  size_t page_start = ROUND_DOWN(addr, pgsize);
  assert(pages.find(page_start) != pages.end());
  void *page = pages[page_start];
  if (read) memcpy(content, (char *)page + addr - page_start, bytes);
  else {
    if (content) memcpy((char *)page + addr - page_start, content, bytes);
    else memset((char *)page + addr - page_start, 0, bytes);
  }
}


#ifndef CACHE_STORAGE_H_
#define CACHE_STORAGE_H_

#include <stdint.h>
#include <stdio.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

// Storage access stats
typedef struct StorageStats_ {
  size_t access_counter;
  size_t miss_num;
  size_t access_time; // In CPU cycles
  size_t replace_num; // Evict old lines
  size_t fetch_num; // Fetch lower layer
  size_t prefetch_num; // Prefetch
} StorageStats;

// Storage basic config
typedef struct StorageLatency_ {
  int hit_latency; // In nanoseconds
  int bus_latency; // Added to each request
} StorageLatency;

class Storage {
 public:
  Storage() {}
  ~Storage() {}

  // Sets & Gets
  void SetStats(StorageStats ss) { stats_ = ss; }
  void GetStats(StorageStats &ss) { ss = stats_; }
  void SetLatency(StorageLatency sl) { latency_ = sl; }
  void GetLatency(StorageLatency &sl) { sl = latency_; }

  // Main access process
  // [in]  addr: access address
  // [in]  bytes: target number of bytes
  // [in]  read: 0|1 for write|read
  // [i|o] content: in|out data
  // [out] hit: 0|1 for miss|hit
  // [out] time: total access time
  virtual void HandleRequest(uint64_t addr, int bytes, int read,
                             char *content, int &hit, int &time) = 0;

 protected:
  StorageStats stats_;
  StorageLatency latency_;
};

#endif //CACHE_STORAGE_H_ 

#include "cache.h"
#include <math.h>
#include <assert.h>
#include <iostream>
using namespace std;

#define GETBITS(x, low, high) (x & ((~(1 << (low)) + 1) & ((1 << (high)) - 1)))

void CacheSet::moveToHead(CacheLine *line) {
  if (head == line) return;
  if (line->prev) line->prev->next = line->next;
  if (line->next) line->next->prev = line->prev;
  line->next = head; 
  if (head) head->prev = line; 
  head = line;
  if (end == line) end = line->prev;
  if (!end) end = line;
  line->prev = nullptr; 
}

bool CacheSet::hit(size_t tag) {
  return tagmap.find(tag) != tagmap.end();
}

bool CacheSet::full() {
  return tagmap.size() == e;
}

void CacheSet::read(size_t tag, size_t offset, size_t bytes, char *content) { 
  assert(tagmap.find(tag) != tagmap.end());
  int idx = tagmap[tag];
  return lines[idx].read(tag, bytes, content);
}

void CacheSet::write(size_t tag, size_t offset, size_t bytes, char *content) {
  assert(tagmap.find(tag) != tagmap.end());
  int idx = tagmap[tag];
  lines[idx].write(offset, bytes, content);
  moveToHead(&lines[idx]);
}

void CacheSet::add(size_t tag, char *content) {
  int idx = tagmap[tag] = tagmap.size();
  lines[idx].place(tag, content);
  moveToHead(&lines[idx]);
}

void CacheSet::replace(CacheLine *victim, size_t tag, char *content) {
  assert(tagmap.find(victim->getTag()) != tagmap.end());
  tagmap[tag] = tagmap[victim->getTag()];
  tagmap.erase(victim->getTag());
  victim->place(tag, content);
  moveToHead(victim);
}

void Cache::SetConfig(CacheConfig cc) {
  config_ = cc;
  if (cachesets) delete [] cachesets;
  int size = config_.size;
  int associativity = config_.associativity;
  int set_num = config_.set_num;
  int block_size = size / (associativity * set_num);
  sbits = (int)(log(set_num)/log(2));
  bbits = (int)(log(block_size)/log(2));
  tbits = sizeof(size_t) * 8 - sbits - bbits;
  cachesets = new CacheSet[set_num];
  for (int i = 0; i < set_num; i++) {
    cachesets[i].init(associativity);
  }
}

void Cache::GetConfig(CacheConfig &cc) {
  cc = config_;
}

void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  hit = 0;
  time = 0;
  // Bypass?
  if (!BypassDecision()) {
    stats_.access_counter++;
    int lower_hit, lower_time;
    size_t tag = GETBITS(addr, sbits + bbits, sizeof(size_t) * 8) >> (sbits + bbits);
    size_t s = GETBITS(addr, bbits, bbits + sbits) >> bbits;
    size_t offset = GETBITS(addr, 0, bbits);
    time += latency_.bus_latency + latency_.hit_latency;
    stats_.access_time += time;
    if (cachesets[s].hit(tag)) {  // Hit
      hit = 1;
      if (read) {   // Read Hit 
        cachesets[s].read(tag, offset, bytes, content);
      } else {  // Write Hit
        if (this->config_.write_through) {  // Write through
          cachesets[s].write(tag, offset, bytes, content);
          lower_->HandleRequest(addr, bytes, 1, content, lower_hit, lower_time);
          time += lower_time;
        } else {  // Write back
          cachesets[s].write(tag, offset, bytes, content);
        }
      }
      return;
    }
    else { // Miss
      hit = 0;
      stats_.miss_num++;
      if (read) {   // Read Miss
        CacheLine *victim = cachesets[s].getVictim();
        stats_.replace_num++;
        if (!this->config_.write_through && victim && victim->isDirty()) {  // Write back
          size_t wb_addr = ((victim->getTag() << (sbits + bbits)) | (s << sbits));
          lower_->HandleRequest(wb_addr, 1 << bbits, 0, victim->getContent(), lower_hit, lower_time);
          time += lower_time;
        }
        char buf[1 << bbits];
        lower_->HandleRequest(addr, 1 << bbits, 1, buf, lower_hit, lower_time);
        time += lower_time;
        stats_.fetch_num++;
        if (victim == nullptr) cachesets[s].add(tag, buf);
        else cachesets[s].replace(victim, tag, buf);
        cachesets[s].read(tag, offset, bytes, content);
      } else {    // Write Miss
        if (this->config_.write_allocate) { // Write alloc
          CacheLine *victim = cachesets[s].getVictim();
          stats_.replace_num++;
          if (!this->config_.write_through && victim && victim->isDirty()) {  // Write back
            size_t wb_addr = ((victim->getTag() << (sbits + bbits)) | (s << sbits));
            lower_->HandleRequest(wb_addr, 1 << bbits, 0, victim->getContent(), lower_hit, lower_time);
            time += lower_time;
          }
          char buf[1 << bbits];
          lower_->HandleRequest(addr, 1 << bbits, 1, buf, lower_hit, lower_time);
          time += lower_time;
          stats_.fetch_num++;
          if (victim == nullptr) cachesets[s].add(tag, buf);
          else cachesets[s].replace(victim, tag, buf);
          if (this->config_.write_through) {  // Write through
            cachesets[s].write(tag, offset, bytes, content);
            lower_->HandleRequest(addr, bytes, 1, content, lower_hit, lower_time);
            time += lower_time;
          } else {  // Write back
            cachesets[s].write(tag, offset, bytes, content);
          }
        } else { // No write alloc
          lower_->HandleRequest(addr, bytes, 1, content, lower_hit, lower_time);
          time += lower_time;
        }
      }
    }
  } else {
    // Fetch from lower layer
    int lower_hit, lower_time;
    lower_->HandleRequest(addr, bytes, read, content, lower_hit, lower_time);
    hit = 0;
    time += latency_.bus_latency + lower_time;
    stats_.fetch_num++;
    stats_.access_time += latency_.bus_latency;
  }

  // Prefetch?
  if (PrefetchDecision()) {
    PrefetchAlgorithm();
    stats_.prefetch_num++;
  } 
}

bool Cache::BypassDecision() {
  return false;
}

bool Cache::PrefetchDecision() {
  return false;
}

void Cache::PrefetchAlgorithm() {
}


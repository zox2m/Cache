#pragma once
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "cache.h"
// Second Chance Algorithm
//  4kB / Cache Line size 64bytes

struct line_ {
  int tag;
  bool valid;
  bool sca;
  bool dirty;
  int* memory; // ??4bytes 16
};

class Cache_sca : public Cache {
private:
  int oldset;
  int entries;
  struct line_* cache;
  int find_tag(int tag);
  void write_mem(int index);
  void load_mem(int index, int tag);
  void print_all();

public:
  virtual void setting(int* memory) override;
  virtual int load(int i) override;
  virtual void write(int i, int data) override;
};
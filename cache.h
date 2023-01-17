#pragma once
#include <cstdio>
#include <cstdlib>
// basic memory access algorithm

// 사용 정보 저장용 구조체
struct info_ {
  int hit_time;
  int miss_time;
  int load_cache;
  int load_mem;
  int write_cache;
  int write_mem;
};

// 캐시라는 이름의 메모리만 사용
class Cache {
protected:
  struct info_ info; // 저장용
  int* memory; // 램부분

public:
  virtual void setting(int* memory);
  virtual int load(int i);
  virtual void write(int i, int data);
  struct info_ get_info();
};

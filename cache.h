#pragma once
#include <cstdio>
#include <cstdlib>
// basic memory access algorithm

// ��� ���� ����� ����ü
struct info_ {
  int hit_time;
  int miss_time;
  int load_cache;
  int load_mem;
  int write_cache;
  int write_mem;
};

// ĳ�ö�� �̸��� �޸𸮸� ���
class Cache {
protected:
  struct info_ info; // �����
  int* memory; // ���κ�

public:
  virtual void setting(int* memory);
  virtual int load(int i);
  virtual void write(int i, int data);
  struct info_ get_info();
};

#include "cache.h"

void Cache::setting(int* memory) {
  this->memory = memory;
}

int Cache::load(int i) {
  info.load_mem += 1; // ������ ����
  return memory[i / 4]; // �޸� ����
}

void Cache::write(int i, int d) {
  info.write_mem += 1; // ������ ����
  memory[i / 4] = d; // �޸𸮿� ������ �Է�
}

struct info_ Cache::get_info() {
  return info;
}
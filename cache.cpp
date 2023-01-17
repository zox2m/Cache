#include "cache.h"

void Cache::setting(int* memory) {
  this->memory = memory;
}

int Cache::load(int i) {
  info.load_mem += 1; // 데이터 저장
  return memory[i / 4]; // 메모리 참조
}

void Cache::write(int i, int d) {
  info.write_mem += 1; // 데이터 저장
  memory[i / 4] = d; // 메모리에 데이터 입력
}

struct info_ Cache::get_info() {
  return info;
}
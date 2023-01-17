#include "cache_sca.h"

// 디버깅용 (필없음)
void Cache_sca::print_all() {
  for (int i = 0; i < 100; i++) {
    printf("tag: %x ", cache[i].tag);
    for (int j = 0; j < 16; j++) {
      printf("%X, ", cache[i].memory[j]);
    }
    printf("\n");
  }
}

// tag를 받아서 맞는 tag를 찾음 없으면 -1
int Cache_sca::find_tag(int tag) {
  for (int i = 0; i < entries; i++) {
    if (cache[i].tag == tag && cache[i].valid) {
      return i;
    }
  }
  return -1;
}

// 메모리에 데이터를 입력
void Cache_sca::write_mem(int index) {
  int tag = cache[index].tag;
  info.write_mem += 1; // 메모리 접근 사이클
  for (int i = 0; i < 16; i++) {
    int mem = (tag << 4) | i;
    memory[mem] = cache[index].memory[i];
  }
}

// 메모리에서 데이터를 가져옴
void Cache_sca::load_mem(int index, int tag) {
  info.load_mem += 1; // 메모리 접근 사이클
  // line초기화
  cache[index].tag = tag;
  cache[index].valid = 1;
  cache[index].sca = 0;
  cache[index].dirty = 0;
  for (int i = 0; i < 16; i++) {
    int mem = (tag << 4) | i;
    cache[index].memory[i] = memory[mem];
  }
}

void Cache_sca::setting(int* memory) {
  Cache::setting(memory);
  int cache_size = 0b1000000000000;
  int line_size = 0b1000000;
  entries = cache_size/line_size; // 4kB
  cache = new struct line_[entries];
  oldset = 0;

  // cache 초기화
  for (int i = 0; i < entries; i++) {
    cache[i].tag = 0;
    cache[i].valid = 0;
    cache[i].sca = 0;
    cache[i].dirty = 0;
    cache[i].memory = new int[16];
    memset(cache[i].memory, 0, 16 * sizeof(int));
  }
}

// 캐시에서 로드
int Cache_sca::load(int i) {
  int tag = i >> 6;
  int data = (i >> 2) & 0b1111;
  int index;
  info.load_cache += 1; // 캐시 접근 +1
  if ((index = find_tag(tag)) == -1) {
    // Miss
    info.miss_time += 1;
    while (cache[oldset].sca) {
      cache[oldset].sca = 0;
      oldset = (oldset + 1) % entries;
    }
    index = oldset;
    oldset = (oldset + 1) % entries;
    if (cache[index].dirty) {
      write_mem(index);
    }
    load_mem(index, tag);
  }
  else {
    // Hit
    info.hit_time += 1;
    cache[index].sca = 1;
  }
  //print_all();
  //printf("%x\n",cache[index].memory[data]);
  return cache[index].memory[data];
}

// 캐시에 쓰기
void Cache_sca::write(int i, int dat) {
  int tag = i >> 6;
  int data = (i >> 2) & 0b1111;
  int index;
  info.write_cache += 1; // 캐시 접근 +1
  if ((index = find_tag(tag)) == -1) {
    // Miss
    info.miss_time += 1;
    while (cache[oldset].sca) {
      cache[oldset].sca = 0;
      oldset = (oldset + 1) % entries;
    }
    index = oldset;
    oldset = (oldset + 1) % entries;
    if (cache[index].dirty) {
      write_mem(index);
    }
    load_mem(index, tag);
  }
  else {
    // Hit
    info.hit_time += 1;
  }
  cache[index].sca = 1;
  cache[index].dirty = 1;
  cache[index].memory[data] = dat;
}

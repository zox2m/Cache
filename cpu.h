#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include "const_list.h"
#include "cache.h"
#include "cache_sca.h"


using namespace std;

struct instruction_ // 명령어 값 임시 저장소
{
  int instruction; // ???
  int opcode;      // 명령
  int rs;
  int rd;
  int rt;
  int shamt;  // 쉬프트 연산용
  int funct;  // R-format 명령어
  int simm;   // 상수 저장소
  int target; // 다음 pc 주소
};

class Process {
private:
  Cache* cache;
  int instruction; // 4바이트 즉 32비트짜리 명령어를 읽어야 한다!
  int* R;            // 레지스터!! 32개임.
  int* Memory;      // 메모리
  int pc;               // 프로그램 카운터...
  int cycle;            //몇 사이클 째 인지 세는 용
  FILE* output;       // 디버깅용
  struct instruction_ in;         // 인스트럭션 한 줄 읽어서 저장할 인스트럭션 구조체 in
  void log(const char* str); // 로그 기록
  void print_result(); // 마지막 출력

public:
  Process(const char* f_name, Cache* cache);
  int get_sign(int add); // 뒤쪽 16비트를
  void fetch(); // pc 위치에서 명령어 읽어오기
  struct instruction_ decode(int i); // i 는 32비트 명령어!!
  void write_mem();
  void load_mem();
  void update_pc();
  void execute(); // 명령 별 계산
  void run();
  ~Process();
};
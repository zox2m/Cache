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

struct instruction_ // ��ɾ� �� �ӽ� �����
{
  int instruction; // ???
  int opcode;      // ���
  int rs;
  int rd;
  int rt;
  int shamt;  // ����Ʈ �����
  int funct;  // R-format ��ɾ�
  int simm;   // ��� �����
  int target; // ���� pc �ּ�
};

class Process {
private:
  Cache* cache;
  int instruction; // 4����Ʈ �� 32��Ʈ¥�� ��ɾ �о�� �Ѵ�!
  int* R;            // ��������!! 32����.
  int* Memory;      // �޸�
  int pc;               // ���α׷� ī����...
  int cycle;            //�� ����Ŭ ° ���� ���� ��
  FILE* output;       // ������
  struct instruction_ in;         // �ν�Ʈ���� �� �� �о ������ �ν�Ʈ���� ����ü in
  void log(const char* str); // �α� ���
  void print_result(); // ������ ���

public:
  Process(const char* f_name, Cache* cache);
  int get_sign(int add); // ���� 16��Ʈ��
  void fetch(); // pc ��ġ���� ��ɾ� �о����
  struct instruction_ decode(int i); // i �� 32��Ʈ ��ɾ�!!
  void write_mem();
  void load_mem();
  void update_pc();
  void execute(); // ��� �� ���
  void run();
  ~Process();
};
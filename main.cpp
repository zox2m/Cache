#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include "cpu.h"
#include "cache.h"
#include "cache_sca.h"
#include "cache_DM.h"

int main(void)
{
  Process* cpu;
  printf("just memory\n");
  const char* file_name = "fib.bin";
  cpu = new Process(file_name, new Cache());
  cpu->run();
  delete cpu;

  printf("\n");
  printf("second chance algorithm\n");
  cpu = new Process(file_name, new Cache_sca());
  cpu->run();
  delete cpu;
  return 0;
}
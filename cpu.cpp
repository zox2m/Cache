#include "cpu.h"

int to_be(int _target) // 리틀 엔디안 -> 빅 엔디안
{
  unsigned char bytes[4];
  int ret;
  bytes[0] = (unsigned char)((_target >> 0) & 0xff);
  bytes[1] = (unsigned char)((_target >> 8) & 0xff);
  bytes[2] = (unsigned char)((_target >> 16) & 0xff);
  bytes[3] = (unsigned char)((_target >> 24) & 0xff);
  ret = ((int)bytes[0] << 24) | ((int)bytes[1] << 16) | ((int)bytes[2] << 8) | ((int)bytes[3] << 0);
  return ret;
}

void get_mem(const char* f_name, int* Memory) {
  int inst_ = 0;
  int value = 0;
  size_t ret = 0;
  int i = 0;
  FILE* fp = fopen(f_name, "rb");
  if (fp == NULL)
  {
    printf("File Open Error! \n"); // fopen이 제대로 되었다면 &fp값은 NULL이 아닐것이다
    exit(1);
  }

  //메모리에 inst 다 저장해놓는 과정!! stored 니까!!
  while (1)
  {
    ret = fread(&value, sizeof(value), 1, fp); // 버퍼, 읽기 크기 4바이트, 읽기 횟수,파일 포인터
    if (ret != 1)
      break; // fread 함수는 성공한 읽기 횟수를 반환한다. 1번 시켰으니 1이 나와야 정상
    // printf("ret: %d, 0x%08X\n", ret, value); // value엔 파일의 4바이트가 읽혔을 것이다

    // inst_ = 10;
    inst_ = to_be(value); // ntohl(value) 로 엔디안 어쩌고 읽으려 했는데 이거 헤더 추가 실패해서! 그냥 ㄱㄱ
    //printf("Mem[%d]:0x%08X\n", i, inst_);
    Memory[i] = inst_;
    i++;
  }
  fclose(fp); // 파일 닫기
}

void Process::log(const char* str) {
  //printf("%s\n", str);
  //char tmp[100];
  //strcpy(tmp, str);
  //strcat(tmp, "\n");
  //fwrite(tmp, sizeof(char), strlen(tmp), output);
}

void Process::print_result() {
  struct info_ info = cache->get_info();
  printf("total number of cycles: %d\n", cycle);
  printf("memory> load: %d, write: %d\n", info.load_mem, info.write_mem);
  printf("cache> load: %d, write: %d\n", info.load_cache, info.write_cache);
  printf("hit:%d, miss: %d\n", info.hit_time, info.miss_time);
  printf("final return: %d\n", R[2]);
}

Process::Process(const char* f_name, Cache* cache) {
  int size = 0x1000000;
  Memory = new int[size];
  this->cache = cache;
  this->cache->setting(Memory);
  R = new int[32];
  memset(Memory, 0, size * sizeof(int));
  memset(R, 0, 32 * sizeof(int));
  get_mem(f_name, Memory);
  pc = 0;
  cycle = 0;
  R[31] = -1;       // RA . 얘는 -1 로 고정임
  R[29] = 0x100000; // SP . 얘도 고정
  instruction = NULL;

  output = fopen("output.txt", "w");
  if (output == NULL)
  {
    printf("File Open Error! \n"); // fopen이 제대로 되었다면 &fp값은 NULL이 아닐것이다
    exit(1);
  }
}

Process::~Process() {
  delete[] Memory;
  delete[] R;
  delete cache;
  fclose(output);
}

int Process::get_sign(int add) // 뒤쪽 16비트를
{
  return (add << 16) >> 16;
}

void Process::fetch() // pc 위치에서 명령어 읽어오기
{
  // pc 주소값은 4바이트씩 올라감. 근데 메모리는 1바이트(8비트 )씩 쓸거니깐 나누기 4. (4바이트 = 32비트)
  instruction = cache->load(pc);
}

struct instruction_ Process::decode(int i) // i 는 32비트 명령어!!
{
  struct instruction_ ret; // 새로운 구조체
  // memset
  ret.instruction = 0;
  ret.opcode = 0;
  ret.rs = 0;
  ret.rd = 0;
  ret.rt = 0;
  ret.shamt = 0;
  ret.funct = 0;
  ret.simm = 0;
  ret.target = 0;

  // get opcode
  ret.opcode = (i >> 26) & 0x3F; // op 6자리만 남기고 오른쪽으로 민다! 거기서 6비트 읽음. &는 그 비트 1인애만 읽기때문
  if (ret.opcode == 0)
  {
    log("R");
    ret.rs = (i >> 21) & 0x1F;   // rs 11자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.rt = (i >> 16) & 0x1F;   // rt 16자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.rd = (i >> 11) & 0x1F;   // rd 11자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.shamt = (i >> 6) & 0x1F; // shamt 6자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.funct = i & 0x3F;        // funct 오른쪽 끝 6비트 읽는다
  }
  else if (ret.opcode == 2 || ret.opcode == 3)
  {
    log("J");
    ret.target = (i & 0x3FFFFFF) << 2;
  }
  else
  {
    log("I");
    ret.rs = (i >> 21) & 0x1F; // rs 11자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.rt = (i >> 16) & 0x1F; // rt 16자리만 남기고 오른쪽으로 민다! 오른쪽부터 5비트 읽는다
    ret.simm = i & 0xFFFF;     // 오른쪽 끝 16비트 읽기
  }
  return ret;
}

void Process::write_mem()
{
  cache->write(in.target, R[in.rt]);
  //Memory[in.target / 4] = R[in.rt];
}

void Process::load_mem()
{
  //printf("1: %x\n", cache->load(in.target));
  //printf("2: %x\n", Memory[in.target / 4]);
  R[in.rt] = cache->load(in.target);
  //R[in.rt] = Memory[in.target / 4];
}

void Process::update_pc()
{
  pc = pc + 4;
}

void Process::execute() // 명령 별 계산
{
  if (instruction == 0)
    return;
  if (in.opcode == 0) // R-format
  {
    switch (in.funct)
    {
    case ADD:
      if ((unsigned)R[in.rs] > (unsigned)0xffffffff - (unsigned)R[in.rt])
      {
        log("Integer overflow error");
        exit(1);
      }
    case ADDU:
      log("ADDU");
      R[in.rd] = R[in.rs] + R[in.rt];
      break;
    case AND:
      log("AND");
      R[in.rd] = R[in.rs] & R[in.rt];
      break;
    case JR:
      log("JR");
      pc = R[in.rs] - 4;
      break;
    case NOR:
      log("NOR");
      R[in.rd] = ~(R[in.rs] | R[in.rt]);
      break;
    case OR:
      log("OR");
      R[in.rd] = R[in.rs] | R[in.rt];
      break;
    case SLT:
      log("SLT");
      R[in.rd] = R[in.rs] < R[in.rt] ? 1 : 0;
      break;
    case SLTU:
      log("SLTU");
      R[in.rd] = (unsigned int)R[in.rs] < (unsigned int)R[in.rt] ? 1 : 0;
      break;
    case SLL:
      log("SLL");
      R[in.rd] = R[in.rt] << in.shamt;
      break;
    case SRL:
      log("SRL");
      R[in.rd] = R[in.rt] >> in.shamt;
      break;
    case SUB:
      if (R[in.rs] < R[in.rt]) {
        log("Integer underflow error");
        exit(1);
      }
    case SUBU:
      log("SUBU");
      R[in.rd] = R[in.rs] - R[in.rt];
      break;
    default:
      log("ERROR");
      exit(1);
    }
  }
  else // I-format
  {
    switch (in.opcode)
    {
    case ADDI:
    case ADDIU:
      log("ADD");
      R[in.rt] = R[in.rs] + get_sign(in.simm);
      break;
    case ANDI:
      log("ANDI");
      R[in.rt] = R[in.rs] & in.simm;
    case BEQ:
      log("BEQ");
      if (R[in.rs] == R[in.rt])
        pc = pc + (get_sign(in.simm) << 2);
      break;
    case BNE:
      log("BNE");
      if (R[in.rs] != R[in.rt])
        pc = pc + (get_sign(in.simm) << 2);
      break;
    case LUI:
      log("LUI");
      R[in.rt] = (in.simm << 16);
      break;
    case LW:
      log("LW");
      in.target = R[in.rs] + get_sign(in.simm);
      load_mem();
      break;
    case ORI:
      log("ORI");
      R[in.rt] = R[in.rs] | in.simm;
      break;
    case SLTI:
      log("SLTI");
      R[in.rt] = get_sign(R[in.rs]) < in.simm ? 1 : 0;
      break;
    case SLTIU:
      log("SLTI");
      R[in.rt] = R[in.rs] < in.simm ? 1 : 0;
      break;
    case SW:
      log("SW");
      in.target = R[in.rs] + get_sign(in.simm);
      write_mem();
      break;

      // J-format
    case J:
      log("J");
      pc = in.target - 4;
      break;
    case JAL:
      log("JAL");
      R[31] = pc + 8;
      pc = in.target - 4;
      break;
    default:
      log("ERROR");
      exit(1);
    }
  }
}

void Process::run() {
  while (1)
  {
    log(to_string(pc).c_str());
    if (pc == -1)
      break; // pc가 -1이면 종료!
    //인스트럭션 가져오기
    fetch();
    in = decode(instruction);
    execute();
    update_pc();

    //변한 스탯을 출력한다
    // printf("pc : %X\n", pc); //변환 스택 출력 하는디 예시로 pc 적어둠. 레지스터 적어야하나?

    cycle++; //여까지 하면 한사이클~
  }
  print_result();
}
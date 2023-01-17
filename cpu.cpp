#include "cpu.h"

int to_be(int _target) // ��Ʋ ����� -> �� �����
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
    printf("File Open Error! \n"); // fopen�� ����� �Ǿ��ٸ� &fp���� NULL�� �ƴҰ��̴�
    exit(1);
  }

  //�޸𸮿� inst �� �����س��� ����!! stored �ϱ�!!
  while (1)
  {
    ret = fread(&value, sizeof(value), 1, fp); // ����, �б� ũ�� 4����Ʈ, �б� Ƚ��,���� ������
    if (ret != 1)
      break; // fread �Լ��� ������ �б� Ƚ���� ��ȯ�Ѵ�. 1�� �������� 1�� ���;� ����
    // printf("ret: %d, 0x%08X\n", ret, value); // value�� ������ 4����Ʈ�� ������ ���̴�

    // inst_ = 10;
    inst_ = to_be(value); // ntohl(value) �� ����� ��¼�� ������ �ߴµ� �̰� ��� �߰� �����ؼ�! �׳� ����
    //printf("Mem[%d]:0x%08X\n", i, inst_);
    Memory[i] = inst_;
    i++;
  }
  fclose(fp); // ���� �ݱ�
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
  R[31] = -1;       // RA . ��� -1 �� ������
  R[29] = 0x100000; // SP . �굵 ����
  instruction = NULL;

  output = fopen("output.txt", "w");
  if (output == NULL)
  {
    printf("File Open Error! \n"); // fopen�� ����� �Ǿ��ٸ� &fp���� NULL�� �ƴҰ��̴�
    exit(1);
  }
}

Process::~Process() {
  delete[] Memory;
  delete[] R;
  delete cache;
  fclose(output);
}

int Process::get_sign(int add) // ���� 16��Ʈ��
{
  return (add << 16) >> 16;
}

void Process::fetch() // pc ��ġ���� ��ɾ� �о����
{
  // pc �ּҰ��� 4����Ʈ�� �ö�. �ٵ� �޸𸮴� 1����Ʈ(8��Ʈ )�� ���Ŵϱ� ������ 4. (4����Ʈ = 32��Ʈ)
  instruction = cache->load(pc);
}

struct instruction_ Process::decode(int i) // i �� 32��Ʈ ��ɾ�!!
{
  struct instruction_ ret; // ���ο� ����ü
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
  ret.opcode = (i >> 26) & 0x3F; // op 6�ڸ��� ����� ���������� �δ�! �ű⼭ 6��Ʈ ����. &�� �� ��Ʈ 1�ξָ� �б⶧��
  if (ret.opcode == 0)
  {
    log("R");
    ret.rs = (i >> 21) & 0x1F;   // rs 11�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.rt = (i >> 16) & 0x1F;   // rt 16�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.rd = (i >> 11) & 0x1F;   // rd 11�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.shamt = (i >> 6) & 0x1F; // shamt 6�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.funct = i & 0x3F;        // funct ������ �� 6��Ʈ �д´�
  }
  else if (ret.opcode == 2 || ret.opcode == 3)
  {
    log("J");
    ret.target = (i & 0x3FFFFFF) << 2;
  }
  else
  {
    log("I");
    ret.rs = (i >> 21) & 0x1F; // rs 11�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.rt = (i >> 16) & 0x1F; // rt 16�ڸ��� ����� ���������� �δ�! �����ʺ��� 5��Ʈ �д´�
    ret.simm = i & 0xFFFF;     // ������ �� 16��Ʈ �б�
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

void Process::execute() // ��� �� ���
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
      break; // pc�� -1�̸� ����!
    //�ν�Ʈ���� ��������
    fetch();
    in = decode(instruction);
    execute();
    update_pc();

    //���� ������ ����Ѵ�
    // printf("pc : %X\n", pc); //��ȯ ���� ��� �ϴµ� ���÷� pc �����. �������� ������ϳ�?

    cycle++; //������ �ϸ� �ѻ���Ŭ~
  }
  print_result();
}
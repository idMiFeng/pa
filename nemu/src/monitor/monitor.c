/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome()
{
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
                          "to record the trace. This may lead to a large log file. "
                          "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static long load_img()
{
  if (img_file == NULL)
  {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static char *elf_file=NULL;
static int parse_args(int argc, char *argv[])
{
  const struct option table[] = {
      //传入elf文件
      {"elf",required_argument,NULL,'e'},
      {"batch", no_argument, NULL, 'b'},//如果解析到 -b 或 --batch 选项，会调用 sdb_set_batch_mode() 函数来设置批处理模式。
      {"log", required_argument, NULL, 'l'},
      {"diff", required_argument, NULL, 'd'},
      {"port", required_argument, NULL, 'p'},
      {"help", no_argument, NULL, 'h'},
      {0, 0, NULL, 0},
  };
  /*name：表示长选项的名称，例如，--help。
has_arg：表示选项是否需要参数，可以是no_argument（无参数）、required_argument（必须有参数）或optional_argument（可选参数）。
flag：如果不为空，指向一个整数，用于存储选项的返回值。如果为NULL，getopt_long将返回选项的返回值。
val：选项的返回值。通常是一个字符或整数。*/
  int o;
  //在getopt_long的使用中，程序会循环调用该函数来处理每个命令行参数，根据解析的选项和参数进行相应的操作
  /*int getopt_long(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex);
参数说明：

argc：命令行参数的数量，通常是 main 函数的参数 argc。
argv：命令行参数的数组，通常是 main 函数的参数 argv。
optstring：一个包含短选项字符的字符串，用于指示哪些选项需要参数，哪些选项不需要参数。类似于传统的 getopt 函数的用法。
longopts：一个指向 struct option 结构数组的指针，定义了长选项的信息，包括名称、是否需要参数、短选项等。
longindex：一个指向整数的指针，用于存储当前解析的长选项在 longopts 数组中的索引。通常可以为 NULL，如果不需要获得索引信息
-bhl:d:p:e是 optstring 参数
-b：不需要参数。这是一个标志选项，用于表示批处理模式。
-h：不需要参数。这通常表示帮助选项，用于显示帮助信息。
-l：需要参数。接下来的参数应该是一个日志文件的路径。
-d：需要参数。接下来的参数应该是差分测试的引用共享库文件的路径。
-p：需要参数。接下来的参数应该是差分测试的端口号。
-e：需要参数。接下来的参数应该是一个 ELF 文件的路径。*/
  while ((o = getopt_long(argc, argv, "-bhl:d:p:e", table, NULL)) != -1)
  {
    switch (o)
    {
    case 'e':
    elf_file=optarg;
    break;
    case 'b':
      sdb_set_batch_mode();
      break;
    case 'p':
      sscanf(optarg, "%d", &difftest_port);
      break;
    case 'l':
      log_file = optarg;
      break;
    case 'd':
      diff_so_file = optarg;
      break;
    case 1:
      img_file = optarg;
      return 0;
    default:
      printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
      printf("\t-e,--elf=FILE           elf file to be parsed\n");
      printf("\t-b,--batch              run with batch mode\n");
      printf("\t-l,--log=FILE           output log to FILE\n");
      printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
      printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
      printf("\n");
      exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[])
{
  /* Perform some global initialization. */

  /* Parse arguments. *///这个函数用于解析命令行参数，argc是参数计数，argv是参数数组。它会解析并记录命令行参数，以便在后续的初始化和运行过程中使用。
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. *///初始化内存，为程序运行提供了内存空间
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
                           MUXDEF(CONFIG_ISA_x86, "i686",
                                  MUXDEF(CONFIG_ISA_mips32, "mipsel",
                                         MUXDEF(CONFIG_ISA_riscv32, "riscv32",
                                                MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"));
#endif

  /* Display welcome message. */
   welcome();
}
#else // CONFIG_TARGET_AM
static long load_img()
{
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor()
{
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif

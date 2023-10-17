# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)
#这个 Makefile 片段定义了一个名为 "html" 的规则，用于生成 HTML 版本的当前 Makefile
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html



#Makefile 中的变量可以从多个地方获取值：\
Makefile 内部设置： 可能在 Makefile 的其他地方设置了 $(AM_HOME) 变量，但你在当前查看的部分没有找到。\
外部设置： $(AM_HOME) 可能是在调用 Make 命令时通过命令行参数设置的，例如：\
javascript\
Copy code\
make AM_HOME=/path/to/abstractmachine\
这样会将 $(AM_HOME) 变量设置为 /path/to/abstractmachine。\
Shell 环境变量： Make 可以访问系统环境变量。如果 $(AM_HOME) 没有在 Makefile 中显式设置，它可能会从操作系统的环境变量中获取值。\
要确定 $(AM_HOME) 变量的来源，你可以搜索整个项目，包括 Makefile 文件以及调用 Make 命令的方式。通常，这个变量会在项目的某个地方设置，以指示 Make 如何定位 AbstractMachine 代码库。


## 1. Basic Setup and Checks

### Default to create a bare-metal kernel image 默认情况下，如果没有指定构建目标，将创建一个裸机内核映像
#这是一个条件语句，用于检查是否有指定的构建目标。$(MAKECMDGOALS) 是一个特殊的变量，它包含了用户在命令行中指定的构建目标。
ifeq ($(MAKECMDGOALS),)
#如果上述条件为真，即没有指定构建目标，那么这一行会将变量 MAKECMDGOALS 设置为 "image"。
#一般来说，MAKECMDGOALS 用于获取用户在命令行上指定的目标，而 .DEFAULT_GOAL 用于设置默认的构建目标。
#设置它们都是为了确保在没有明确指定目标时，有一个默认的目标可以被构建。
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### Override checks when `make clean/clean-all/html`
#假设用户在命令行中运行以下命令之一：
#make clean: 清理项目，删除临时文件等。
#make clean-all: 清理项目，删除所有生成的文件。
#make html: 生成 HTML 版本的 Makefile。
#那么这段代码将会执行相应的操作。
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message
#$(info ...)：$(info) 是一个 Makefile 内置函数，用于输出信息消息到标准输出（通常是终端）。
# Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)]：这是要输出的信息消息的文本内容。它包含了以下变量：
#$(NAME)：这是一个在 Makefile 中定义的变量，通常用于表示构建的项目名称。
#$(MAKECMDGOALS)：这是一个包含用户在命令行上指定的构建目标的变量。
#$(ARCH)：这是另一个在 Makefile 中定义的变量，通常表示目标体系结构或架构。
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane
#$(wildcard ...) 是一个 Makefile 函数，用于查找文件系统中的文件。在这里，它用于检查 $(AM_HOME)/am/include/am.h 文件是否存在。
#如果 am.h 文件不存在，条件为真，执行下面的操作。
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### Check if there is something to build
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets

### Create the destination directory (`build/$ARCH`)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (a binary image or archive)
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))

## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

### Compilation flags
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INCFLAGS += $(addprefix -I, $(INC_PATH))

CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"arch/$(ARCH).h\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS)
LDFLAGS  += -z noexecstack

## 4. Arch-Specific Configurations

### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`)
-include $(AM_HOME)/scripts/$(ARCH).mk

### Fall back to native gcc/binutils if there is no cross compiler
ifeq ($(wildcard $(shell which $(CC))),)
  $(info #  $(CC) not found; fall back to default gcc and binutils)
  CROSS_COMPILE :=
endif

## 5. Compilation Rules

### Rule (compile): a single `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

### Rule (recursive make): build a dependent library (am, klib, ...)
$(LIBS): %:
	@$(MAKE) -s -C $(AM_HOME)/$* archive

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
$(IMAGE).elf: $(OBJS) am $(LIBS)
	@echo + LD "->" $(IMAGE_REL).elf
	@$(LD) $(LDFLAGS) -o $(IMAGE).elf --start-group $(LINKAGE) --end-group

### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@ar rcs $(ARCHIVE) $(OBJS)

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

### Build order control
image: image-dep
archive: $(ARCHIVE)
image-dep: $(OBJS) am $(LIBS)
	@echo \# Creating image [$(ARCH)]
.PHONY: image image-dep archive run $(LIBS)

### Clean a single project (remove `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)

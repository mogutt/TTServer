#基本
CC = g++
CFLAGS=-Wall -Wno-deprecated -g -O2
LDFLAGS= -lpthread 
LN=/bin/ln -s 
AR=ar
CP=/bin/cp
RM=-/bin/rm -rf
ARCH=PC

#链接库名
LIB_NAME=
#链接库版本
LIB_VER=1.0.0

# 二进制目标
BIN=msfs

#源文件目录
SrcDir= . common http storage
#头文件目录
IncDir= . include/common include/http include/storage
#连接库目录
LibDir=

SRCS=$(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
INCS=$(foreach dir,$(IncDir),$(addprefix -I,$(dir)))
LINKS=$(foreach dir,$(LibDir),$(addprefix -L,$(dir)))
CFLAGS := $(CFLAGS) $(INCS)
LDFLAGS:= $(LINKS) $(LDFLAGS)

OBJS = $(SRCS:%.cpp=%.o)
.PHONY:all clean

all:$(BIN)
$(BIN):$(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)
	@echo " OK!\tComplie $@ "

%.o:%.cpp
	@echo "Compileing $@..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning files..."
	@$(RM) $(OBJS) $(BIN)

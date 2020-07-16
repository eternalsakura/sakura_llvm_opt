.PHONY : clean build run_licm
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/assignment3/cmake-build-debug/src/
# 替换成你的so名
MODULE_NAME = libAssignment3.so
# 替换成你的pass名
OPTION_LICM= -loop-invariant-code-motion

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S

all : build run_licm

build: loop.ll

loop.ll :
	${CC} -c ${CFLAGS} loop.c -o nopt_loop.ll
	opt -mem2reg nopt_loop.ll -S -o m2r_nopt_loop.ll

run_licm :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_LICM} m2r_nopt_loop.ll -S -o trans_loop.ll


clean :
	rm -rf *.ll
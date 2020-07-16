.PHONY : all clean build run_lo run_fi run_tf
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/assignment1/cmake-build-debug/SSA/
# 替换成你的so名
MODULE_NAME = libSSA.so
# 替换成你的pass名
OPTION_SSA = -ssa

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S

all : build run_ssa

build: benchmark.ll

benchmark.ll :
	clang -c ${CFLAGS} benchmark.c -o nopt_benchmark.ll
	opt -mem2reg nopt_benchmark.ll -S -o m2r_nopt_benchmark.ll

run_ssa:
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_SSA} m2r_nopt_benchmark.ll -S -o ssa_benchmark.ll

clean :
	rm -rf *.ll

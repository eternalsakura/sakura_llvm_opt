.PHONY : all clean build run_lo run_fi run_tf
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/assignment1/cmake-build-debug/passManager/
# 替换成你的so名
MODULE_NAME = libpassManager.so
# 替换成你的pass名
OPTION_AS = -analysis
OPTION_AT = -another-transform
OPTION_TF = -transform

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S

all : build run_as run_at run_tf

build: benchmark.ll

benchmark.ll :
	clang -c ${CFLAGS} benchmark.c -o nopt_benchmark.ll
	opt -mem2reg nopt_benchmark.ll -S -o m2r_nopt_benchmark.ll

run_as :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_AS} m2r_nopt_benchmark.ll -S -o trans_benchmark.ll

run_at:
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_AT} m2r_nopt_benchmark.ll -S -o trans_benchmark.ll

run_tf:
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_TF} m2r_nopt_benchmark.ll -S -o trans_benchmark.ll

run_all:
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_AS}  ${OPTION_TF} ${OPTION_AT} m2r_nopt_benchmark.ll -S -o trans_benchmark.ll

clean :
	rm -rf *.ll

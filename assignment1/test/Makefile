.PHONY : all clean build run_lo run_fi run_tf
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/assignment1/cmake-build-debug/src/
# 替换成你的so名
MODULE_NAME = libAssignment1.so
# 替换成你的pass名
OPTION_LO= -local-opts
OPTION_FI= -function-info
OPTION_TF = -transform

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S
opt_file = algebraic.ll constfold.ll strength.ll

all : build run_fi run_lo run_tf

build: ${opt_file} loop.ll benchmark.ll

${opt_file}: %.ll: %.c
	${CC} -c ${CFLAGS} $< -o nopt_$@
	opt -mem2reg nopt_$@ -S -o m2r_nopt_$@

loop.ll :
	clang -c ${CFLAGS} loop.c -o nopt_loop.ll
	opt -mem2reg nopt_loop.ll -S -o m2r_nopt_loop.ll

benchmark.ll :
	clang -c ${CFLAGS} benchmark.c -o nopt_benchmark.ll
	opt -mem2reg nopt_benchmark.ll -S -o m2r_nopt_benchmark.ll

run_lo :
	$(foreach n, $(opt_file), opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_LO}\
                                 	 m2r_nopt_${n} -S -o localopts_${n};)

run_fi :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_FI} m2r_nopt_loop.ll -S -o trans_loop.ll

run_tf:
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_TF} m2r_nopt_benchmark.ll -S -o trans_benchmark.ll

clean :
	rm -rf *.ll

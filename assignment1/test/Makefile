.PHONY : all clean build run_lo run_fi
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/llvm_test/cmake-build-debug/src/
# 替换成你的so名
MODULE_NAME = libAssignment1.so
# 替换成你的pass名
OPTION_LO= -local-opts
OPTION_FI= -function-info

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S
opt_file = algebraic.ll constfold.ll strength.ll

all : build run_fi run_lo

build: ${opt_file} loop.ll

${opt_file}: %.ll: %.c
	${CC} -c ${CFLAGS} $< -o nopt_$@
	opt -mem2reg nopt_$@ -S -o m2r_nopt_$@

loop.ll :
	clang -c -O -emit-llvm -S loop.c -o loop.ll

run_lo :
	$(foreach n, $(opt_file), opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_LO}\
                                 	 m2r_nopt_${n} -S -o localopts_${n};)

run_fi :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_FI} loop.ll -S -o loop.ll

clean :
	rm -rf *.ll

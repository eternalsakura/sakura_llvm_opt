.PHONY : run_ae run_la
# 替换成你的so存放的路径
MODULE_PATH = /Users/sakura/CLionProjects/assignment2/cmake-build-debug/src/
# 替换成你的so名
MODULE_NAME = libAssignment2.so
# 替换成你的pass名
OPTION_AE= -avail_expr
OPTION_LA= -liveness

CC = clang
CFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S

run_ae :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_AE} available-test-m2r.ll -S -o available-test-m2r.ll;)

run_la :
	opt -load ${MODULE_PATH}${MODULE_NAME} ${OPTION_LA} liveness-test-m2r.ll -S -o liveness-test-m2r.ll

LIBUV_PATH=/usr/local/Cellar/libuv/1.27.0
INCLUDE_PATH=${LIBUV_PATH}/include
LIB_DIR=${LIBUV_PATH}/lib

all:
	gcc -I ${INCLUDE_PATH} -L ${LIB_DIR} -luv  -o zluv zluv.c
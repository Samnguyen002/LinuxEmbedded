.PHONY: all clean shared_lib shared_linking run proc

CC := gcc
PARA := 
FLAGS := -lpthread -I.
LIB_DRIR := ./shared_lib_outplog
OBJ_DIR := ./obj_outplog

shared_lib:
#create object
	$(CC) -c -fPIC outp_log.c -o $(OBJ_DIR)/outp_log.o	$(FLAGS)
	gcc -c main.c -o main.o $(FLAGS)
#create shared library
	$(CC) -shared -o $(LIB_DRIR)/liboutp_log.so $(OBJ_DIR)/outp_log.o
shared_linking:	shared_lib
#linking
	$(CC) -o app2 main.o -L$(LIB_DRIR) -loutp_log	-lpthread
run:	shared_linking
#hệ thống sẽ tìm shared lib ở folder mặc định /usr/lib, nên cần đổi đia chỉ tìm kiếm
	LD_LIBRARY_PATH=$(LIB_DRIR) ./app2 $(PARA)

all: main.c	
	$(CC) -g -o app1 main.c	outp_log.c  $(FLAGS)	
clean:
	rm -rf app1	
	rm -rf app2
	rm -rf $(LIB_DRIR)/*.so
	rm -rf $(OBJ_DIR)/*.o

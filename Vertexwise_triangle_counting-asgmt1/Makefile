CC = gcc
MPICC = mpicc 
CILKCC=/usr/local/OpenCilk-9.0.1-Linux/bin/clang
CFLAGS = -O2
default: all 

triangle_v1.:
	$(CC) $(CFLAGS) -o triangle_v1 triangle_v1.c

printwithpthread:
	$(CC) $(CFLAGS) -o print_pthread printwithpthread.c -lpthread


all: clean triangle_v1 printwithpthread

test:
	@printf "\n** Running triangle_v1\n\n"
	./triangle_v1
	@printf "\n--------END OF triangle_v1----------\n"
	@printf "\n** Running printwithpthread\n\n"
	./print_pthread
	@printf "\n--------END OF print_pthread----------\n"
clean:
	rm -f triangle_v1 print_pthread

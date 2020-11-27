CC = gcc
MPICC = mpicc 
CILKCC=/usr/local/OpenCilk-9.0.1-Linux/bin/clang
CFLAGS = -O2
default: all 

triangle_v1:
	$(CC) $(CFLAGS) -o triangle_v1 triangle_v1.c



printwithpthread:
	$(CC) $(CFLAGS) -o print_pthread printwithpthread.c -lpthread
triangle_v2:
	$(CC) $(CFLAGS) -o triangle_v2 triangle_v2.c
all: clean triangle_v1 triangle_v2 printwithpthread
.PHONY: clean
test:
	@printf "\n** Running triangle_v1\n\n"
	./triangle_v1
	@printf "\n--------END OF triangle_v1------------\n"
	@printf "\n** Running printwithpthread\n\n"
	./print_pthread
	@printf "\n--------END OF print_pthread----------\n"
	@printf "\n**Running triangle_v2\n\n"
	./triangle_v2
	@printf "\n-------END OF triangle_v2-------------\n"
clean:
	rm -f triangle_v1 print_pthread triangle_v2

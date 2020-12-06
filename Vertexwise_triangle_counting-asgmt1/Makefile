CC = gcc
MPICC = mpicc 
CILKCC=/usr/local/OpenCilk-9.0.1-Linux/bin/clang
CFLAGS = -O2
default: all 

triangle_v1: triangle_v1.c
	$(CC) $(CFLAGS) -o triangle_v1 triangle_v1.c

printwithpthread: printwithpthread.c
	$(CC) $(CFLAGS) -o print_pthread printwithpthread.c -lpthread

triangle_v2: triangle_v2.c
	$(CC) $(CFLAGS) -o triangle_v2 triangle_v2.c

triangle_v3: mmio.o coo2csc.o triangle_v3.o readmtxfile.o  
	$(CC) $(CFLAGS) -o triangle_v3 mmio.c coo2csc.c readmtxfile.c triangle_v3.c

triangle_v3_cilk: mmio.o coo2csc.o triangle_v3_cilk.c
	$(CILKCC) $(CFLAGS) -o triangle_v3_cilk mmio.c coo2csc.c triangle_v3_cilk.c -fcilkplus -lm

readmtxfile: mmio.o readmtxfile.c
	$(CC) $(CFLAGS) -o readfile mmio.c readmtxfile.c

triangle_v3_opmp: mmio.o coo2csc.o triangle_v3_opmp.c
	$(CC) $(CFLAGS) -o triangle_v3_opmp mmio.c coo2csc.c triangle_v3_opmp.c -fopenmp
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
all: triangle_v1 triangle_v2 triangle_v3 triangle_v3_cilk triangle_v3_opmp printwithpthread
.PHONY: clean
test:
	@printf "\n** Running triangle_v1\n\n"
	./triangle_v1 10
	@printf "\n--------END OF triangle_v1------------\n"
	@printf "\n** Running printwithpthread\n\n"
	./print_pthread
	@printf "\n--------END OF print_pthread----------\n"
	@printf "\n**Running triangle_v2\n\n"
	./triangle_v2 10
	@printf "\n-------END OF triangle_v2-------------\n"
	@printf "\n** Running triangle_v3\n\n"
	./triangle_v3 $(matrixfile)
	@printf "\n-------END OF triangle_v3-------------\n"
	@printf "\n** Running triangle_v3_cilk\n\n"
	./triangle_v3_cilk $(matrixfile)
	@printf "\n-------END OF triangle_v3_cilk-------------\n"
	./triangle_v3_opmp $(matrixfile)
	@printf "\n-------END OF triangle_v3_opmp-------------\n"
clean:
	rm -f triangle_v1 print_pthread triangle_v2 triangle_v3 triangle_v3_cilk triangle_v3_opmp mmio.o coo2csc.o 


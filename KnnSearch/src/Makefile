SHELL := /bin/bash

CC = gcc -O3
MPICC = mpicc
RM = rm -f

EXEC = v0 v1 v2 v2Random v1Random

all : $(EXEC)

v0 : v0.c 
	$(CC) $< -o ../output/$@ -lopenblas -lpthread -lm


v1:  v1.c
	$(MPICC) $< -o ../output/$@ -lopenblas -lpthread -lm

v2: v2.c	
	$(MPICC) $< -o ../output/$@ -lopenblas -lpthread -lm
v2Random: v2Random.c
	$(MPICC) $< -o ../output/$@ -lopenblas -lpthread -lm
v1Random: v1Random.c
	$(MPICC) $< -o ../output/$@ -lopenblas -lpthread -lm
	
clean:
		$(RM) *.o *~ $(EXEC)
default:
		all

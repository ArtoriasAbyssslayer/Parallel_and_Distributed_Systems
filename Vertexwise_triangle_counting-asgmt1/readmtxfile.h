#ifndef READFILE_H
#define READFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void print1dMatrix(int* array, int size);

void readfile_chk(FILE *f, uint32_t *m, uint32_t *n, uint32_t *nnz, int *i, int *j);

void readfile_scan(FILE *f, uint32_t *m, uint32_t *n, uint32_t *nnz, int *I, int *J, double *val);
#endif
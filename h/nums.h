#ifndef h_nums_
#define h_nums_

#include "main.h"

long str2l (const char *ptr);
unsigned long str2ul (const char *ptr);
double str2d (const char *ptr);

word strtoword (const char *ptr);
sword strtosword (const char *ptr);
flt strtoflt(const char *ptr);

int l2str (char *buff, unsigned long buffsize, long n);
int ul2str (char *buff, unsigned long buffsize, unsigned long n);
int d2str (char *buff, unsigned long buffsize, double n);

int sword2str (char *buff, unsigned long buffsize, sword n);
int word2str (char *buff, unsigned long buffsize, word n);
int flt2str (char *buff, unsigned long buffsize, flt n);

#endif

#ifndef h_str2n_
#define h_str2n_

long str2l (const char *ptr);
unsigned long str2ul (const char *ptr);
double str2d (const char *ptr);

int l2str (char *buff, unsigned long buffsize, long n);
int ul2str (char *buff, unsigned long buffsize, unsigned long n);
int d2str (char *buff, unsigned long buffsize, double n);

#endif

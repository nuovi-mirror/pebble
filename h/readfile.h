#ifndef h_readfile_
#define h_readfile_
char *readfile (const char *path, void *buf, unsigned long nbytes);
unsigned long getfilesize (const char *path);
#endif

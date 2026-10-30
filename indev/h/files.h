#ifndef h_files_
#define h_files_
char *readfile (const char *path, void *buf, unsigned long nbytes);
unsigned long getfilesize (const char *path);
unsigned long writefile (const char *path, const char *data, unsigned long datasize);
#endif /* #ifndef h_files_ */

#define error_fatal_int 1
#define error_warning_int 2

#define error_fatal_str "FATAL"
#define error_warning_str "WARNING"

void error (int err, const char *msg);
char *readfile (const char *path, char *buff, unsigned long size);

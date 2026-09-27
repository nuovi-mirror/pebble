#ifndef build_
#define build_

#undef NULL
#define NULL ((void *)0)

typedef struct Config {
	char *cflags; /* compiler flags */
	char *ldflags; /* linker flags */	

	char *program; /* output binary */
	char *compiler; /* C compiler path */
	char *precompiler; /* pre-compiler path */

	char *build; /* build type */
	char *platform; /* platform to build for */
	char *graphics; /* optional graphics backend */
} Config;

/* VM source code */
extern const char *vm_srcs[];

/* platform-specific sources */
extern const char *platform_freestand_srcs[];
extern const char *platform_c_srcs[];
extern const char *platform_posix_srcs[];

/* ffi source code */
extern const char *ffi_base_srcs[];

/* library code */
extern const char *ffi_c_srcs[];
extern const char *ffi_sdl2_srcs[];

/* helper.c */
void error (const char *msg);
void usage (void);

/* args.c */
void parseargs (Config *config, char *argument);

/* compile.c */
void compiledeps (Config *config, char *srcs, char *objs);
void compile (Config *config, char **srcs, unsigned int count, char **objs);
void link (Config *config, char **objs, unsigned int count);
void clean (Config *config, char **srcs, unsigned int count);

/* srcs.c */
const char *getmain (Config *config);
void addsrcs (const char **srcs, char **all, unsigned int *count);
unsigned int mksrclist (Config *config, char **srcs);

/* flags.c */
void setflags (Config *config);

#endif

#define MAX_SRC       (1 << 20)
#define MAX_LOCALS    64
#define MAX_NAME      64
#define MAX_VALUE     512
#define MAX_PATH      1024

#define MAX_HELPERS   4096
#define HELPER_BUFSZ  (1 << 16)

extern char src[MAX_SRC];
extern long pos;
extern long len;

/* every byte of src remembers which source unit and line it came from, so
 * errors stay accurate after include splices text into the middle of the
 * buffer. A unit is one inclusion of a file (the same file included twice is
 * two units), and each unit knows where it was included from, which is what
 * lets errors print a gcc-style include chain */
#define MAX_UNITS     1024

typedef struct Loc {
	int      file;  /* source unit index, -1 when unknown / none */
	unsigned line;  /* 1-based, relative to that unit */
} Loc;

extern unsigned short src_file[MAX_SRC];
extern unsigned       src_line[MAX_SRC];

extern char include_path[MAX_PATH];

extern char locals[MAX_LOCALS][MAX_NAME];
extern int nlocals;

extern unsigned next_control_id;
extern int has_main;

typedef struct Helper {
	char name[MAX_NAME];
	char body[HELPER_BUFSZ];
	unsigned long  body_len;
} Helper;

extern Helper helpers[MAX_HELPERS];
extern int nhelpers;

extern Helper *current_helper;

/* ---- compile-time checking ---- */
#define MAX_FUNCS     2048
#define MAX_CALLS     8192
#define MAX_REFS      8192
#define MAX_GLOBALS   2048

typedef struct FuncInfo {
	char name[MAX_NAME];
	int  nparams;   /* -1 when unknown (defined by a raw Pebble line) */
	int  nresults;  /* -1 when unknown */
	Loc  at;
} FuncInfo;

extern char cur_fn[MAX_NAME];
extern int local_assigned[MAX_LOCALS];
extern int nerrors;

void fail (const char *msg);
void fail_at (const char *msg, long at);
int add_source_unit (const char *name, Loc included_from);
void tag_source (long start, long n, int file);
Loc loc_at (long at);

void outf (const char *fmt, ...);
void outs (const char *s);
int flush_output (void);
int peekch (void);
void emitf (const char *fmt, ...);
Helper *new_helper (const char *name);
void emit_helper (const Helper *h);
void emit_helpers_from (int first);
int try_raw_line (void);
void skipws (void);
int is_ident_start (int c);
int is_ident_char (int c);
void read_ident (char *out, unsigned long cap);
void expect_char (char c);
int try_keyword (const char *kw);
void read_delimited (char close, char *out, unsigned long cap);
int is_local (const char *name);
void add_local (const char *name);
void mangle (const char *fn, const char *name, char *out, unsigned long cap);
void subst_expr (const char *fn, const char *text, char *out, unsigned long cap);
void include_file (const char *name);

void report_error (const char *msg, Loc at);
void report_warning (const char *msg, Loc at);
void define_func (const char *name, int nparams, int nresults);
void note_call (const char *name, int argc, int nbound, long at);
void define_global (const char *name);
void mark_assigned (const char *name);
int is_assigned (const char *name);
void use_var (const char *name, long at);
void use_text_var (const char *text, long at);
int finish_checks (void);

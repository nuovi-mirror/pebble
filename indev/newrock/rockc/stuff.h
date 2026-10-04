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

void fail (const char *msg);
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

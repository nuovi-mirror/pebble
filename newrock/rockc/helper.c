#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "stuff.h"
#include "stdlib.h"

char src[MAX_SRC];
long pos;
long len;

unsigned short src_file[MAX_SRC];
unsigned       src_line[MAX_SRC];

typedef struct SrcUnit {
	char     name[MAX_PATH];
	int      parent;       /* unit that included this one, -1 for the main file */
	unsigned parent_line;  /* line of the include statement inside the parent */
	int      trusted;      /* raw '.' Pebble lines are allowed in this exact unit */
} SrcUnit;

static SrcUnit units[MAX_UNITS];
static int nunits;
static int last_diag_unit = -2;

char include_path[MAX_PATH];

char locals[MAX_LOCALS][MAX_NAME];
int nlocals;

unsigned next_control_id;
int has_main;

Helper helpers[MAX_HELPERS];
int nhelpers;

Helper *current_helper;

char cur_fn[MAX_NAME];
int local_assigned[MAX_LOCALS];
int nerrors;

typedef struct CallRef {
	char name[MAX_NAME];
	int  argc;
	int  nbound;
	Loc  at;
} CallRef;

typedef struct VarRef {
	char name[MAX_NAME];
	Loc  at;
} VarRef;

static FuncInfo funcs[MAX_FUNCS];
static int nfuncs;

static CallRef calls[MAX_CALLS];
static int ncalls;

static VarRef vrefs[MAX_REFS];
static int nvrefs;

static char globals[MAX_GLOBALS][MAX_NAME];
static int nglobals;

int add_source_unit
(const char *name, Loc included_from, int trusted)
{
	if (nunits >= MAX_UNITS) return -1;

	SrcUnit *u = &units[nunits];

	strncpy(u->name, name, MAX_PATH - 1);
	u->name[MAX_PATH - 1] = '\0';
	u->parent = included_from.file;
	u->parent_line = included_from.line;
	u->trusted = trusted;

	return nunits++;
}

void tag_source
(long start, long n, int file)
{
	unsigned line = 1;

	for (long i = 0; i < n; i++)
	{
		src_file[start + i] = (unsigned short)file;
		src_line[start + i] = line;

		if (src[start + i] == '\n') line++;
	}
}

Loc loc_at
(long at)
{
	Loc l;

	l.file = -1;
	l.line = 0;

	if (nunits == 0) return l;
	if (at < 0) at = 0;

	if (at >= len) at = len > 0 ? len - 1 : 0;

	l.file = src_file[at];
	l.line = src_line[at];

	return l;
}

static void print_include_chain
(int unit)
{
	int printed = 0;

	for (int u = unit; units[u].parent >= 0; u = units[u].parent)
	{
		fprintf(stderr, printed ? ",\n                 from %s:%u" : "In file included from %s:%u", units[units[u].parent].name, units[u].parent_line);
		printed = 1;
	}

	if (printed) fputs(":\n", stderr);
}

static void print_diag
(const char *kind, const char *msg, Loc l)
{
	if (l.file < 0)
	{
		fprintf(stderr, "rockc: %s: %s\n", kind, msg);
		return;
	}

	if (l.file != last_diag_unit)
	{
		print_include_chain(l.file);
		last_diag_unit = l.file;
	}

	fprintf(stderr, "%s:%u: %s: %s\n", units[l.file].name, l.line, kind, msg);
}

void fail_at
(const char *msg, long at)
{
	print_diag("error", msg, loc_at(at));
	exit(1);
}

void fail
(const char *msg)
{ fail_at(msg, pos); }

static char *obuf;
static size_t olen;
static size_t ocap;

static void out_reserve
(size_t extra)
{
	if (olen + extra + 1 <= ocap) return;
	size_t ncap = ocap ? ocap : 4096;
	while (ncap < olen + extra + 1) ncap *= 2;
	char *p = realloc(obuf, ncap);
	if (!p) fail("out of memory while buffering output");

	obuf = p;
	ocap = ncap;
}

static void voutf
(const char *fmt, va_list ap)
{
	va_list copy;

	va_copy(copy, ap);
	int n = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);

	if (n < 0) fail("failed to generate output");

	out_reserve((size_t)n);
	vsnprintf(obuf + olen, (size_t)n + 1, fmt, ap);
	olen += (size_t)n;
}

void outf
(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	voutf(fmt, ap);
	va_end(ap);
}

void outs
(const char *s)
{
	size_t n = strlen(s);

	out_reserve(n);
	memcpy(obuf + olen, s, n);
	olen += n;
	obuf[olen] = '\0';
}

int flush_output
(void)
{
	if (olen > 0 && fwrite(obuf, 1, olen, stdout) != olen) return -1;
	return fflush(stdout) == 0 ? 0 : -1;
}

int peekch
(void)
{ return pos < len ? (unsigned char)src[pos] : -1; }

void emitf
(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if (current_helper)
	{
		size_t available = HELPER_BUFSZ - current_helper->body_len;
		if (available == 0) fail("generated helper is too large");

		int n = vsnprintf(current_helper->body + current_helper->body_len, available, fmt, ap);

		if (n < 0) fail("failed to generate output");
		if ((size_t)n >= available) fail("generated helper is too large");

		current_helper->body_len += (size_t)n;
	}
	else
	{
		voutf(fmt, ap);
	}

	va_end(ap);
}

Helper *new_helper
(const char *name)
{
	if (nhelpers >= MAX_HELPERS) fail("too many generated helpers");

	Helper *h = &helpers[nhelpers++];

	memset(h, 0, sizeof *h);
	strncpy(h->name, name, MAX_NAME - 1);
	h->name[MAX_NAME - 1] = '\0';

	return h;
}

void emit_helper
(const Helper *h)
{
	outf("Func %s\n", h->name);
	outs(h->body);

	if (h->body_len == 0 || h->body[h->body_len - 1] != '\n') outs("\n");
	outs("End\n");
}

void emit_helpers_from
(int first)
{
	for (int i = first; i < nhelpers; i++) emit_helper(&helpers[i]);
}

static void scan_raw_line
(const char *line)
{
	char op[MAX_NAME], arg[MAX_NAME];
	int n = 0;

	n = sscanf(line, " %63s %63s", op, arg);
	if (n < 2) return;

	if (strcmp(op, "Func") == 0)
	{
		define_func(arg, -1, -1);
		return;
	}

	if (strcmp(op, "New") != 0) return;

	if (cur_fn[0])
	{
		char prefix[MAX_NAME + 16];
		size_t plen;

		snprintf(prefix, sizeof prefix, "__Func_%s_", cur_fn);
		plen = strlen(prefix);

		if (strncmp(arg, prefix, plen) == 0)
		{
			mark_assigned(arg + plen);
			return;
		}
	}

	if (is_ident_start((unsigned char)arg[0]) && !(arg[0] == '_' && arg[1] == '_')) define_global(arg);
}

int try_raw_line
(void)
{
	char line[256];
	size_t nline = 0;

	if (pos > 0 && src[pos - 1] != '\n') return 0;
	if (peekch() != '.') return 0;

	if (!units[loc_at(pos).file].trusted) fail_at("WARNING: Pebble injection attempted! This is usually done to get around compiler saftey checks; Is this program malicious? Please use the wrappers provided by stdlib!", pos);

	pos++;
	if (pos < len && src[pos] != '\n') pos++;

	while (pos < len && src[pos] != '\n')
	{
		if (nline + 1 < sizeof line) line[nline++] = src[pos];
		emitf("%c", src[pos++]);
	}

	line[nline] = '\0';
	scan_raw_line(line);
	emitf("\n");
	if (pos < len && src[pos] == '\n') pos++;

	return 1;
}

void skipws
(void)
{
	for (;;)
	{
		while (pos < len && isspace((unsigned char)src[pos])) pos++;
		if (pos < len && (src[pos] == '#' || src[pos] == '/'))
		{
			while (pos < len && src[pos] != '\n') pos++;
			continue;
		}

		break;
	}
}

int is_ident_start
(int c)
{ return isalpha(c) || c == '_'; }

int is_ident_char
(int c)
{ return isalnum(c) || c == '_' || c == '.'; }

void read_ident
(char *out, size_t cap)
{
	skipws();
	if (!is_ident_start(peekch())) fail("expected an identifier");

	size_t n = 0;

	while (pos < len && is_ident_char((unsigned char)src[pos]))
	{
		if (n + 1 >= cap) fail("identifier too long");
		out[n++] = src[pos++];
	}

	out[n] = '\0';
}

void expect_char
(char c)
{
	skipws();

	if (peekch() != c) fail("unexpected character");
	pos++;
}

int try_keyword
(const char *kw)
{
	skipws();
	long save = pos;

	if (!is_ident_start(peekch())) return 0;

	char buf[MAX_NAME];
	size_t n = 0;

	while (pos < len && is_ident_char((unsigned char)src[pos]))
	{
		if (n + 1 < sizeof buf) buf[n++] = src[pos];
		pos++;
	}

	buf[n] = '\0';

	if (strcmp(buf, kw) == 0) return 1;

	pos = save;
	return 0;
}

void read_delimited
(char close, char *out, size_t cap)
{
	size_t n = 0;

	while (pos < len && src[pos] != close)
	{
		if (n + 1 >= cap) fail("quoted text too long");
		out[n++] = src[pos++];
	}

	if (pos >= len) fail("unterminated quote");

	out[n] = '\0';
	pos++;
}

int is_local
(const char *name)
{
	for (int i = 0; i < nlocals; i++) if (strcmp(locals[i], name) == 0) return 1;
	return 0;
}

void add_local
(const char *name)
{
	if (is_local(name)) return;
	if (nlocals >= MAX_LOCALS) fail("too many locals in one function");

	strncpy(locals[nlocals++], name, MAX_NAME - 1);
	locals[nlocals - 1][MAX_NAME - 1] = '\0';
	local_assigned[nlocals - 1] = 0;
}

void mark_assigned
(const char *name)
{
	for (int i = 0; i < nlocals; i++) if (strcmp(locals[i], name) == 0) local_assigned[i] = 1;
}

int is_assigned
(const char *name)
{
	for (int i = 0; i < nlocals; i++) if (strcmp(locals[i], name) == 0) return local_assigned[i];
	return 0;
}

void mangle
(const char *fn, const char *name, char *out, size_t cap)
{
	if (is_local(name)) snprintf(out, cap, "__Func_%s_%s", fn, name);
	else snprintf(out, cap, "%s", name);
}

static void shift_source
(long at, long n)
{
	size_t count = (size_t)(len - at) + 1;

	memmove(src + at + n, src + at, count);
	memmove(src_file + at + n, src_file + at, count * sizeof *src_file);
	memmove(src_line + at + n, src_line + at, count * sizeof *src_line);
}

void include_file
(const char *name)
{
	char path[MAX_PATH];
	size_t path_len = strlen(include_path);
	size_t name_len = strlen(name);
	long at = pos; /* errors point at the include statement itself */
	Loc site = loc_at(at);

	if (pos < len && src[pos] == '\n') pos++;

	if (strcmp(name, "stdlib") == 0)
	{
		size_t stdlib_len = strlen(stdlib_data);
		if (stdlib_len + 1 > MAX_SRC - len - 1) fail_at("source buffer is too small for standard library", at);
		int unit = add_source_unit("<stdlib>", site, 1 /* trusted: this is the literal stdlib text */);
		if (unit < 0) fail_at("too many nested or repeated includes (include cycle?)", at);

		shift_source(pos, (long)stdlib_len + 1);
		memcpy(src + pos, stdlib_data, stdlib_len);
		src[pos + (long)stdlib_len] = '\n';
		tag_source(pos, (long)stdlib_len + 1, unit);

		len += stdlib_len + 1;

		return;
	}

	if (path_len + 1 + name_len + 1 > sizeof path) fail_at("include path is too long", at);

	if (path_len > 0 && include_path[path_len - 1] == '/') snprintf(path, sizeof path, "%s%s", include_path, name);
	else snprintf(path, sizeof path, "%s/%s", include_path, name);

	for (int u = site.file; u >= 0; u = units[u].parent)
	{
		if (strcmp(units[u].name, path) == 0)
		{
			char msg[MAX_PATH + 48];

			snprintf(msg, sizeof msg, "'%s' is included recursively", path);
			fail_at(msg, at);
		}
	}

	FILE *file = fopen(path, "r");

	if (!file)
	{
		char msg[MAX_PATH + 32];

		snprintf(msg, sizeof msg, "could not open include file '%s'", path);
		fail_at(msg, at);
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		fail_at("could not seek include file", at);
	}

	long file_len = ftell(file);

	if (file_len < 0)
	{
		fclose(file);
		fail_at("could not determine include file size", at);
	}

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		fail_at("could not seek include file", at);
	}

	if ((unsigned long)file_len > (unsigned long)(MAX_SRC - len - 1))
	{
		fclose(file);
		fail_at("source buffer is too small for included file", at);
	}

	int unit = add_source_unit(path, site, 0 /* ordinary included file: not trusted */);

	if (unit < 0)
	{
		fclose(file);
		fail_at("too many nested or repeated includes (include cycle?)", at);
	}

	shift_source(pos, file_len);
	size_t got = fread(src + pos, 1, (size_t)file_len, file);
	fclose(file);

	if (got != (size_t)file_len) fail_at("could not read include file", at);
	tag_source(pos, file_len, unit);
	len += file_len;
}

void subst_expr
(const char *fn, const char *text, char *out, unsigned long cap)
{
	unsigned long oi = 0;
	const char *p = text;

	while (*p)
	{
		while (*p == ' ')
		{
			if (oi + 1 < cap) out[oi++] = *p;
			p++;
		}

		if (!*p) break;

		char word[MAX_NAME];
		unsigned long wi = 0;

		while (*p && *p != ' ')
		{
			if (wi + 1 < sizeof word) word[wi++] = *p;
			p++;
		}

		word[wi] = '\0';

		char mangled[MAX_NAME];

		if (is_local(word)) snprintf(mangled, sizeof mangled, "__Func_%s_%s", fn, word);
		else snprintf(mangled, sizeof mangled, "%s", word);

		for (unsigned long k = 0; mangled[k] && oi + 1 < cap; k++) out[oi++] = mangled[k];
	}

	out[oi] = '\0';
}

/* compiler saftey checks */

void report_error
(const char *msg, Loc at)
{
	print_diag("error", msg, at);
	nerrors++;
}

void report_warning
(const char *msg, Loc at)
{ print_diag("warning", msg, at); }

static FuncInfo *find_func
(const char *name)
{
	for (int i = 0; i < nfuncs; i++) if (strcmp(funcs[i].name, name) == 0) return &funcs[i];
	return NULL;
}

void define_func
(const char *name, int nparams, int nresults)
{
	/* XXX allow function to be re-defined
	if (find_func(name))
	{
		char msg[MAX_NAME + 48];

		snprintf(msg, sizeof msg, "function '%s' is defined more than once", name);
		fail(msg);
	}
	*/

	if (nfuncs >= MAX_FUNCS) fail("too many functions");

	FuncInfo *f = &funcs[nfuncs++];

	strncpy(f->name, name, MAX_NAME - 1);
	f->name[MAX_NAME - 1] = '\0';
	f->nparams = nparams;
	f->nresults = nresults;
	f->at = loc_at(pos);
}

void note_call
(const char *name, int argc, int nbound, long at)
{
	if (ncalls >= MAX_CALLS) fail("too many function calls");

	CallRef *r = &calls[ncalls++];

	strncpy(r->name, name, MAX_NAME - 1);
	r->name[MAX_NAME - 1] = '\0';
	r->argc = argc;
	r->nbound = nbound;
	r->at = loc_at(at);
}

static int has_global
(const char *name)
{
	for (int i = 0; i < nglobals; i++) if (strcmp(globals[i], name) == 0) return 1;
	return 0;
}

void define_global
(const char *name)
{
	if (has_global(name)) return;
	if (nglobals >= MAX_GLOBALS) fail("too many global variables");

	strncpy(globals[nglobals], name, MAX_NAME - 1);
	globals[nglobals][MAX_NAME - 1] = '\0';
	nglobals++;
}

void use_var
(const char *name, long at)
{
	if (is_local(name))
	{
		if (!is_assigned(name))
		{
			char msg[MAX_NAME + 64];

			snprintf(msg, sizeof msg, "variable '%s' is used before it is assigned", name);
			report_error(msg, loc_at(at));
		}

		return;
	}

	if (name[0] == '_' && name[1] == '_') return;
	if (nvrefs >= MAX_REFS) fail("too many variable references");

	VarRef *r = &vrefs[nvrefs++];

	strncpy(r->name, name, MAX_NAME - 1);
	r->name[MAX_NAME - 1] = '\0';
	r->at = loc_at(at);
}

void use_text_var
(const char *text, long at)
{
	char name[MAX_NAME];
	size_t n = 0;

	while (*text == ' ') text++;
	if (!is_ident_start((unsigned char)*text)) return;

	while (*text && is_ident_char((unsigned char)*text))
	{
		if (n + 1 >= sizeof name) return;
		name[n++] = *text++;
	}

	name[n] = '\0';

	while (*text == ' ') text++;
	if (*text) return;
	use_var(name, at);
}

int finish_checks
(void)
{
	char msg[3 * MAX_NAME + 96];

	for (int i = 0; i < ncalls; i++)
	{
		const CallRef *r = &calls[i];
		const FuncInfo *f = find_func(r->name);

		if (!f)
		{
			snprintf(msg, sizeof msg, "call to undefined function '%s'", r->name);
			report_error(msg, r->at);
			continue;
		}

		if (f->nparams >= 0 && r->argc != f->nparams)
		{
			snprintf(msg, sizeof msg, "function '%s' takes %d argument(s) but %d given", r->name, f->nparams, r->argc);
			report_error(msg, r->at);
		}

		if (f->nresults >= 0 && r->nbound > f->nresults)
		{
			snprintf(msg, sizeof msg, "function '%s' returns %d value(s) but %d are being assigned", r->name, f->nresults, r->nbound);
			report_error(msg, r->at);
		}
	}

	for (int i = 0; i < nvrefs; i++)
	{
		if (has_global(vrefs[i].name)) continue;

		snprintf(msg, sizeof msg, "undeclared variable '%s' (not a local, an export, or a raw New)", vrefs[i].name);
		report_error(msg, vrefs[i].at);
	}

	if (has_main)
	{
		const FuncInfo *m = find_func("main");

		if (m && m->nparams > 0) report_error("'main' is called with no arguments, so it cannot take parameters", m->at);
	}

	return nerrors;
}

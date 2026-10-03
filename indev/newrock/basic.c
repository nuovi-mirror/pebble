/* simple New Rock demo compiler
 * uses Pebble AAE schematics instead of older Rock marker symbols
 * reads program from stdin and writes compiled Pebble code to stdout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SRC      (1 << 20)
#define MAX_LOCALS   64
#define MAX_NAME     64
#define MAX_VALUE    512

static char src[MAX_SRC];
static long pos;
static long len;

static char locals[MAX_LOCALS][MAX_NAME];
static int nlocals;

static void fail(const char *msg)
{
	fprintf(stderr, "rockc: error: %s (at byte %ld)\n", msg, pos);
	exit(1);
}

static int peekch (void) 
{ 
	return pos < len ? (unsigned char)src[pos] : -1; 
}

static void skipws (void)
{
	for (;;) {
		while (pos < len && isspace((unsigned char)src[pos])) pos++;
		if (pos + 1 < len && src[pos] == '/' && src[pos + 1] == '/') {
			while (pos < len && src[pos] != '\n') pos++;
			continue;
		}
		break;
	}
}

static int is_ident_start(int c) { return isalpha(c) || c == '_'; }
static int is_ident_char(int c)  { return isalnum(c) || c == '_' || c == '.'; }

static void read_ident(char *out, size_t cap)
{
	skipws();
	if (!is_ident_start(peekch()))
		fail("expected an identifier");

	size_t n = 0;
	while (pos < len && is_ident_char((unsigned char)src[pos])) {
		if (n + 1 >= cap) fail("identifier too long");
		out[n++] = src[pos++];
	}
	out[n] = '\0';
}

static void expect_char(char c)
{
	skipws();
	if (peekch() != c)
		fail("unexpected character");
	pos++;
}

static int try_keyword(const char *kw)
{
	skipws();
	long save = pos;
	if (!is_ident_start(peekch())) return 0;

	char buf[MAX_NAME];
	size_t n = 0;
	while (pos < len && is_ident_char((unsigned char)src[pos])) {
		if (n + 1 < sizeof buf) buf[n++] = src[pos];
		pos++;
	}
	buf[n] = '\0';

	if (strcmp(buf, kw) == 0) return 1;
	pos = save;
	return 0;
}

static void read_delimited(char close, char *out, size_t cap)
{
	size_t n = 0;
	while (pos < len && src[pos] != close) {
		if (n + 1 >= cap) fail("quoted text too long");
		out[n++] = src[pos++];
	}
	if (pos >= len) fail("unterminated quote");
	out[n] = '\0';
	pos++; /* consume the closing delimiter */
}

static int is_local(const char *name)
{
	for (int i = 0; i < nlocals; i++)
		if (strcmp(locals[i], name) == 0) return 1;
	return 0;
}

static void add_local(const char *name)
{
	if (is_local(name)) return;
	if (nlocals >= MAX_LOCALS) fail("too many locals in one function");
	strncpy(locals[nlocals++], name, MAX_NAME - 1);
}

static void mangle(const char *fn, const char *name, char *out, size_t cap)
{
	if (is_local(name))
		snprintf(out, cap, "__Func_%s_%s", fn, name);
	else
		snprintf(out, cap, "%s", name); /* global: left exactly as named */
}

static void subst_expr(const char *fn, const char *text, char *out, size_t cap)
{
	size_t oi = 0;
	const char *p = text;

	while (*p) {
		while (*p == ' ') { if (oi + 1 < cap) out[oi++] = *p; p++; }
		if (!*p) break;

		char word[MAX_NAME];
		size_t wi = 0;
		while (*p && *p != ' ') {
			if (wi + 1 < sizeof word) word[wi++] = *p;
			p++;
		}
		word[wi] = '\0';

		char mangled[MAX_NAME];
		if (is_local(word)) {
			snprintf(mangled, sizeof mangled, "__Func_%s_%s", fn, word);
		} else {
			snprintf(mangled, sizeof mangled, "%s", word);
		}
		for (size_t k = 0; mangled[k] && oi + 1 < cap; k++)
			out[oi++] = mangled[k];
	}
	out[oi] = '\0';
}

static void compile_value(const char *fn, char *out, size_t cap)
{
	skipws();
	int c = peekch();

	if (c == '\'') {
		pos++;
		char text[MAX_VALUE];
		read_delimited('\'', text, sizeof text);
		snprintf(out, cap, "'%s'", text);
		return;
	}

	if (c == '"') {
		pos++;
		char text[MAX_VALUE], subst[MAX_VALUE];
		read_delimited('"', text, sizeof text);
		subst_expr(fn, text, subst, sizeof subst);
		snprintf(out, cap, "\"%s\"", subst);
		return;
	}

	if (is_ident_start(c)) {
		char name[MAX_NAME];
		read_ident(name, sizeof name);
		mangle(fn, name, out, cap);
		return;
	}

	fail("expected a value ('literal', \"expression\", or a bare name)");
}

static void compile_call(const char *fn, const char *dest_local_or_null)
{
	char callee[MAX_NAME];
	read_ident(callee, sizeof callee);
	expect_char('(');

	skipws();
	int argc = 0;
	while (peekch() != ')') {
		char val[MAX_VALUE];
		compile_value(fn, val, sizeof val);
		printf("New __Func_%s_ARG%d %s\n", callee, argc, val);
		argc++;
		skipws();
	}
	expect_char(')');

	printf("Call %s\n", callee);

	if (dest_local_or_null) {
		char mangled[MAX_NAME];
		mangle(fn, dest_local_or_null, mangled, sizeof mangled);
		printf("New %s __Func_%s_RET0\n", mangled, callee);
	}
}

static void compile_stmt(const char *fn)
{
	if (try_keyword("import")) {
		char name[MAX_NAME];
		read_ident(name, sizeof name);
		add_local(name);
		printf("New __Func_%s_%s %s\n", fn, name, name);
		return;
	}

	if (try_keyword("export")) {
		char name[MAX_NAME];
		read_ident(name, sizeof name);
		/* exported name must already be a known local to have anything
		 * meaningful to copy out */
		printf("New %s __Func_%s_%s\n", name, fn, name);
		return;
	}

	if (try_keyword("call")) {
		compile_call(fn, NULL);
		return;
	}

	/* IDENT '=' ( 'call' IDENT '(' value* ')' | value ) */
	char name[MAX_NAME];
	read_ident(name, sizeof name);
	add_local(name);
	expect_char('=');

	if (try_keyword("call")) {
		compile_call(fn, name);
		return;
	}

	char val[MAX_VALUE];
	compile_value(fn, val, sizeof val);
	char mangled[MAX_NAME];
	mangle(fn, name, mangled, sizeof mangled);
	printf("New %s %s\n", mangled, val);
}

static void compile_fn(void)
{
	char fn[MAX_NAME];
	read_ident(fn, sizeof fn);

	nlocals = 0;
	char params[MAX_LOCALS][MAX_NAME];
	int nparams = 0;

	expect_char('(');
	skipws();
	while (peekch() != ')') {
		read_ident(params[nparams], sizeof params[nparams]);
		add_local(params[nparams]);
		nparams++;
		skipws();
	}
	expect_char(')');

	char result[MAX_NAME];
	read_ident(result, sizeof result);
	add_local(result);

	expect_char('{');

	printf("Func %s\n", fn);
	for (int i = 0; i < nparams; i++)
		printf("New __Func_%s_%s __Func_%s_ARG%d\n", fn, params[i], fn, i);

	skipws();
	while (peekch() != '}') {
		compile_stmt(fn);
		skipws();
	}
	expect_char('}');

	printf("New __Func_%s_RET0 __Func_%s_%s\n", fn, fn, result);
	printf("End\n");
}

static void compile_top_stmt(void)
{
	if (try_keyword("call")) {
		compile_call("", NULL);
		return;
	}

	fail("expected 'fn' or 'call' at top level");
}

int main(void)
{
	len = (long)fread(src, 1, sizeof src - 1, stdin);
	src[len] = '\0';
	pos = 0;

	skipws();

	while (pos < len) {
		if (try_keyword("fn")) {
			compile_fn();
		} else {
			compile_top_stmt();
		}

		skipws();
	}

	return 0;
}

/* simple New Rock demo compiler
 * uses Pebble AAE schematics instead of older Rock marker symbols
 * reads program from a file and writes compiled Pebble code to stdout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "stdlib.h"

#define MAX_SRC       (1 << 20)
#define MAX_LOCALS    64
#define MAX_NAME      64
#define MAX_VALUE     512
#define MAX_PATH      1024

#define MAX_HELPERS   4096
#define HELPER_BUFSZ  (1 << 16)

static char src[MAX_SRC];
static long pos;
static long len;

static char include_path[MAX_PATH];

static char locals[MAX_LOCALS][MAX_NAME];
static int nlocals;

static unsigned next_control_id;
static int has_main;

typedef struct Helper {
	char name[MAX_NAME];
	char body[HELPER_BUFSZ];
	size_t body_len;
} Helper;

static Helper helpers[MAX_HELPERS];
static int nhelpers;

static Helper *current_helper;

static void fail
(const char *msg)
{
	fprintf(stderr, "rockc: error: %s (at byte %ld)\n", msg, pos);
	exit(1);
}

static int peekch
(void)
{ return pos < len ? (unsigned char)src[pos] : -1; }

static void emitf
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
		vprintf(fmt, ap);
	}

	va_end(ap);
}

static Helper *new_helper
(const char *name)
{
	if (nhelpers >= MAX_HELPERS) fail("too many generated helpers");

	Helper *h = &helpers[nhelpers++];

	memset(h, 0, sizeof *h);
	strncpy(h->name, name, MAX_NAME - 1);
	h->name[MAX_NAME - 1] = '\0';

	return h;
}

static void emit_helper
(const Helper *h)
{
	printf("Func %s\n", h->name);
	fputs(h->body, stdout);

	if (h->body_len == 0 || h->body[h->body_len - 1] != '\n') putchar('\n');
	printf("End\n");
}

static void emit_helpers_from
(int first)
{
	for (int i = first; i < nhelpers; i++) emit_helper(&helpers[i]);
}

static int try_raw_line
(void)
{
	if (pos > 0 && src[pos - 1] != '\n') return 0;
	if (peekch() != '.') return 0;

	pos++; /* consume '.' */

	if (pos < len && src[pos] != '\n') pos++;

	while (pos < len && src[pos] != '\n')
		emitf("%c", src[pos++]);

	emitf("\n");

	if (pos < len && src[pos] == '\n') pos++;

	return 1;
}

static void skipws
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

static int is_ident_start
(int c)
{ return isalpha(c) || c == '_'; }

static int is_ident_char
(int c)
{ return isalnum(c) || c == '_' || c == '.'; }

static void read_ident
(char *out, size_t cap)
{
	skipws();

	if (!is_ident_start(peekch()))
		fail("expected an identifier");

	size_t n = 0;

	while (pos < len && is_ident_char((unsigned char)src[pos]))
	{
		if (n + 1 >= cap) fail("identifier too long");
		out[n++] = src[pos++];
	}

	out[n] = '\0';
}

static void expect_char
(char c)
{
	skipws();

	if (peekch() != c) fail("unexpected character");
	pos++;
}

static int try_keyword
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

static void read_delimited
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

static int is_local
(const char *name)
{
	for (int i = 0; i < nlocals; i++) if (strcmp(locals[i], name) == 0) return 1;
	return 0;
}

static void add_local
(const char *name)
{
	if (is_local(name)) return;
	if (nlocals >= MAX_LOCALS) fail("too many locals in one function");

	strncpy(locals[nlocals++], name, MAX_NAME - 1);
	locals[nlocals - 1][MAX_NAME - 1] = '\0';
}

static void mangle
(const char *fn, const char *name, char *out, size_t cap)
{
	if (is_local(name)) snprintf(out, cap, "__Func_%s_%s", fn, name);
	else snprintf(out, cap, "%s", name);
}

static void subst_expr
(const char *fn, const char *text, char *out, size_t cap)
{
	size_t oi = 0;
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
		size_t wi = 0;

		while (*p && *p != ' ')
		{
			if (wi + 1 < sizeof word) word[wi++] = *p;
			p++;
		}

		word[wi] = '\0';

		char mangled[MAX_NAME];

		if (is_local(word)) snprintf(mangled, sizeof mangled, "__Func_%s_%s", fn, word);
		else snprintf(mangled, sizeof mangled, "%s", word);

		for (size_t k = 0; mangled[k] && oi + 1 < cap; k++) out[oi++] = mangled[k];
	}

	out[oi] = '\0';
}

static void include_file
(const char *name)
{
	char path[MAX_PATH];
	size_t path_len = strlen(include_path);
	size_t name_len = strlen(name);

	if (pos < len && src[pos] == '\n') pos++;

	if (strcmp(name, "stdlib") == 0)
	{
		size_t stdlib_len = strlen(stdlib);

		if (stdlib_len + 1 > MAX_SRC - len - 1) fail("source buffer is too small for standard library");

		memmove(src + pos + stdlib_len + 1, src + pos, (size_t)(len - pos) + 1);
		memcpy(src + pos, stdlib, stdlib_len);
		src[pos + stdlib_len] = '\n';

		len += stdlib_len + 1;

		return;
	}

	if (path_len + 1 + name_len + 1 > sizeof path) fail("include path is too long");

	if (path_len > 0 && include_path[path_len - 1] == '/') snprintf(path, sizeof path, "%s%s", include_path, name);
	else snprintf(path, sizeof path, "%s/%s", include_path, name);

	FILE *file = fopen(path, "r");

	if (!file)
	{
		char msg[MAX_PATH + 32];

		snprintf(msg, sizeof msg, "could not open include file '%s'", path);
		fail(msg);
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		fail("could not seek include file");
	}

	long file_len = ftell(file);

	if (file_len < 0)
	{
		fclose(file);
		fail("could not determine include file size");
	}

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		fail("could not seek include file");
	}

	if ((unsigned long)file_len >
		(unsigned long)(MAX_SRC - len - 1))
	{
		fclose(file);
		fail("source buffer is too small for included file");
	}

	memmove(src + pos + file_len, src + pos, (size_t)(len - pos) + 1);
	size_t got = fread(src + pos, 1, (size_t)file_len, file);
	fclose(file);

	if (got != (size_t)file_len) fail("could not read include file");

	len += file_len;
}

/* forward declaration */
static void compile_stmt(const char *fn);

static void compile_value
(const char *fn, char *out, size_t cap)
{
	skipws();

	int c = peekch();

	if (c == '\'')
	{
		pos++;

		char text[MAX_VALUE];

		read_delimited('\'', text, sizeof text);

		snprintf(out, cap, "'%s'", text);
		return;
	}

	if (c == '"')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('"', text, sizeof text);
		subst_expr(fn, text, subst, sizeof subst);

		snprintf(out, cap, "\"%s\"", subst);

		return;
	}

	if (is_ident_start(c))
	{
		char name[MAX_NAME];

		read_ident(name, sizeof name);
		mangle(fn, name, out, cap);

		return;
	}

	fail("expected a value ('literal', \"expression\", or a bare name)");
}

static void compile_call
(const char *fn, const char *dest_local_or_null)
{
	char callee[MAX_NAME];

	read_ident(callee, sizeof callee);

	expect_char('(');

	skipws();

	int argc = 0;

	while (peekch() != ')')
	{
		char val[MAX_VALUE];

		compile_value(fn, val, sizeof val);
		emitf("New __Func_%s_ARG%d %s\n", callee, argc, val);

		argc++;

		skipws();
	}

	expect_char(')');

	emitf("Call %s\n", callee);

	if (dest_local_or_null)
	{
		char mangled[MAX_NAME];

		mangle(fn, dest_local_or_null, mangled, sizeof mangled);
		emitf("New %s __Func_%s_RET0\n", mangled, callee);
	}
}

/* forward declaration */
static void compile_stmt(const char *fn);

static void compile_if_branch
(const char *fn, Helper *helper)
{
	Helper *old_helper = current_helper;

	current_helper = helper;

	skipws();

	while (peekch() != '}')
	{
		if (peekch() < 0)
			fail("unterminated if block");

		compile_stmt(fn);
		skipws();
	}

	expect_char('}');

	current_helper = old_helper;
}

/* forward declaration */
static void compile_stmt(const char *fn);

static void compile_if
(const char *fn)
{
#define MAX_IF_BRANCHES 64

	char conditions[MAX_IF_BRANCHES][MAX_VALUE];
	char body_fns[MAX_IF_BRANCHES][MAX_NAME];

	int nbranches = 0;
	int has_else = 0;
	char else_fn[MAX_NAME];

	for (;;)
	{
		if (nbranches >= MAX_IF_BRANCHES)
			fail("too many if branches");

		expect_char('(');

		compile_value(fn, conditions[nbranches], sizeof conditions[nbranches]);

		expect_char(')');
		expect_char('{');

		snprintf(body_fns[nbranches], sizeof body_fns[nbranches],
			"__RockIf_%u", next_control_id++);

		Helper *branch = new_helper(body_fns[nbranches]);

		compile_if_branch(fn, branch);

		nbranches++;

		skipws();

		if (!try_keyword("else"))
			break;

		skipws();

		if (try_keyword("if"))
			continue;

		expect_char('{');

		snprintf(else_fn, sizeof else_fn,
			"__RockElse_%u", next_control_id++);

		Helper *else_body = new_helper(else_fn);

		compile_if_branch(fn, else_body);

		has_else = 1;
		break;
	}

	char dispatch_fns[MAX_IF_BRANCHES][MAX_NAME];

	for (int i = 0; i < nbranches; i++)
		snprintf(dispatch_fns[i], sizeof dispatch_fns[i],
			"__RockDispatch_%u", next_control_id++);

	for (int i = 0; i < nbranches; i++)
	{
		Helper *old_helper = current_helper;
		Helper *dispatch = new_helper(dispatch_fns[i]);

		current_helper = dispatch;

		emitf("If %s %s\n", body_fns[i], conditions[i]);

		if (i + 1 < nbranches)
		{
			char inverse[MAX_VALUE];
			size_t n = strlen(conditions[i]);

			if (n >= 2 &&
				conditions[i][0] == '"' &&
				conditions[i][n - 1] == '"')
				snprintf(inverse, sizeof inverse,
					"\"(%.*s) != 0\"",
					(int)(n - 2),
					conditions[i] + 1);
			else
				snprintf(inverse, sizeof inverse,
					"(%s) != 0",
					conditions[i]);

			emitf("If %s %s\n",
				dispatch_fns[i + 1], inverse);
		}
		else if (has_else)
		{
			char inverse[MAX_VALUE];
			size_t n = strlen(conditions[i]);

			if (n >= 2 &&
				conditions[i][0] == '"' &&
				conditions[i][n - 1] == '"')
				snprintf(inverse, sizeof inverse,
					"\"(%.*s) != 0\"",
					(int)(n - 2),
					conditions[i] + 1);
			else
				snprintf(inverse, sizeof inverse,
					"(%s) != 0",
					conditions[i]);

			emitf("If %s %s\n", else_fn, inverse);
		}

		current_helper = old_helper;
	}

	emitf("If %s 0\n", dispatch_fns[0]);

#undef MAX_IF_BRANCHES
}

static void compile_while
(const char *fn)
{
	char condition[MAX_VALUE];
	char while_fn[MAX_NAME];

	expect_char('(');

	compile_value(fn, condition, sizeof condition);

	expect_char(')');
	expect_char('{');

	snprintf(while_fn, sizeof while_fn, "__RockWhile_%u", next_control_id++);

	Helper *old_helper = current_helper;
	Helper *loop = new_helper(while_fn);
	current_helper = loop;

	skipws();

	while (peekch() != '}')
	{
		if (peekch() < 0) fail("unterminated while block");
		compile_stmt(fn);
		skipws();
	}

	expect_char('}');
	emitf("If %s %s\n", while_fn, condition);

	current_helper = old_helper;
	emitf("If %s %s\n", while_fn, condition);
}

static void compile_stmt
(const char *fn)
{
	if (try_raw_line())
		return;

	if (try_keyword("include"))
	{
		char name[MAX_PATH];

		read_ident(name, sizeof name);
		include_file(name);

		return;
	}

	if (try_keyword("if"))
	{
		compile_if(fn);
		return;
	}

	if (try_keyword("while"))
	{
		compile_while(fn);
		return;
	}

	if (try_keyword("export"))
	{
		char name[MAX_NAME];

		read_ident(name, sizeof name);
		emitf("New %s __Func_%s_%s\n", name, fn, name);
		return;
	}

	if (try_keyword("call"))
	{
		compile_call(fn, NULL);
		return;
	}

	char name[MAX_NAME];

	read_ident(name, sizeof name);
	add_local(name);

	expect_char('=');

	if (try_keyword("call"))
	{
		compile_call(fn, name);
		return;
	}

	char val[MAX_VALUE];
	compile_value(fn, val, sizeof val);

	char mangled[MAX_NAME];
	mangle(fn, name, mangled, sizeof mangled);

	emitf("New %s %s\n", mangled, val);
}

static void compile_fn
(void)
{
	char fn[MAX_NAME];

	read_ident(fn, sizeof fn);

	if (strcmp(fn, "main") == 0) has_main = 1;

	nlocals = 0;

	char params[MAX_LOCALS][MAX_NAME];
	int nparams = 0;

	expect_char('(');

	skipws();

	while (peekch() != ')')
	{
		if (nparams >= MAX_LOCALS) fail("too many parameters");

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

	int first_helper = nhelpers;

	Helper *old_helper = current_helper;
	current_helper = NULL;

	printf("Func %s\n", fn);

	for (int i = 0; i < nparams; i++) printf("New __Func_%s_%s __Func_%s_ARG%d\n", fn, params[i], fn, i);

	skipws();

	while (peekch() != '}')
	{
		if (peekch() < 0) fail("unterminated function");
		compile_stmt(fn);
		skipws();
	}

	expect_char('}');

	printf("New __Func_%s_RET0 __Func_%s_%s\n", fn, fn, result);
	printf("End\n");

	current_helper = old_helper;

	emit_helpers_from(first_helper);
}

static void compile_top_stmt
(void)
{
	if (try_raw_line())
		return;

	if (try_keyword("include"))
	{
		char name[MAX_PATH];

		read_ident(name, sizeof name);
		include_file(name);

		return;
	}

	if (try_keyword("call"))
	{
		compile_call("", NULL);
		return;
	}

	fail("expected 'fn', 'include', or 'call' at top level");
}

int main
(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s source search-path\n", argv[0]);
		return 1;
	}

	if (strlen(argv[2]) >= sizeof include_path) fail("include search path is too long");

	strcpy(include_path, argv[2]);

	FILE *file = fopen(argv[1], "r");

	if (!file)
	{
		fprintf(stderr, "rockc: error: could not open source file '%s'\n", argv[1]);
		return 1;
	}

	size_t input_len = fread(src, 1, sizeof src - 1, file);

	if (ferror(file))
	{
		fclose(file);
		fprintf(stderr, "rockc: error: could not read source file '%s'\n", argv[1]);
		return 1;
	}

	fclose(file);

	len = (long)input_len;
	src[len] = '\0';
	pos = 0;

	skipws();

	while (pos < len)
	{
		if (try_raw_line()) continue;
		if (try_keyword("fn")) compile_fn();
		else compile_top_stmt();

		skipws();
	}

	if (has_main) printf("Call main\n");
	return 0;
}

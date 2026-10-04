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

char include_path[MAX_PATH];

char locals[MAX_LOCALS][MAX_NAME];
int nlocals;

unsigned next_control_id;
int has_main;

Helper helpers[MAX_HELPERS];
int nhelpers;

Helper *current_helper;

void fail
(const char *msg)
{
	fprintf(stderr, "rockc: error: %s (at byte %ld)\n", msg, pos);
	exit(1);
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
		vprintf(fmt, ap);
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
	printf("Func %s\n", h->name);
	fputs(h->body, stdout);

	if (h->body_len == 0 || h->body[h->body_len - 1] != '\n') putchar('\n');
	printf("End\n");
}

void emit_helpers_from
(int first)
{
	for (int i = first; i < nhelpers; i++) emit_helper(&helpers[i]);
}

int try_raw_line
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
}

void mangle
(const char *fn, const char *name, char *out, size_t cap)
{
	if (is_local(name)) snprintf(out, cap, "__Func_%s_%s", fn, name);
	else snprintf(out, cap, "%s", name);
}

void include_file
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

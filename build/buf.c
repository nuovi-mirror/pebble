#include "build.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void bufinit 
(Buf *b) 
{ b->data = NULL; b->len = 0; b->cap = 0; }

void buffree 
(Buf *b) 
{ free(b->data); bufinit(b); }

const char *bufstr 
(const Buf *b) 
{ return b->data != NULL ? b->data : ""; }

static void bufgrow 
(Buf *b, unsigned long need) 
{
	unsigned long cap = b->cap != 0 ? b->cap : 64;
	char *p;

	if (b->len + need + 1 <= b->cap) return;
	while (cap < b->len + need + 1) cap *= 2;

	p = realloc(b->data, cap);
	if (p == NULL) error("out of memory");
	b->data = p;
	b->cap = cap;
}

void bufaddn 
(Buf *b, const char *s, unsigned long n) 
{
	bufgrow(b, n);
	memcpy(b->data + b->len, s, n);
	b->len += n;
	b->data[b->len] = '\0';
}

void bufadd (Buf *b, const char *s) { bufaddn(b, s, strlen(s)); }

void bufshquote 
(Buf *b, const char *s) 
{
	bufaddn(b, "'", 1);
	for (; *s != '\0'; s++) 
	{
		if (*s == '\'') bufadd(b, "'\\''");
		else bufaddn(b, s, 1);
	}

	bufaddn(b, "'", 1);
}

int addflags 
(Buf *dst, const char *s) 
{
	Buf tok;

	for (;;) 
	{
		while (isspace((unsigned char)*s)) s++;
		if (*s == '\0') break;

		bufinit(&tok);
		while (*s != '\0' && !isspace((unsigned char)*s)) 
		{
			if (*s == '\'') 
			{
				for (s++; *s != '\0' && *s != '\''; s++) bufaddn(&tok, s, 1);
				if (*s == '\0') { buffree(&tok); return 0; }
				s++;
			} 
			else if (*s == '"') 
			{
				for (s++; *s != '\0' && *s != '"'; s++) 
				{
					if (*s == '\\' && (s[1] == '"' || s[1] == '\\')) s++;
					bufaddn(&tok, s, 1);
				}
				if (*s == '\0') { buffree(&tok); return 0; }
				s++;
			} 
			else if (*s == '\\' && s[1] != '\0') 
			{
				s++;
				bufaddn(&tok, s++, 1);
			} 
			else 
			{
				bufaddn(&tok, s++, 1);
			}
		}

		if (dst->len != 0) bufadd(dst, " ");
		bufshquote(dst, bufstr(&tok));
		buffree(&tok);

	}
	return 1;
}

/* dead code elimination code */

#include <stdlib.h>
#include <string.h>
#include "stuff.h"

typedef struct DceRef
{
	const char   *name;
	unsigned long slen;
} DceRef;

typedef struct DceBlock
{
	const char   *name;
	unsigned long slen;
	unsigned long start;     /* first byte of the Func line */
	unsigned long end;       /* one past the newline of the End line */
	unsigned long ref_first; /* this block's refs, a slice of the shared array */
	unsigned long nrefs;
	int           dynamic;   /* can call a function whose name is only known at run time */
	int           live;
} DceBlock;

typedef struct DceList
{
	void         *items;
	unsigned long count;
	unsigned long cap;
	unsigned long size; /* bytes per item */
} DceList;

static int is_ws
(char c)
{ return c == ' ' || c == '\t' || c == '\r'; }

static int is_name_char
(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9') || c == '_' || c == '.';
}

/* returns a pointer to the new slot, or NULL if out of memory */
static void *list_add
(DceList *l)
{
	if (l->count == l->cap)
	{
		unsigned long ncap = l->cap ? l->cap * 2 : 32;
		void *p = realloc(l->items, ncap * l->size);

		if (!p) return NULL;

		l->items = p;
		l->cap = ncap;
	}

	return (char *)l->items + (l->count++ * l->size);
}

static int is_op
(const char *p, unsigned long n, const char *op)
{ return strlen(op) == n && memcmp(p, op, n) == 0; }

/* look at the first operand of an instruction (q is just past the opcode).
 * returns 1 and fills name/slen if it is a plain function name, 0 for
 * anything else: computed, missing, or not shaped like a name */
static int first_operand
(const char *q, const char *e, const char **name, unsigned long *slen)
{
	const char *s;
	const char *t;

	while (q < e && is_ws(*q)) q++;
	if (q >= e) return 0;

	if (*q == '\'')
	{
		s = ++q;
		while (q < e && *q != '\'') q++;
		if (q >= e) return 0; /* never closed */
		t = q;
	}
	else if (*q == '"' || *q == '{' || *q == '<' || *q == '[') return 0;
	else
	{
		s = q;
		while (q < e && !is_ws(*q)) q++;
		t = q;
	}

	if (t == s) return 0;

	for (const char *c = s; c < t; c++)
		if (!is_name_char(*c)) return 0;

	*name = s;
	*slen = (unsigned long)(t - s);

	return 1;
}

/* mark every block called 'name' as live and queue it for scanning */
static void mark
(DceBlock *blocks, unsigned long nblocks, const char *name, unsigned long slen,
 unsigned long *stack, unsigned long *sp)
{
	for (unsigned long k = 0; k < nblocks; k++)
	{
		if (blocks[k].live || blocks[k].slen != slen) continue;
		if (memcmp(blocks[k].name, name, slen) != 0) continue;

		blocks[k].live = 1;
		stack[(*sp)++] = k;
	}
}

/* buf holds slen bytes of Pebble text. Dead functions are cut out of it in
 * place and the new length is returned. *removed is how many were cut. */
unsigned long dce_functions
(char *buf, unsigned long slen, unsigned long *removed)
{
	DceList blocks = { NULL, 0, 0, sizeof(DceBlock) };
	DceList refs   = { NULL, 0, 0, sizeof(DceRef) };  /* refs made inside functions */
	DceList trefs  = { NULL, 0, 0, sizeof(DceRef) };  /* refs made outside any function */
	unsigned long *stack = NULL;
	unsigned long result = slen;
	unsigned long count = 0;
	int top_dynamic = 0;
	int bail = 0;
	long cur = -1;

	if (removed) *removed = 0;

	/* pass one: find the blocks and what each one refers to */
	unsigned long i = 0;

	while (i < slen && !bail)
	{
		unsigned long ls = i;
		const char *nl = memchr(buf + i, '\n', slen - i);
		unsigned long le = nl ? (unsigned long)(nl - buf) : slen;
		unsigned long next = nl ? le + 1 : slen;
		const char *p = buf + ls;
		const char *e = buf + le;

		i = next;

		while (p < e && is_ws(*p)) p++;
		if (p >= e || *p == '#' || *p == '/') continue; /* blank or a comment */

		const char *q = p;
		while (q < e && !is_ws(*q)) q++;

		unsigned long oplen = (unsigned long)(q - p);
		const char *name = NULL;
		unsigned long nlen = 0;

		if (is_op(p, oplen, "Func"))
		{
			if (cur >= 0 || !first_operand(q, e, &name, &nlen)) { bail = 1; break; }

			DceBlock *b = list_add(&blocks);
			if (!b) { bail = 1; break; }

			b->name = name;
			b->slen = nlen;
			b->start = ls;
			b->end = 0;
			b->ref_first = refs.count;
			b->nrefs = 0;
			b->dynamic = 0;
			b->live = 0;

			cur = (long)blocks.count - 1;
		}
		else if (is_op(p, oplen, "End"))
		{
			if (cur >= 0)
			{
				DceBlock *b = (DceBlock *)blocks.items + cur;

				b->end = next;
				b->nrefs = refs.count - b->ref_first;
				cur = -1;
			}
		}
		else if (is_op(p, oplen, "Call") || is_op(p, oplen, "If") || is_op(p, oplen, "Spawn"))
		{
			if (first_operand(q, e, &name, &nlen))
			{
				DceRef *r = list_add(cur >= 0 ? &refs : &trefs);
				if (!r) { bail = 1; break; }

				r->name = name;
				r->slen = nlen;
			}
			else if (cur >= 0) ((DceBlock *)blocks.items)[cur].dynamic = 1;
			else top_dynamic = 1;
		}
	}

	if (cur >= 0) bail = 1;          /* a Func that never ends */
	if (top_dynamic) bail = 1;       /* load-time code that calls something unknown */
	if (bail || blocks.count == 0) goto done;

	/* pass two: everything reachable from the load-time code */
	DceBlock *b = blocks.items;
	unsigned long sp = 0;

	stack = malloc(blocks.count * sizeof(*stack));
	if (!stack) goto done;

	for (unsigned long k = 0; k < trefs.count; k++)
	{
		DceRef *r = (DceRef *)trefs.items + k;
		mark(b, blocks.count, r->name, r->slen, stack, &sp);
	}

	while (sp > 0)
	{
		DceBlock *cb = &b[stack[--sp]];

		if (cb->dynamic) { bail = 1; break; }

		for (unsigned long k = 0; k < cb->nrefs; k++)
		{
			DceRef *r = (DceRef *)refs.items + cb->ref_first + k;
			mark(b, blocks.count, r->name, r->slen, stack, &sp);
		}
	}

	if (bail) goto done;

	/* pass three: cut out whatever never went live. blocks are in file order */
	unsigned long w = 0;
	unsigned long r = 0;

	for (unsigned long k = 0; k < blocks.count; k++)
	{
		if (b[k].live) continue;

		memmove(buf + w, buf + r, b[k].start - r);
		w += b[k].start - r;
		r = b[k].end;
		count++;
	}

	if (count == 0) goto done;

	memmove(buf + w, buf + r, slen - r);
	result = w + (slen - r);

	if (removed) *removed = count;

done:
	free(stack);
	free(blocks.items);
	free(refs.items);
	free(trefs.items);

	return result;
}

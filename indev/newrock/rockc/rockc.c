/* simple New Rock demo compiler
 * uses Pebble AAE schematics instead of older Rock marker symbols
 * reads program from a file and writes compiled Pebble code to stdout */

#include <stdio.h>
#include <string.h>
#include "stuff.h"

void compile_stmt(const char *fn);
	
void compile_value
(const char *fn, char *out, unsigned long cap)
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

	if (c == '{')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('}', text, sizeof text);
		subst_expr(fn, text, subst, sizeof subst);

		snprintf(out, cap, "{%s}", subst);

		return;
	}
	
	if (c == '<')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('>', text, sizeof text);
		subst_expr(fn, text, subst, sizeof subst);

		snprintf(out, cap, "<%s>", subst);

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

void compile_call
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


void compile_if_branch
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


void compile_if
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
			unsigned long n = strlen(conditions[i]);

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
			unsigned long n = strlen(conditions[i]);

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

void compile_while
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

void compile_stmt
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

void compile_fn
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

void compile_top_stmt
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

	unsigned long input_len = fread(src, 1, sizeof src - 1, file);

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

	printf("#!/usr/bin/pblvm\n");

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

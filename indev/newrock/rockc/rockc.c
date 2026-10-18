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
		use_text_var(text, pos);
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
		use_text_var(text, pos);
		subst_expr(fn, text, subst, sizeof subst);

		snprintf(out, cap, "<%s>", subst);

		return;
	}

	if (is_ident_start(c))
	{
		char name[MAX_NAME];
		long at = pos;

		read_ident(name, sizeof name);
		use_var(name, at);
		mangle(fn, name, out, cap);

		return;
	}

	fail("expected a value ('literal', \"expression\", or a bare name)");
}

void compile_call
(const char *fn, char dests[][MAX_NAME], int ndests)
{
	char callee[MAX_NAME];

	skipws();

	long at = pos;

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

	note_call(callee, argc, ndests, at);

	emitf("Call %s\n", callee);

	/* results are bound in order: first name gets RET0, second RET1, ... */
	for (int i = 0; i < ndests; i++)
	{
		char mangled[MAX_NAME];

		mangle(fn, dests[i], mangled, sizeof mangled);
		emitf("New %s __Func_%s_RET%d\n", mangled, callee, i);
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

	if (try_keyword("import"))
	{
		char name[MAX_NAME];

		read_ident(name, sizeof name);

		if (is_local(name))
		{
			char msg[MAX_NAME + 64];

			snprintf(msg, sizeof msg, "cannot import '%s': already a variable of this function", name);
			fail(msg);
		}

		add_local(name);
		mark_assigned(name);
		emitf("New __Func_%s_%s %s\n", fn, name, name);
		return;
	}

	if (try_keyword("export"))
	{
		char name[MAX_NAME];

		read_ident(name, sizeof name);

		if (!is_local(name))
		{
			char msg[MAX_NAME + 64];

			snprintf(msg, sizeof msg, "cannot export '%s': not a variable of this function", name);
			fail(msg);
		}

		use_var(name, pos);
		define_global(name);
		emitf("New %s __Func_%s_%s\n", name, fn, name);
		return;
	}



	if (try_keyword("call"))
	{
		compile_call(fn, NULL, 0);
		return;
	}

	/* one or more space-separated destination names, then '=' */
	char names[MAX_LOCALS][MAX_NAME];
	int nnames = 0;

	for (;;)
	{
		if (nnames >= MAX_LOCALS) fail("too many assignment targets");

		skipws();

		if (!is_ident_start(peekch()))
			fail(nnames == 0 ? "expected a statement"
				: "expected '=' after assignment target(s) (unknown statement?)");

		read_ident(names[nnames], sizeof names[nnames]);
		add_local(names[nnames]);
		nnames++;

		skipws();

		if (peekch() == '=') break;
	}

	expect_char('=');

	if (try_keyword("call"))
	{
		compile_call(fn, names, nnames);

		for (int i = 0; i < nnames; i++) mark_assigned(names[i]);

		return;
	}

	char val[MAX_VALUE];
	compile_value(fn, val, sizeof val);

	/* same value is assigned to every target */
	for (int i = 0; i < nnames; i++)
	{
		char mangled[MAX_NAME];

		mangle(fn, names[i], mangled, sizeof mangled);
		emitf("New %s %s\n", mangled, val);
	}

	for (int i = 0; i < nnames; i++) mark_assigned(names[i]);
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

		if (is_local(params[nparams])) fail("duplicate parameter name");

		add_local(params[nparams]);
		mark_assigned(params[nparams]);

		nparams++;
		skipws();
	}

	expect_char(')');

	char results[MAX_LOCALS][MAX_NAME];
	int nresults = 0;

	/* one or more result names; they map to RET0, RET1, ... in order */
	for (;;)
	{
		if (nresults >= MAX_LOCALS) fail("too many results");

		read_ident(results[nresults], sizeof results[nresults]);

		for (int i = 0; i < nresults; i++)
			if (strcmp(results[i], results[nresults]) == 0)
				fail("duplicate result name");

		add_local(results[nresults]);
		nresults++;

		skipws();

		if (peekch() == '{') break;
	}

	expect_char('{');

	Loc fn_loc = loc_at(pos);

	define_func(fn, nparams, nresults);
	snprintf(cur_fn, sizeof cur_fn, "%s", fn);

	int first_helper = nhelpers;

	Helper *old_helper = current_helper;
	current_helper = NULL;

	outf("Func %s\n", fn);

	for (int i = 0; i < nparams; i++) outf("New __Func_%s_%s __Func_%s_ARG%d\n", fn, params[i], fn, i);

	skipws();

	while (peekch() != '}')
	{
		if (peekch() < 0) fail("unterminated function");
		compile_stmt(fn);
		skipws();
	}

	expect_char('}');

	for (int i = 0; i < nresults; i++)
		if (!is_assigned(results[i]))
		{
			char msg[3 * MAX_NAME + 64];

			snprintf(msg, sizeof msg, "result '%s' of function '%s' is never assigned", results[i], fn);
			report_warning(msg, fn_loc);
		}

	for (int i = 0; i < nresults; i++)
		outf("New __Func_%s_RET%d __Func_%s_%s\n", fn, i, fn, results[i]);

	outf("End\n");

	current_helper = old_helper;

	emit_helpers_from(first_helper);

	/* locals belong to the function; top-level code has none */
	nlocals = 0;
	cur_fn[0] = '\0';
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
		compile_call("", NULL, 0);
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

	Loc no_parent = { -1, 0 };
	int main_file = add_source_unit(argv[1], no_parent);

	tag_source(0, len, main_file);
	src_file[len] = (unsigned short)main_file;
	src_line[len] = len > 0 ? src_line[len - 1] : 1;

	skipws();

	outf("#!/usr/bin/env pblvm\n");

	while (pos < len)
	{
		if (try_raw_line()) continue;
		if (try_keyword("fn")) compile_fn();
		else compile_top_stmt();

		skipws();
	}

	if (finish_checks() > 0)
	{
		fprintf(stderr, "rockc: %d error(s); no output written\n", nerrors);
		return 1;
	}

	if (has_main) outf("Call main\n");

	if (flush_output() != 0)
	{
		fprintf(stderr, "rockc: error: could not write output\n");
		return 1;
	}

	return 0;
}

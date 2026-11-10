#include <stdio.h>
#include <string.h>
#include "stuff.h"

static char cur_results[MAX_LOCALS][MAX_NAME];
static int cur_nresults = 0;

static void emit_ret_fill
(const char *fn)
{
	for (int i = 0; i < cur_nresults; i++)
		emitf("New __Func_%s_RET%d __Func_%s_%s\n", fn, i, fn, cur_results[i]);
}

#define CTL_RETURN    "1"
#define CTL_BREAK     "2"
#define CTL_CONTINUE  "3"

static unsigned ev_return, ev_break, ev_continue; /* escaping events seen so far */
static int loop_depth;                            /* while loops around the statement */
static int list_nested;                           /* 0: function level, 1: inside an if/else/while body */
static int fn_uses_ctl;                           /* the control variable is in use in this function */

static void ctl_name
(const char *fn, char *out, unsigned long cap)
{
	snprintf(out, cap, "__Func_%s___RockCtl", fn);
}

void compile_stmt
(const char *fn);
	
void compile_value
(const char *fn, char *out, unsigned long cap)
{
	skipws();

	int c = peekch();

	if (c == '\'')
	{
		pos++;

		char text[MAX_VALUE];

		read_delimited('\'', text, sizeof(text));

		snprintf(out, cap, "'%s'", text);
		return;
	}

	if (c == '"')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('"', text, sizeof(text));
		subst_expr(fn, text, subst, sizeof(subst));

		snprintf(out, cap, "\"%s\"", subst);

		return;
	}

	if (c == '{')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('}', text, sizeof(text));
		use_text_var(text, pos);
		subst_expr(fn, text, subst, sizeof(subst));

		snprintf(out, cap, "{%s}", subst);

		return;
	}

	if (c == '[')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited(']', text, sizeof(text));
		use_text_var(text, pos);
		subst_expr(fn, text, subst, sizeof(subst));

		snprintf(out, cap, "[%s]", subst);

		return;
	}

	
	if (c == '<')
	{
		pos++;

		char text[MAX_VALUE];
		char subst[MAX_VALUE];

		read_delimited('>', text, sizeof(text));
		use_text_var(text, pos);
		subst_expr(fn, text, subst, sizeof(subst));

		snprintf(out, cap, "<%s>", subst);

		return;
	}

	if (is_ident_start(c))
	{
		char name[MAX_NAME];
		long at = pos;

		read_ident(name, sizeof(name));
		use_var(name, at);
		mangle(fn, name, out, cap);

		return;
	}

	fail("expected a value ('literal', \"expression\", <pointer>, [indirect], {forced evaluation}, or a bare name)");
}

void compile_call
(const char *fn, char dests[][MAX_NAME], int ndests)
{
	char callee[MAX_NAME];
	skipws();
	long at = pos;
	read_ident(callee, sizeof(callee));
	expect_char('(');
	skipws();
	int argc = 0;

	while (peekch() != ')')
	{
		char val[MAX_VALUE];

		compile_value(fn, val, sizeof(val));
		emitf("New __Func_%s_ARG%d %s\n", callee, argc, val);

		argc++;
		skipws();
	}

	expect_char(')');
	note_call(callee, argc, ndests, at);
	emitf("Call %s\n", callee);

	for (int i = 0; i < ndests; i++)
	{
		char mangled[MAX_NAME];

		mangle(fn, dests[i], mangled, sizeof(mangled));
		emitf("New %s __Func_%s_RET%d\n", mangled, callee, i);
	}
}


static void compile_block
(const char *fn, int nested, const char *unterminated)
{
	int saved_nested = list_nested;
	list_nested = nested;
	skipws();

	while (peekch() != '}')
	{
		if (peekch() < 0)
			fail(unterminated);

		unsigned before = ev_return + ev_break + ev_continue;

		compile_stmt(fn);
		skipws();

		if (ev_return + ev_break + ev_continue != before && peekch() != '}')
		{
			char ctl[MAX_NAME], cont[MAX_NAME];

			ctl_name(fn, ctl, sizeof(ctl));
			snprintf(cont, sizeof(cont), "__RockCont_%u", next_control_id++);

			emitf("If %s \"%s == 0\"\n", cont, ctl);
			current_helper = new_helper(cont);
		}
	}

	list_nested = saved_nested;
}

void compile_if_branch
(const char *fn, Helper *helper)
{
	Helper *old_helper = current_helper;
	current_helper = helper;

	compile_block(fn, 1, "unterminated if block");

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
		if (nbranches >= MAX_IF_BRANCHES) fail("too many if branches");
		expect_char('(');
		compile_value(fn, conditions[nbranches], sizeof(conditions[nbranches]));

		expect_char(')');
		expect_char('{');

		snprintf(body_fns[nbranches], sizeof(body_fns[nbranches]), "__RockIf_%u", next_control_id++);
		Helper *branch = new_helper(body_fns[nbranches]);

		compile_if_branch(fn, branch);

		nbranches++;
		skipws();

		if (!try_keyword("else")) break;
		skipws();

		if (try_keyword("if")) continue;
		expect_char('{');

		snprintf(else_fn, sizeof(else_fn), "__RockElse_%u", next_control_id++);
		Helper *else_body = new_helper(else_fn);
		compile_if_branch(fn, else_body);

		has_else = 1;
		break;
	}

	char dispatch_fns[MAX_IF_BRANCHES][MAX_NAME];
	for (int i = 0; i < nbranches; i++) snprintf(dispatch_fns[i], sizeof(dispatch_fns[i]), "__RockDispatch_%u", next_control_id++);

	for (int i = 0; i < nbranches; i++)
	{
		Helper *old_helper = current_helper;
		Helper *dispatch = new_helper(dispatch_fns[i]);

		current_helper = dispatch;

		if (i + 1 < nbranches || has_else)
		{
			char temp[MAX_NAME];

			snprintf(temp, sizeof(temp), "__Func_%s___RockCond_%u", fn, next_control_id++);

			emitf("New %s %s\n", temp, conditions[i]);
			emitf("If %s %s\n", body_fns[i], temp);
			emitf("If %s \"%s != 0\"\n", i + 1 < nbranches ? dispatch_fns[i + 1] : else_fn, temp);
		}
		else
			emitf("If %s %s\n", body_fns[i], conditions[i]);

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
	compile_value(fn, condition, sizeof(condition));

	expect_char(')');
	expect_char('{');

	unsigned id = next_control_id++;
	snprintf(while_fn, sizeof(while_fn), "__RockWhile_%u", id);

	Helper *old_helper = current_helper;
	Helper *loop = new_helper(while_fn);
	current_helper = loop;

	unsigned ret0 = ev_return, brk0 = ev_break, cnt0 = ev_continue;

	loop_depth++;
	compile_block(fn, 1, "unterminated while block");
	loop_depth--;

	expect_char('}');

	int has_loopctl = ev_break != brk0 || ev_continue != cnt0;
	int exits = ev_return != ret0 || has_loopctl;
	char entry[MAX_NAME];

	snprintf(entry, sizeof(entry), "%s", while_fn);

	if (!exits)
	{
		emitf("If %s %s\n", while_fn, condition);
	}
	else
	{
		char ctl[MAX_NAME], temp[MAX_NAME];
		Helper *driver;

		ctl_name(fn, ctl, sizeof(ctl));
		snprintf(entry, sizeof(entry), "__RockLoop_%u", id);
		snprintf(temp, sizeof(temp), "__Func_%s___RockCond_%u", fn, next_control_id++);

		driver = new_helper(entry);
		current_helper = driver;

		emitf("New %s '0'\n", ctl);                 /* a 'continue' ends here */
		emitf("Call %s\n", while_fn);
		emitf("New %s %s\n", temp, condition);
		/* go round again if (ctl is 0 or 3) and the condition holds */
		emitf("If %s \"(((%s == 0) * (%s == 3)) == 0) + (%s == 0) == 0\"\n", entry, ctl, ctl, temp);

		fn_uses_ctl = 1;
	}

	current_helper = old_helper;
	emitf("If %s %s\n", entry, condition);

	if (has_loopctl)
	{
		char ctl[MAX_NAME], clr[MAX_NAME];

		ctl_name(fn, ctl, sizeof(ctl));
		snprintf(clr, sizeof(clr), "__RockClr_%u", id);

		Helper *c = new_helper(clr);
		emitf("If %s \"%s > 1\"\n", clr, ctl);
		snprintf(c->body, HELPER_BUFSZ, "New %s '0'\n", ctl);
		c->body_len = strlen(c->body);
	}

	/* break and continue are used up by this loop; only a return gets past it */
	ev_break = brk0;
	ev_continue = cnt0;
}

void compile_stmt
(const char *fn)
{
	if (try_raw_line()) return;
	if (try_keyword("include"))
	{
		char name[MAX_PATH];

		read_ident(name, sizeof(name));
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

		read_ident(name, sizeof(name));

		if (is_local(name))
		{
			char msg[MAX_NAME + 64];

			snprintf(msg, sizeof(msg), "cannot import '%s': already a variable of this function", name);
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

		read_ident(name, sizeof(name));

		if (!is_local(name))
		{
			char msg[MAX_NAME + 64];

			snprintf(msg, sizeof(msg), "cannot export '%s': not a variable of this function", name);
			fail(msg);
		}

		use_var(name, pos);
		define_global(name);
		emitf("New %s __Func_%s_%s\n", name, fn, name);
		return;
	}



	if (try_keyword("return"))
	{
		emit_ret_fill(fn);

		if (list_nested)
		{
			char ctl[MAX_NAME];

			ctl_name(fn, ctl, sizeof(ctl));
			emitf("New %s '" CTL_RETURN "'\n", ctl);
			ev_return++;
			fn_uses_ctl = 1;
		}

		emitf("Return\n");
		return;
	}

	int is_break = try_keyword("break");

	if (is_break || try_keyword("continue"))
	{
		char ctl[MAX_NAME];

		if (loop_depth == 0)
			fail(is_break ? "'break' is only allowed inside a while loop" : "'continue' is only allowed inside a while loop");

		ctl_name(fn, ctl, sizeof(ctl));

		if (is_break)
		{
			emitf("New %s '" CTL_BREAK "'\n", ctl);
			ev_break++;
		}
		else
		{
			emitf("New %s '" CTL_CONTINUE "'\n", ctl);
			ev_continue++;
		}

		fn_uses_ctl = 1;
		emitf("Return\n");
		return;
	}

	if (try_keyword("call"))
	{
		compile_call(fn, NULL, 0);
		return;
	}

	char names[MAX_LOCALS][MAX_NAME];
	int nnames = 0;

	for (;;)
	{
		if (nnames >= MAX_LOCALS) fail("too many assignment targets");

		skipws();

		if (!is_ident_start(peekch())) fail(nnames == 0 ? "expected a statement" : "expected '=' after assignment target(s) (unknown statement?)");

		read_ident(names[nnames], sizeof(names[nnames]));
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
	compile_value(fn, val, sizeof(val));

	for (int i = 0; i < nnames; i++)
	{
		char mangled[MAX_NAME];

		mangle(fn, names[i], mangled, sizeof(mangled));
		emitf("New %s %s\n", mangled, val);
	}

	for (int i = 0; i < nnames; i++) mark_assigned(names[i]);
}

void compile_fn
(void)
{
	char fn[MAX_NAME];
	read_ident(fn, sizeof(fn));
	if (strcmp(fn, "main") == 0) has_main = 1;
	nlocals = 0;

	char params[MAX_LOCALS][MAX_NAME];
	int nparams = 0;

	expect_char('(');
	skipws();

	while (peekch() != ')')
	{
		if (nparams >= MAX_LOCALS) fail("too many parameters");
		read_ident(params[nparams], sizeof(params[nparams]));
		if (is_local(params[nparams])) fail("duplicate parameter name");
		add_local(params[nparams]);
		mark_assigned(params[nparams]);

		nparams++;
		skipws();
	}

	expect_char(')');

	char results[MAX_LOCALS][MAX_NAME];
	int nresults = 0;

	for (;;)
	{
		if (nresults >= MAX_LOCALS) fail("too many results");
		read_ident(results[nresults], sizeof(results[nresults]));

		for (int i = 0; i < nresults; i++) if (strcmp(results[i], results[nresults]) == 0) fail("duplicate result name");
		add_local(results[nresults]);
		nresults++;
		skipws();

		if (peekch() == '{') break;
	}

	cur_nresults = nresults;
	for (int i = 0; i < nresults; i++)
		snprintf(cur_results[i], sizeof(cur_results[i]), "%s", results[i]);

	expect_char('{');

	Loc fn_loc = loc_at(pos);

	define_func(fn, nparams, nresults);
	snprintf(cur_fn, sizeof(cur_fn), "%s", fn);

	int first_helper = nhelpers;

	Helper *old_helper = current_helper;
	current_helper = NULL;
	loop_depth = 0;
	fn_uses_ctl = 0;

	outf("Func %s\n", fn);
	for (int i = 0; i < nparams; i++) outf("New __Func_%s_%s __Func_%s_ARG%d\n", fn, params[i], fn, i);
	skipws();

	compile_block(fn, 0, "unterminated function");

	expect_char('}');

	for (int i = 0; i < nresults; i++)
		if (!is_assigned(results[i]))
		{
			char msg[3 * MAX_NAME + 64];

			snprintf(msg, sizeof(msg), "result '%s' of function '%s' is never assigned", results[i], fn);
			report_warning(msg, fn_loc);
		}

	emit_ret_fill(fn);   /* lands in the last continuation, if there is one */

	current_helper = NULL;

	if (fn_uses_ctl)
	{
		char ctl[MAX_NAME];

		ctl_name(fn, ctl, sizeof(ctl));
		outf("New %s '0'\n", ctl);   /* every exit path ends up here: leave it clear */
	}

	outf("End\n");
	current_helper = old_helper;
	emit_helpers_from(first_helper);

	if (fn_uses_ctl)
	{
		char ctl[MAX_NAME];

		ctl_name(fn, ctl, sizeof(ctl));
		outf("New %s '0'\n", ctl);   /* defined before anything can read it */
	}

	nlocals = 0;
	cur_nresults = 0;
	cur_fn[0] = '\0';
}

void compile_top_stmt
(void)
{
	if (try_raw_line()) return;

	if (try_keyword("include"))
	{
		char name[MAX_PATH];

		read_ident(name, sizeof(name));
		include_file(name);

		return;
	}

	if (try_keyword("call"))
	{
		compile_call("", NULL, 0);
		return;
	}

	if (is_ident_start(peekch()))
	{
		char names[MAX_LOCALS][MAX_NAME];
		int nnames = 0;

		for (;;)
		{
			if (nnames >= MAX_LOCALS) fail("too many assignment targets");
			skipws();

			if (!is_ident_start(peekch())) fail(nnames == 0 ? "expected a statement" : "expected '=' after assignment target(s) (unknown statement?)");

			read_ident(names[nnames], sizeof(names[nnames]));
			define_global(names[nnames]);
			nnames++;

			skipws();

			if (peekch() == '=') break;
		}

		expect_char('=');

		if (try_keyword("call"))
		{
			compile_call("", names, nnames);
			return;
		}

		char val[MAX_VALUE];
		compile_value("", val, sizeof(val));

		for (int i = 0; i < nnames; i++) emitf("New %s %s\n", names[i], val);

		return;
	}

	fail("expected 'fn', 'include', 'call' or a definition at top level");
}

int main
(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s source search-path\n", argv[0]);
		return 1;
	}

	if (strlen(argv[2]) >= sizeof(include_path)) fail("include search path is too long");
	strcpy(include_path, argv[2]);

	FILE *file = fopen(argv[1], "r");
	if (!file)
	{
		fprintf(stderr, "rockc: error: could not open source file '%s'\n", argv[1]);
		return 1;
	}

	unsigned long input_len = fread(src, 1, sizeof(src) - 1, file);

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
	int main_file = add_source_unit(argv[1], no_parent, 0 /* the user's own file: not trusted */);

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

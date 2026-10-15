const char stdlib[] = {
	". # New Rock Standard Library (stdlib)\n"

	/* io */
	"fn std.io.print ( value ) data {\n"
	". New __Func_std.io.print_data {__Func_std.io.print_value}\n"
	". New __Escape_std.io.print_ARG0 <__Func_std.io.print_data>\n"
	". Escape std.io.print\n"
	"}\n"

	"fn std.io.printRaw ( value ) value {\n"
	". New __Escape_std.io.print_ARG0 <__Func_std.io.printRaw_value>\n"
	". Escape std.io.print\n"
	"}\n"

	"fn std.io.printLn ( value ) data {\n"
	". New __Func_std.io.printLn_data {__Func_std.io.printLn_value}\n"
	". New __Escape_std.io.printLn_ARG0 <__Func_std.io.printLn_data>\n"
	". Escape std.io.printLn\n"
	"}\n"
	
	"fn std.io.input ( ) input {\n"
	". Escape std.io.input\n"
	". New __Func_std.io.input_input __Escape_std.io.input_RET0\n"
	"}\n"

	/* misc */
	"fn std.misc.random ( ) random {\n"
	". Escape std.misc.random\n"
	". New __Func_std.misc.random_random __Escape_std.misc.random_RET0\n"
	"}\n"

	"fn std.misc.sleep ( time ) slept {\n"
	". New __Escape_std.misc.sleep_ARG0 <__Func_std.misc.sleep_time>\n"
	". Escape std.misc.sleep\n"
	". New __Func_std.misc.sleep_slept __Escape_std.misc.sleep_RET0\n"
	"}"

	/* proc */
	"fn std.proc.spawn ( args ) ret {\n"
	". New __Escape_std.proc.spawn_ARG0 <__Func_std.proc.spawn_args>\n"
	". Escape std.proc.spawn\n"
	". New __Func_std.proc.spawn_ret __Escape_std.proc.spawn_RET0\n"
	"}\n"

	/* term */
	"fn std.term.screen.clear ( ) ret {\n"
	". Escape std.term.screen.clear\n"
	". New __Func_std.term.screen.clear_ret '0'\n"
	"}\n"

	"fn std.term.color.basic.fore ( color ) ret {\n"
	". New __Escape_std.term.color.basic.fore_ARG0 <__Func_std.term.color.basic.fore_color>\n"
	". Escape std.term.color.basic.fore\n"
	"ret = color\n"
	"}\n"

	"fn std.term.color.basic.back ( color ) ret {\n"
	". New __Escape_std.term.color.basic.back_ARG0 <__Func_std.term.color.basic.back_color>\n"
	". Escape std.term.color.basic.back\n"
	"ret = color\n"
	"}\n"

	". # end of New Rock Standard Library (stdlib)\n" 
};

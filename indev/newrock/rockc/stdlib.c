const char stdlib[] = {
	". # rock stdlib\n"
	/* io */

	"fn std.io.print ( value ) data {\n"
	". New __Func_std.io.print_data {__Func_std.io.print_value}\n"
	". New __Escape_std.io.print_ARG0 <__Func_std.io.print_data>\n"
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
	". # end rock stdlib\n" 
};

enum Statements {
	statements_statement_assiagn,
};

struct Statement {
	enum Statements type;
	enum As {
		statement_assiagn,
	}as;
};

struct statement_assign {
	char *variable; /* the variable to assiagn */
	char *expression; /* the expression being assiagned to the variable */
	char *funcname; /* name of function we are in - NULL if we are not in one */
};

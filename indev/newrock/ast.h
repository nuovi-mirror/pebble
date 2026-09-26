enum Statements {
	statements_statement_assiagn,
};

struct Statement {
	enum Statements type;
	enum As {
		statement_assiagn,
	}as;
};

struct statement_assiagn {
	char *variable;
	char *expression;
};

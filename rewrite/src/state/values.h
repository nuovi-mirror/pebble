typedef enum {
	type_word,
	type_sword,
	type_float,
	type_string,
} ValueTypes;

typedef struct Value {
	ValueTypes type;
	union {
		unsigned long word;
		long sword;
		double flt;
		char *str;
	}as;
} Value;

void printValue(Value v) {
	switch(v.type) {
		case type_word:
			printf("word of %lu\n", v.as.word);
			break;
		case type_sword:
			printf("sword of %ld\n", v.as.sword);
			break;
		case type_float:
			printf("float of %f\n", v.as.flt);
			break;
		case type_string:
			printf("string of \"%s\"\n", v.as.str);
			break;
	}
}

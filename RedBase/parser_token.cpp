#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser_common.h"
#include "parser_token.h"
#define MAXCHAR 5000

static char charpool[MAXCHAR];
static int charptr = 0;

/*
* string_alloc: returns a pointer to a string of length len if possible
*/
static char *string_alloc(int len)
{
	char *s;
	if (charptr + len > MAXCHAR) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	s = charpool + charptr;
	charptr += len;

	return s;
}

/*
* reset_charptr: releases all memory allocated in preparation for the
* next query.
*
* No return value.
*/
void reset_charptr(void)
{
	charptr = 0;
}

/*
* reset_scanner: resets the scanner after a syntax error
*
* No return value.
*/
void reset_scanner(void)
{
	charptr = 0;
}

/*
* mk_string: allocates space for a string of length len and copies s into
* it.
*
* Returns:
* 	a pointer to the new string
*/
static char *mk_string(const char *s, int len)
{
	char *copy;

	/* allocate space for new string */
	if ((copy = string_alloc(len + 1)) == NULL) {
		printf("out of string space\n");
		exit(1);
	}

	/* copy the string */
	strncpy(copy, s, len + 1);
	return copy;
}

string tokenName(TOKENKIND type)
{
	switch (type)
	{
	case RW_CREATE: return "create";
	case RW_DROP: return "drop";
	case RW_TABLE: return "table";
	case RW_INDEX: return "index";
	case RW_LOAD: return "load";
	case RW_HELP: return "help";
	case RW_EXIT: return "exit";
	case RW_PRINT: return "print";
	case RW_SET: return "set";
	case RW_AND: return "and";
	case RW_INTO: return "into";
	case RW_VALUES: return "values";
	case RW_SELECT: return "select";
	case RW_FROM: return "from";
	case RW_WHERE: return "where";
	case RW_ORDER: return "order";
	case RW_GROUP: return "group";
	case RW_BY: return "by";
	case RW_DESC: return "desc";
	case RW_ASC: return "asc";
	case RW_INSERT: return "insert";
	case RW_DELETE: return "delete";
	case RW_UPDATE: return "update";
	case RW_MAX: return "max";
	case RW_MIN: return "min";
	case RW_AVG: return "avg";
	case RW_SUM: return "sum";
	case RW_COUNT: return "count";
	case RW_RESET: return "reset";
	case RW_BUFFER: return "buffer";
	case RW_ON: return "on";
	case RW_OFF: return "off";
	case T_EOF: return "eof";

	case RW_COMMENT: return "comment";
	case T_NE: return "not_equal";
	case T_INT: return "int";
	case T_FLOAT: return "float";
	case T_STRING: return "string";
	case RW_LPAREN: return "lparen";
	case RW_RPAREN: return "rparen";
	case RW_COMMA: return "comma";
	case RW_SEMICOLON: return "semicolon";
	}
	return "";
}


//
// printToken 用于输出Token的值
//
ostream& operator<<(ostream& os, const Token& tk)
{
	os << tk.content << "[" << tokenName(tk.type) << "]" << "(" << tk.line << ")" << endl;
	return os;
}

Token::Token(TOKENKIND type, size_t line, const string& str)
	: type(type), line(line)
{
	if (type == T_QSTRING) {
		string buff = str.substr(1, str.size() - 2);
		content = mk_string(buff.c_str(), buff.size());
	}
	else content = mk_string(str.c_str(), str.size());
}

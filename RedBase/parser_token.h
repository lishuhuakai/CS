#pragma once
#include <string>
#include "parser_common.h"
using namespace std;

enum TOKENKIND {
	RW_CREATE,
	RW_DROP,
	RW_TABLE,
	RW_INDEX,
	RW_LOAD,
	RW_HELP,
	RW_EXIT,
	RW_PRINT,
	RW_SET,
	RW_AND,
	RW_INTO,
	RW_VALUES,
	RW_SELECT,
	RW_FROM,
	RW_WHERE,
	RW_ORDER,
	RW_GROUP,
	RW_BY,
	RW_DESC,
	RW_ASC,
	RW_INSERT,
	RW_DELETE,
	RW_UPDATE,
	RW_MAX,
	RW_MIN,
	RW_AVG,
	RW_SUM,
	RW_COUNT,
	RW_RESET,
	RW_BUFFER,
	RW_ON,
	RW_OFF,
	RW_STRING,		// 普通的字符串

	T_STRING,
	T_INT,
	T_FLOAT,

	RW_COMMENT,
	RW_SPACE,
	T_QSTRING,
	T_LT,
	T_LE,
	T_GT,
	T_GE,
	T_EQ,
	T_NE,
	T_EOF,

	RW_LPAREN,		// (
	RW_RPAREN,		// )
	RW_COMMA,		// ,
	RW_SEMICOLON,	// ;
	RW_ATTRTYPE, 
	RW_STAR,
	RW_DOT,
};
string tokenName(TOKENKIND type);

//
// Token 词法分析器每次会吐出一个词法单元,即token
// 
struct Token {
	TOKENKIND type;
	size_t line;
	char* content;
	Token(TOKENKIND type, size_t line, const string& content);
	// 下面这个构造函数在某些时候也是用得到的!
	Token(TOKENKIND type) 
		: type(type), content(nullptr)
		, line(0)
	{}
	friend ostream& operator<<(ostream& os, const Token& tk);
};


struct TokenDef
{
	TOKENKIND type;		// 记录这个Token的类型
	regex pattern;		// 记录对应的正则表达式
	TokenDef(TOKENKIND type, const char* patt) :
		type(type), pattern(patt)
	{}
};


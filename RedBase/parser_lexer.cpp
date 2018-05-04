#include <iostream>
#include <memory>
#include <string.h>
#include <vector>
#include "parser_common.h"
#include "parser_lexer.h"
#include "parser_token.h"
using namespace std;

static vector<TokenDef> tokens = {
	{ RW_SPACE, "[[:blank:]]+"},
	{ RW_CREATE, "(create)|(CREATE)\\b" },
	{ RW_DROP, "(drop)|(DROP)\\b" },
	{ RW_TABLE, "(table)|(TABLE)\\b" },
	{ RW_INDEX, "(index)|(INDEX)\\b" },
	{ RW_LOAD, "(load)|(LOAD)\\b" },
	{ RW_HELP, "(help)|(HELP)\\b" },
	{ RW_EXIT, "(exit)|(EXIT)\\b" },
	{ RW_PRINT, "(print)|(PRINT)\\b" },
	{ RW_INSERT, "(insert)|(INSERT)\\b" },
	{ RW_INTO, "(into)|(INTO)\\b" },
	{ RW_VALUES, "(values)|(VALUES)\\b" },
	{ RW_SELECT, "(select)|(SELECT)\\b" },
	{ RW_WHERE, "(where)|(WHERE)\\b" },
	{ RW_FROM, "(from)|(FROM)\\b" },

	{ RW_COUNT, "(count)|(COUNT)\\b" },
	{ RW_AVG, "(avg)|(AVG)\\b" },
	{ RW_SUM, "(sum)|(SUM)\\b" },
	{ RW_MAX, "(max)|(MAX)\\b" },
	{ RW_MIN, "(min)|(MIN)\\b" },

	{ RW_GROUP, "(group)|(GROUP)\\b" },
	{ RW_ORDER, "(order)|(ORDER)\\b" },
	{ RW_BY, "(by)|(BY)\\b" },
	{ RW_DESC, "(desc)|(DESC)\\b" },
	{ RW_ASC, "(asc)|(ASC)\\b" },

	{ RW_STRING, "[a-zA-Z][a-zA-Z0-9_]*" },
	{ RW_COMMENT, "%[^\n]*" },
	{ T_QSTRING, "\"([^\"\n]|(\"\"))*\""},
	{ T_NE, "(<>)|(!=)" },
	{ T_LT, "<" },
	{ T_LE, "<=" },
	{ T_GT, ">" },
	{ T_GE, ">="},
	{ T_EQ, "=" },
	{ T_FLOAT, "[\\+-]?[0-9]+\\.[0-9]*[eE][0-9]+" },
	{ T_FLOAT, "[\\+-]?[0-9]+\\.[0-9]*" }, // 写这个东西的时候,还要注意字符的转义
	{ T_INT, "[+-]?[0-9]+" },
	{ RW_COMMA, "," },
	{ RW_SEMICOLON, ";" },
	{ RW_LPAREN, "\\(" },
	{ RW_RPAREN, "\\)" },
	{ RW_STAR, "\\*" },
	{ RW_DOT, "\\."}
};

static set<TOKENKIND> ignores = { RW_SPACE };

LexerPtr Lexer::instance_ = nullptr;

LexerPtr Lexer::instance()
{
	if (instance_ == nullptr) {
		instance_ = make_shared<Lexer>();
	}
	return instance_;
}

Lexer::Lexer()
{
}

Lexer::~Lexer()
{
}

//
// next函数用于获取下一个元素,并且消耗掉这个元素.
//
TokenPtr Lexer::next()
{
	if (eof_) return make_shared<Token>(T_EOF, line_, "");
	bool wrong = true;
	// 使用各种正则表达式去匹配
	size_t	line = line_;
	size_t i = 0;
	smatch mc;
	for (; i < tokens.size(); ++i) {
		bool finded = regex_search(stream_.cbegin() + offset_, stream_.cend(), mc, tokens[i].pattern);
		if (finded && mc.position() == 0 && mc.length() != 0) {
			wrong = false;
			offset_ += mc.length();
			eof_ = offset_ == stream_.length();
			string matchStr = mc.str();
			if (matchStr.find('\n') != matchStr.npos) { // new line
				line_ += countNL(matchStr);
			}
			if (ignores.find(tokens[i].type) != ignores.end()) { // should be ignored!
				if (!eof_) return next();
				return make_shared<Token>(T_EOF, line, "");
			}
			break;
		}
	}
	if (wrong) { // 这么多正则表达式,居然没有一个匹配,也就是所,出现了无法匹配的字符.
		GrammarError error = { line, "There are some words that We can't recongnize it!" };
		throw error;
	}
	return make_shared<Token>(tokens[i].type, line, mc.str());
}

int Lexer::countNL(const string& str) 
{
	int res = 0;
	for (auto s : str) {
		if ('\n' == s)
			res++;
	}
	return res;
}




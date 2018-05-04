#ifndef PARSER_LEXER_H
#define PARSER_LEXER_H

#include <regex>
#include <set>
#include <list>
#include <tr1/memory>
#include "parser_common.h"
#include "parser_token.h"
using namespace std;
struct TokenDef;


class Lexer;
using LexerPtr = shared_ptr<Lexer>;
using TokenPtr = shared_ptr<Token>;

//
// Lexer是一个词法分析器.
//
class Lexer
{
public:
	Lexer();
public:
	~Lexer();
private:
	string stream_;
	size_t offset_;					// 用于记录已经解析到了的位置
	size_t line_;					// 记录文本所在的行,列也非常重要
	bool eof_;
public:
	void setStream(const string& stream) {
		offset_ = 0;
		line_ = 1;				// 从第1行开始
		stream_ = stream;
		eof_ = stream.size() == 0;
	}
	TokenPtr next();
private:
	int countNL(const string&);
private:
	static LexerPtr instance_;
public:
	static LexerPtr instance();
};

#endif /* PARSER_LEXER_H */

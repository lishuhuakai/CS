#pragma once
#include <iostream>
#include <list>
#include <regex>
#include <sstream>
#include <memory>
using namespace std;

//
// GeneralError是一个非常简易的错误类,只用于记录出错的信息,对于我们这个简易的YACC来说,足够了.
//
struct GeneralError {
public:
	string msg;
	GeneralError(const string& msg = "") :
		msg(msg)
	{}
public:
	string what() const {
		return msg;
	}
};

//
// GrammarError用于记录文法的错误. 
//
struct GrammarError : public GeneralError {
public:
	GrammarError(size_t line, const string& message = "") 
		: GeneralError()
	{
		ostringstream grammarErr;
		grammarErr << "In line " << line << " : " << message << endl;
		msg = grammarErr.str();
	}
};
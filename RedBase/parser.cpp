#include <iostream>
#include <string>
#include "parser.h"
#include "parser_node.h"
#include "parser_token.h"
#include "parser_lexer.h"
#include "parser_syntaxtree.h"
#include "parser_interp.h"
#include "rm_error.h"
#include "pf_error.h"
#include "ix_error.h"
#include "ql_error.h"
#include "sm_error.h"
using namespace std;

static NODE* parse_tree;
extern PFManager pfManager;
extern SMManager smManager;
extern QLManager qlManager;
bool stop = false;

void PrintError(RC rc)
{
	if (abs(rc) <= END_PF_WARN)
		PFPrintError(rc);
	else if (abs(rc) <= END_RM_WARN)
		RMPrintError(rc);
	else if (abs(rc) <= END_IX_WARN)
		IXPrintError(rc);
	else if (abs(rc) <= END_SM_WARN)
		SMPrintError(rc);
	else if (abs(rc) <= END_QL_WARN)
		QLPrintError(rc);
	else
		cerr << "Error code out of range: " << rc << "\n";
}

RC RBparse()
{
	RC errval;
	stop = false;
	string buffer;
	LexerPtr lexer = Lexer::instance();
	SyntaxTree tree(lexer);
	while (!stop) {
		cout << PROMPT;
		cout.flush();
		getline(cin, buffer);
 	    tree.resetParser(buffer);
		try {
			parse_tree = tree.buildSyntaxTree();
		}
		catch (const GeneralError& err)
		{
			cerr << err.what() << endl;
			continue;
		}
		if (errval = interp(parse_tree)) { /* ½âÎöÓï·¨Ê÷ */
			PrintError(errval);
			if (errval < 0) stop = true;
		}

	}
	return 0;
}
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "redbase.h"
#include "sm_manager.h"
#include "pf_manager.h"
#include "ql_manager.h"
#include "ix_manager.h"
#include "parser.h"
using namespace std;

extern PFManager pfManager;
extern RMManager rmManager;
extern IXManager ixManager;
extern SMManager smManager;
extern QLManager qlManager;

int main(int argc, char **argv)
{
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " dbname\n";
		exit(1);
	}
	char* dbname = argv[1];
	smManager.openDb(dbname);
	RBparse();
	smManager.closeDb();
	cout << "Bye.\n";
	return 0;
}

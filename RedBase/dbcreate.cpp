#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include "rm.h"
#include "pf_manager.h"
#include "rm_manager.h"
#include "rm_filehandle.h"
#include "data_attr.h"
#include "catalog.h"

using namespace std;

int main(int argc, char* argv[])
{
	RC rc;
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " <dbname>\n";
		exit(1);
	}

	string dbname(argv[1]);	 /* 数据库的名称 */
	stringstream command;
	command << "mkdir " << dbname;
	rc = system(command.str().c_str());
	if (rc != 0) {
		cerr << argv[0] << " mkdir error for " << dbname << "\n";
		exit(rc);
	}

	if (chdir(dbname.c_str()) < 0) {
		cerr << argv[0] << " chdir error to " << dbname << "\n";
		exit(1);
	}

	// create the system catalogs...
	PFManager pfManager;
	RMManager rmManager;
	RMFilePtr rel, attr;
	rmManager.createFile("relcat", sizeof(DataRel));
	rmManager.openFile("relcat", rel);
	rmManager.createFile("attrcat", sizeof(DataAttr));
	rmManager.openFile("attrcat", attr);

	DataRel relcat_rel;
	strcpy(relcat_rel.relname, "relcat");
	relcat_rel.attrcount = DataRel::members();
	relcat_rel.rcdlen = sizeof(DataRel);
	relcat_rel.pages = 1;	// initially
	relcat_rel.rcds = 2;	// relcat & attrcat

	DataRel attrcat_rel;
	strcpy(attrcat_rel.relname, "attrcat");
	attrcat_rel.attrcount = DataAttr::members();
	attrcat_rel.rcdlen = sizeof(DataAttr);
	attrcat_rel.pages = 1; // initially
	attrcat_rel.rcds = DataAttr::members() + DataRel::members();

	RID rid;
	
	rel->insertRcd((Ptr)&relcat_rel, rid);
	rel->insertRcd((Ptr)&attrcat_rel, rid);

	DataAttr a;
	strcpy(a.relname, "relcat");
	strcpy(a.attrname, "relname");
	a.offset = offsetof(DataRel, relname);
	a.type = STRING;
	a.len = MAXNAME + 1;
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "relcat");
	strcpy(a.attrname, "rcdlen");
	a.offset = offsetof(DataRel, rcdlen);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "relcat");
	strcpy(a.attrname, "attrcount");
	a.offset = offsetof(DataRel, attrcount);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "relcat");
	strcpy(a.attrname, "pages");
	a.offset = offsetof(DataRel, pages);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "relcat");
	strcpy(a.attrname, "numRecords");
	a.offset = offsetof(DataRel, rcds);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);


	// attrcat attrs
	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "relname");
	a.offset = offsetof(DataAttr, relname);
	a.type = STRING;
	a.len = MAXNAME + 1;
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "attrname");
	a.offset = offsetof(DataAttr, relname) + MAXNAME + 1;
	a.type = STRING;
	a.len = MAXNAME + 1;
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "offset");
	a.offset = offsetof(DataAttr, offset);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "type");
	a.offset = offsetof(DataAttr, type);
	a.type = INT;
	a.len = sizeof(AttrType);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "len");
	a.offset = offsetof(DataAttr, len);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	strcpy(a.relname, "attrcat");
	strcpy(a.attrname, "idxno");
	a.offset = offsetof(DataAttr, idxno);
	a.type = INT;
	a.len = sizeof(int);
	a.idxno = -1;
	attr->insertRcd((Ptr)&a, rid);

	rmManager.closeFile(attr);
	rmManager.closeFile(rel);
	return 0;
}

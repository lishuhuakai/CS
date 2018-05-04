#ifndef SM_MANAGER
#define SM_MANAGER
#include "ix_manager.h"
#include "rm_manager.h"
#include "data_attr.h"
#include "catalog.h"

struct AttrInfo;
class SMManager {
	friend class QLManager;
public:
	SMManager() : opened_(false) {}
	~SMManager() {}
public:
	RC openDb(const char* dbname);
	RC closeDb();
	RC createTable(const char *relname, int count, AttrInfo  *infos);
	RC dropTable(const char *relname);           // Destroy relname
	RC createIndex(const char *relname, const char *attrname);
	RC dropIndex(const char *relname, const char *attrname);
	RC load(const char *relname, const char *filename);
	RC help();                                   // Help for database
	RC help(char *relname);						// Help for relname
	RC print(const char *relname);               // Print relname
public:
	RC lookupAttr(const char* relname, const char* attrname, DataAttr& attr, RID &rid);
	RC lookupAttrs(const char* relname, int& nattrs, DataAttr attrs[]);
	RC lookupRel(const char* relname, DataRel &rel, RID &rid);
private:
	RC loadFromTable(const char* relname, int& count, DataAttr* &attrs);
private:
	RMFilePtr rel_;
	RMFilePtr attr_;
	char workdir_[1024];
	bool opened_;		// 是否已经打开
};

#endif /* SM_MANAGER */
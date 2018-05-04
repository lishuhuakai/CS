#include "pf_internal.h"
#include "pf_hashtable.h"

PFHashTable::PFHashTable(uint capacity)
	: capacity_(capacity)
	, table_(capacity)
{
}

//
// search - 在表中搜寻这样的三元组,找到了,返回即可,否则的话,返回-1
// 
int PFHashTable::search(int fd, Page num)
{
	int key = calcHash(fd, num);
	if (key < 0) return -1;
	list<Triple>& lst = table_[key];
	list<Triple>::const_iterator it;
	for (it = lst.begin(); it != lst.end(); it++) {
		if ((it->fd == fd) && (it->num == num))
			return it->slot;
	}
	return -1;
}


//
// insert - 往hash表中插入一个元素
// 
bool PFHashTable::insert(int fd, Page num, int slot)
{
	int key = calcHash(fd, num);
	if (key < 0) return false;
	list<Triple>& lst = table_[key];
	list<Triple>::const_iterator it;
	// table中不能已经存在这样的entry
	for (it = lst.begin(); it != lst.end(); it++) {
		if ((it->fd == fd) && (it->num == num))
			return false;
	}
	lst.push_back(Triple(fd, num, slot));
	return true;
}

//
// remove - 从hash表中移除掉一个元素
// 
bool PFHashTable::remove(int fd, Page num)
{
	int key = calcHash(fd, num);
	if (key < 0) return false;
	list<Triple>& lst = table_[key];
	list<Triple>::const_iterator it;
	// table中不能已经存在这样的entry
	for (it = lst.begin(); it != lst.end(); it++) {
		if ((it->fd == fd) && (it->num == num)) {
			lst.erase(it);
			return true;
		}
	}
	return false;
}
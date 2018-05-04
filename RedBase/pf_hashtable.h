//
// File:		pf_hashtable.h
// Description: PFHashTable class interface
//

#ifndef PF_HASHTABLE_H
#define PF_HASHTABLE_H

#include "pf_internal.h"
#include <list>
#include <vector>
using namespace std;


//
// PFHashTable - 一个非常简易的hash表实现,你甚至可以说,简单到有些不可思议,
// 这个table记录了一些什么呢?这个玩意其实就是一个map,记录了fd指代的这个文件的第num个页面
// 究竟在那个slot里面.
//
class PFHashTable {
	struct Triple {
		int fd;
		Page num;
		int slot;
		Triple(int fd, Page num, int slot) : fd(fd), num(num), slot(slot) {}
	};
public:
	PFHashTable(uint capacity);
	~PFHashTable() {}
public:
	int search(int fd, Page num);
	bool insert(int fd, Page num, int slot);
	bool remove(int fd, Page num);
private:
	int calcHash(int fd, Page num)
	{
		return (fd + num) % capacity_;
	}
private:
	uint capacity_;
	vector<list<Triple>> table_;
};

#endif /* PF_HASHTABLE_H */
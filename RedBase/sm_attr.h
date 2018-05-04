#ifndef SM_ATTR_H
#define SM_ATTR_H


struct AttrInfo {
	char* attrname;		// 属性的名称
	AttrType type;		// 属性的类型
	int len;			// 属性的长度
};

//
// DataAttr - 主要用于描述表中的属性
//
struct DataAttr {
	char relname[MAXNAME + 1];		// 关系的名称,或者说是table的名称
	char attrname[MAXNAME + 1];		// 属性的名称
	int offset;						// 偏移量
	AttrType type;					// 类型
	int len;						// 长度
	int idxno;						// 如果带有索引的话,索引的编号

public:
	DataAttr()
		: offset(-1)
	{
		memset(relname, 0, MAXNAME + 1);
		memset(attrname, 0, MAXNAME + 1);
	}

	DataAttr(const AttrInfo& info)
		: type(info.type), len(info.len)
		, idxno(-1), offset(-1)
	{
		memcpy(attrname, info.attrname, MAXNAME + 1);
	}

	DataAttr(const DataAttr& rhs)
		: type(rhs.type), len(rhs.len)
		, offset(rhs.offset), idxno(rhs.idxno)
	{
		// tofix
		memcpy(attrname, rhs.attrname, MAXNAME + 1);
		memcpy(relname, rhs.relname, MAXNAME + 1);
	}
	
	DataAttr& operator=(const DataAttr& rhs)
	{
		if (this != &rhs) {
			strcpy(relname, rhs.relname);
			strcpy(attrname, rhs.attrname);
			offset = rhs.offset;
			idxno = rhs.idxno;
			len = rhs.len;
			type = rhs.type;
		}
		return *this;
	}
};

#endif /* SM_ATTR_H */
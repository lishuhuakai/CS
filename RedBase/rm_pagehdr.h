#ifndef RM_PAGEHDR_H
#define RM_PAGEHDR_H
#include "pf.h"
#include "redbase.h"

class BitMap
{
public:
	BitMap(uint slots, Ptr addr)
		: buffer_(addr)
		, bytes_(bytes(slots))
		, capacity_(slots)
	{}
	~BitMap() {}
public:
	//
	// reset - 将pos位置上的bit置为0
	// 
	bool reset(uint pos)
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		buffer_[byte] &= ~(1 << offset);
		return true;
	}

	//
	// set - 给定位置,将对应的bit为设定为1
	// 
	bool set(uint pos)
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		buffer_[byte] |= (1 << offset);
		return true;
	}

	//
	// setAll - 将所有的比特都设置为1
	//
	void setAll()
	{
		for (uint i = 0; i < bytes_; i++) {
			buffer_[i] = 0xff;
		}
	}

	//
	// resetAll - 将所有的比特都设置为0
	// 
	void resetAll()
	{
		for (uint i = 0; i < bytes_; i++)
		{
			buffer_[i] = 0x00;
		}
	}

	//
	// available - 测试给定的位置是否为1,也即该位置是否空闲
	// 
	bool available(uint pos) const
	{
		if (pos >= capacity_) return false;
		uint byte = pos / 8;
		uint offset = pos % 8;
		return buffer_[byte] & (1 << offset);
	}
public:
	//
	// bytes - 计算为slots个数的slot,需要消耗的字节数目
	// 
	uint static bytes(uint slots)
	{
		uint n = slots / 8;
		if (slots % 8 != 0) n++;
		return n;
	}

	uint size()
	{
		return bytes_;
	}
private:
	Ptr buffer_;
	uint capacity_;
	uint bytes_;
};

//
// RMPageHdr - 页的首部,在我们的实现中,每一个页的首部都包含着这样的信息
// 
class RMPageHdr {
public:
	RMPageHdr(uint slots, Ptr addr)
		: buffer_(addr)
		, slots_(slots)
		, map(slots, addr + sizeof(int) + 2 * sizeof(uint))
	{}
	~RMPageHdr() {}
public:
	int lenOfHdr()
	{
		return sizeof(int) + 2 * sizeof(uint) + map.size();
	}

	int next()
	{
		return *reinterpret_cast<int *>(buffer_);
	}

	uint slots()
	{
		return *reinterpret_cast<uint *>(buffer_ + sizeof(int));
	}

	uint remain()
	{
		return *reinterpret_cast<uint *>(buffer_ + sizeof(int) + sizeof(uint));
	}

	void setNext(int val)
	{
		*reinterpret_cast<int *>(buffer_) = val;
	}

	void setSlots(uint val)
	{
		*reinterpret_cast<uint *>(buffer_ + sizeof(int)) = val;
	}

	void setRemain(uint val)
	{
		*reinterpret_cast<uint *>(buffer_ + sizeof(int) + sizeof(uint)) = val;
	}
public:
	BitMap map;
private:
	uint slots_;
	Ptr buffer_;
};


#endif /* RM_PAGEHDR_H */
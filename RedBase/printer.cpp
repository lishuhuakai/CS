#include <iostream>
#include <string.h>
#include <ostream>
#include <algorithm>
#include <sstream>
#include "printer.h"
#include "ql_query.h"
using namespace std;


static const char* AggFuncName[] = {
	"NO_F", "MIN", "MAX", "COUNT", "SUM", "AVG"
};
static char infos[MAXATTRS][MAXPRINTLEN];

Printer::Printer(const DataAttr *attrs, int nattrs)
	: count_(nattrs), printed_(0), ndiscs_(0)
	, discs_(nullptr), attrs_(attrs)
{
	for (int i = 0; i < nattrs; i++) {
		sprintf(infos[i], "%s", attrs[i].attrname);
		int width;
		int len = strlen(infos[i]) + 1;
		if (attrs[i].type == STRING) {
			width = max(len, attrs[i].len);
			width = min(width, MAXPRINTLEN);
		}
		else {
			width = len > 12 ? len: 12;
		}
		infos_.push_back(infos[i]);
		colWidth_.push_back(width);
	}
}

Printer::Printer(const ItemDiscrip *discs, int ndiscs)
	: ndiscs_(ndiscs), discs_(discs)
	, printed_(0), count_(0), attrs_(nullptr)
{
	for (int i = 0; i < ndiscs; i++) {
		if (discs[i].func != NO_F)
			sprintf(infos[i], "%s(%s)", AggFuncName[discs[i].func], discs[i].attrname);
		else
			sprintf(infos[i], "%s", discs[i].attrname);
		int width;
		int len = strlen(infos[i]) + 1;
		if (discs[i].type == STRING) {
			width = max(len, discs[i].len);
			width = min(width, MAXPRINTLEN);
		}
		else {
			width = len > 12 ? len: 12;
		}
		infos_.push_back(infos[i]);
		colWidth_.push_back(width);
	}
}

//
// printHeader - 输出头部的信息
//
void Printer::printHeader()
{
	int len;
	int dashes = 0;
	int spaces = 0;
	for (int i = 0; i < infos_.size(); i++) {
		cout << infos_[i];
		len = strlen(infos_[i]);
		dashes += len;
		spaces = colWidth_[i] - len;
		for (int j = 0; j < spaces; j++) cout << " ";
		dashes += spaces;
	}
	cout << endl;
	for (int k = 0; k < dashes; k++) cout << "-";
	cout << endl;
}


void Printer::printFooter()
{
	cout << endl;
	cout << printed_ << " tuple(s)." << endl;
}


void Printer::printDiscs(uint8_t *data)
{
	char buffer[MAXPRINTLEN];
	printed_++;

	int offset = 0;
	for (int i = 0; i < ndiscs_; i++) {
		int space = 0;
		uint8_t *ptr = data + offset;
		switch (discs_[i].type)
		{
			case STRING:
				if (discs_[i].len > MAXPRINTLEN) {
					memcpy(buffer, ptr, MAXPRINTLEN - 1);
					buffer[MAXPRINTLEN - 1] = '\0';
					buffer[MAXPRINTLEN - 3] = buffer[MAXPRINTLEN - 2] = '.';
				}
				else {
					memcpy(buffer, ptr, discs_[i].len);
				}
				break;
			case INT:
			{
				int val = *reinterpret_cast<int *>(ptr);
				sprintf(buffer, "%d", val);
				break;
			}
			case FLOAT:
			{
				float fval = *reinterpret_cast<float *>(ptr);
				sprintf(buffer, "%f", fval);
				break;
			}
			default:
				break;
		}
		cout << buffer;
		space = colWidth_[i] - strlen(buffer);
		for (int j = 0; j < space; j++) cout << " ";
		offset += discs_[i].len;
	}
	cout << endl;
}

void Printer::printAttrs(uint8_t* data)
{
	char buffer[MAXPRINTLEN];
	printed_++;
	for (int i = 0; i < count_; i++) {
		int space = 0;
		uint8_t *ptr = data + attrs_[i].offset;
		switch (attrs_[i].type)
		{
			case STRING:
				if (attrs_[i].len > MAXPRINTLEN) {
					memcpy(buffer, ptr, MAXPRINTLEN - 1);
					buffer[MAXPRINTLEN - 1] = '\0';
					buffer[MAXPRINTLEN - 3] = buffer[MAXPRINTLEN - 2] = '.';
				}
				else {
					memcpy(buffer, ptr, attrs_[i].len);
				}
				break;
			case INT:
			{
				int val = *reinterpret_cast<int *>(ptr);
				sprintf(buffer, "%d", val);
				break;
			}
			case FLOAT:
			{
				float fval = *reinterpret_cast<float *>(ptr);
				sprintf(buffer, "%f", fval);
				break;
			}
			default:
				break;
		}
		cout << buffer;
		space = colWidth_[i] - strlen(buffer);
		for (int j = 0; j < space; j++) cout << " ";
	}
	cout << endl;
}

void Printer::print(uint8_t* data)
{
	if (data == nullptr) return;
	if (discs_) printDiscs(data);
	else printAttrs(data);
}


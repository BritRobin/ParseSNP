#ifndef SNIPPARSER_H
#define SNIPPARSER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <wchar.h>


class SnipParser
{
 
public:
	std::string fi_;
	int SNPCount(void);
	bool Ancestory(wchar_t* fi_);
	bool FTDNA(wchar_t* fi_);
	bool f23andMe(wchar_t* fi_);
	void FConvert(void);  //<- One off code generator
	unsigned int IllumTransVG(void);
	unsigned int IllumUntransVG(void);
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
private:
	struct ST;
	unsigned int loadCount_ = 0;
	unsigned int illuminaU_ = 0;
	unsigned int illuminaT_ = 0;
};

#endif--------------
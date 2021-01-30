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
	struct ST;
	int SNPCount(void);
	bool Ancestory(wchar_t* fi_);
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
private:
	int unsigned loadCount_= 0;

};

#endif--------------
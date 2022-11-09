#ifndef SNIPPARSER_H
#define SNIPPARSER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <wchar.h>
#include <ctime>
#include <ctype.h>
#include <windows.h>


class SnipParser
{
 
public:
	std::string fi_;
	int SNPCount(void);
	std::string NCBIBuild(void);
	bool Ancestory(wchar_t* fi_);
	bool FTDNA(wchar_t* fi_);
	bool AncestoryWriter(wchar_t* fi_); //output all loaded/parsed SNPs
	bool f23andMe(wchar_t* fi_);
	void FConvert(void);  //<- One off code generator
	unsigned int IllumTransVG(void);
	unsigned int IllumUntransVG(void);
	std::string PVer(void); //return program/class version
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
	std::string PathogenicCall(int rsid, char riskalelle, float oddsratio, float* sumoddsratio);

private:
	struct ST;
	wchar_t fileLoaded_[260];
	std::string Pversion_ = "0.2 Beta"; //Progran Version
	std::string NCBIBuild_="--";
	unsigned int loadCount_ = 0;
	unsigned int illuminaU_ = 0;
	unsigned int illuminaT_ = 0;
};

#endif
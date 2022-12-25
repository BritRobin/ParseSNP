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
	bool MergeAncestory(wchar_t* fi_);
	bool FTDNA(wchar_t* fi_);
	bool MergeFTDNA(wchar_t* fi_);
	bool AncestoryWriter(wchar_t* fi_); //output all loaded/parsed SNPs
	bool f23andMe(wchar_t* fi_);
	bool Mergef23andMe(wchar_t* fi_);
	void FConvert(void);  //<- One off code generator
	wchar_t sex(void);
	unsigned int merged(void);
	bool MergeState(void);
	unsigned int IllumTransVG(void);
	unsigned int IllumUntransVG(void);
	unsigned int MergeProcessed(void);
	
	std::string PVer(void); //return program/class version
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
	std::string PathogenicCall(int rsid, char riskallele, float oddsratio, float* sumoddsratio);

private:
	struct ST;
	bool abortMerge_ = false;
	wchar_t fileLoaded_[260] = { 0 };
	wchar_t sex_ = '?';
	std::string Pversion_ = "0.31 Beta"; //Progran Version
	std::string NCBIBuild_= "--";
	unsigned int loadCount_ = 0;
	unsigned int origloadcount_ = 0; //for reversion and checking
	unsigned int end_index_ = 0;
	unsigned int illuminaU_ = 0;
	unsigned int illuminaT_ = 0;
	unsigned int merged_  = 0;
	unsigned int failcheck_ = 0;
	unsigned int allcecked_ = 0;
	unsigned int mergefile_ = 0;
	int FTDNADecode(std::string code);
	int f23andMeDecode(std::string code);
	__forceinline bool mergeRs(int code, std::string nbuffer);
	void initMergeCopy(void);
	void revertMerge(void);
};

#endif
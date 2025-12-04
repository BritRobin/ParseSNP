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
#include <mutex> //for multithreading
// Forward declarations or define structs SM and ST if needed
struct SM;
struct ST;
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
	void FConvert(void);  //<- One off internal development code generator
	wchar_t sex(void);
	unsigned int merged(void);
	bool MergeState(void);
	unsigned int IllumTransVG(void);
	unsigned int IllumUntransVG(void);
	unsigned int MergeProcessed(void);
	bool mergeRs(int code, const std::string& line);
	std::string PVer(void); //return version
	std::string PAbout(void); //return program/class about info
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
	std::string PathogenicCall(int rsid, char riskallele, float oddsratio, float* sumoddsratio);

	// Conservative buffer size - double typical max to be safe was set to max human SNPs @ 10430639
	static const int DNA_SNP_BUFFER_SIZE = 1600000;

private:
	// === ALL YOUR MEMBER VARIABLES MUST BE HERE ===
	// ONLY add the merge-specific variables and mutex
	std::mutex merge_mutex_;
	bool abortMerge_ = false;
	unsigned int end_index_ = 0;
	unsigned int allcecked_ = 0;
	unsigned int failcheck_ = 0;
	unsigned int merged_ = 0;
	unsigned int origloadcount_ = 0;
	unsigned int mergefile_ = 0;

	// Keep your existing class members (don't remove these):
	unsigned int loadCount_ = 0;
	wchar_t fileLoaded_[260];
	char sex_;
	std::string NCBIBuild_;
	std::string Pversion_ = "1.0.0"; //SET **VERSION** HERE
	std::string PAbouttxt_ = "Written by Robin Taylor. 2021 - 2025 \nReleased under GNU GPL v3.0"; //ABOUT INFO
	unsigned int illuminaU_ = 0;
	unsigned int illuminaT_ = 0;
	int FTDNADecode(std::string code);
	int f23andMeDecode(std::string code);
	void initMergeCopy(void);
	void revertMerge(void);
};

#endif
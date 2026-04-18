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
#include <mutex> //For multithreading
// Forward declarations or define structs SM and ST if needed
struct SM;
struct ST;
class SnipParser
{

public:
	std::string fi_ = "";
	int SNPCount(void) const;
	std::string NCBIBuild(void) const;
	bool Ancestory(wchar_t* fi_);
	bool MergeAncestory(wchar_t* fi_);
	bool FTDNA(wchar_t* fi_);
	bool MergeFTDNA(wchar_t* fi_);
	bool AncestoryWriter(wchar_t* fi_); //output all loaded/parsed SNPs
	bool f23andMe(wchar_t* fi_);
	bool Mergef23andMe(wchar_t* fi_);
	void FConvert(void);  //<- One off internal development code generator
	wchar_t sex(void) const;
	unsigned int merged(void) const;
	bool MergeState(void) const;
	unsigned int IllumTransVG(void) const;
	unsigned int IllumUntransVG(void) const;
	unsigned int MergeProcessed(void);
	bool mergeRs(int code, const std::string& line);
	std::string PVer(void) const; //return version
	std::string PAbout(void) const; //return program/class about info
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
	std::string errorInfo(unsigned int error); //return error message for error code
	std::string PathogenicCall(int rsid, char riskallele, float oddsratio, float* sumoddsratio);
	// Conservative buffer size - double typical max to be safe was set to max human SNPs @ 10430639
	static const int DNA_SNP_BUFFER_SIZE = 1600000;
	//Error code
	unsigned int errorCode_ = 0;
//Note I am aware this program does not currently support trisomy I am not sure if ancestry files squence the extra SNP from the 3 chromosome! 
private:
	// === ALL YOUR MEMBER VARIABLES MUST BE HERE ===
    // Move these from global scope to private members:
	struct ST {
		int rs;       // RS number
		char ch[4];   // Chromosome (max 3 chars + null)
		int pos;      // Position
		char a;       // first nucleotide
		char b;       // second nucleotide
	};

	struct SM {
		int rs;       // RS number
		char a;       // first nucleotide
		char b;       // second nucleotide
	};

	// The actual data vectors
	std::vector<ST> snp;
	std::vector<SM> snpM;

	// Default empty instances (if needed)
	ST defaultST;
	SM defaultSM;
	std::mutex mutex_;  // One mutex to rule them all!
	
	bool abortMerge_ = false;
	// ====================================================
    // CONFIGURABLE PARAMETERS - Adjust for your needs
    // ====================================================
	static constexpr unsigned int Y_CHROMOSOME_NO_READ_THRESHOLD = 16; // Empirical threshold for sex determination in Ancestory files
	//more defensive code for invalid files
	static constexpr unsigned int TOTAL_BUFFER_SIZE				 = MAX_PATH + 20; //Allow for poaiblw overun in corrupt or malicious files
	static constexpr unsigned int MAX_RSID_NUMBER_LEN			 = 23;
	static constexpr unsigned int READ_LIMIT					 = 256;
	static constexpr unsigned int INVALID_LINE_LIMIT			 = 2000;
	static constexpr unsigned int BUFFER_SIZE					 = 260;
	//more defensive code for invalid files
	unsigned int end_index_		= 0;
	unsigned int allcecked_		= 0;
	unsigned int failcheck_		= 0;
	unsigned int merged_        = 0;
	unsigned int origloadcount_ = 0;
	unsigned int mergefile_     = 0;
	unsigned int loadCount_     = 0;
	//Start: Very basic error handling
	std::string errorMessage_	= "";
	std::string error01_		= "File not found or could not be opened.";
	std::string error02_		= "File read error possibly invalid or corrupt .";
	std::string error03_		= "File read aborted possible partial data or corrupt file.";
	std::string error04_		= "File is empty a 0 byte file!";
	//Start: Very basic error handling
	wchar_t fileLoaded_[260] = { '\0' };
	char sex_				 = '-';
	std::string NCBIBuild_	 = "";
	std::string Pversion_	 = "1.0.2"; //SET **VERSION** HERE
	std::string PAbouttxt_	 = "Written by Robin Taylor. 2021 - 2026 \nReleased under GNU GPL v3.0"; //ABOUT INFO
	unsigned int illuminaU_  = 0;
	unsigned int illuminaT_  = 0;
	int FTDNADecode(std::string code);
	int f23andMeDecode(std::string code);
	void initMergeCopy(void);
	void revertMerge(void);
};

#endif
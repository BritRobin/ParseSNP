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
#include <cmath> //for statistical pathogenics calulation // 4/20/2026

// Forward declarations or define structs SM and ST if needed
struct SM;
struct ST;
class SnipParser
{
public:
	std::string fi_ = "";
	bool Ancestory(wchar_t* fi_);
	bool MergeAncestory(wchar_t* fi_);
	bool FTDNA(wchar_t* fi_);
	bool MergeFTDNA(wchar_t* fi_);
	bool AncestoryWriter(wchar_t* fi_); //output all loaded/parsed SNPs
	bool f23andMe(wchar_t* fi_);
	bool Mergef23andMe(wchar_t* fi_);
	wchar_t sex(void) const { return sex_; } //return sex as determined by XX or XY chromosones the sequences won't give data for non-typical cases
	unsigned int IllumTransVG(void) const {	return illuminaT_;	} //Return the number of translated VG codes
	unsigned int IllumUntransVG(void) const	{ return illuminaU_; } //Return the number of untranslated VG codes
	unsigned int SNPCount(void) const { return loadCount_; } //return the number of lines loaded
	unsigned int merged(void) const { return merged_; }
	unsigned int merge_mismatch_errors(void) { return mismatch_rsids_; } //returns the file disaggrement from squencing errors
	unsigned int matchedrsIDs(void) { return matchcount_; } //return the times a rsID in one file matches the rsID in the other!
	// Returns the number of RSIDs present in BOTH files (regardless of allele agreement).
	// For the count of allele disagreements, see merge_mismatch_errors().
	bool mergeRs(int code, const std::string& line);	
	bool RsSearch(int* rs, char* chr1, char* chr2,  char* chr3,  char* chr4, int* pos, char* a, char* b);
	bool MergeState(void) const { return abortMerge_; } //returns if merge failed
	unsigned int MergeProcessed(void) { return(allchecked_); }//Return Merged FIXED 12/31/2025
	std::string errorInfo(unsigned int error); //return error message for error code
	std::string PVer(void) const { return  Pversion_; } //return version number
	std::string PAbout(void) const { return PAbouttxt_; } //return program/class about info
	std::string DetectBuildFromSNPs(void); //Find the build number from offsets
	std::string NCBIBuild(void); //Define here added code to the fuction to ensure a value is found or we have tried!!!
	//New OR code more acurate more contained in the class
	// NEW getters for beta values //rewrite 4/19/2026
	void ResetRisk() { total_beta_ = 0.0f; max_beta_ = 0.0f; missing_beta_ = 0.0f; }
	float GetTotalBeta(void) const { return total_beta_; }
	float GetMaxBeta(void) const { return max_beta_; }
	float GetMaxPossibleOR() const { return exp(max_beta_); }
	float GetPercentile() const { return (max_beta_ > 0.0f) ? (total_beta_ / max_beta_) * 100.0f : 0.0f;}//Add bounds check 4/20/2026 
	float GetMissing() const { return missing_beta_; } //return the max for Missing data!
	double GetCombinedOR() const { return std::exp(total_beta_); } //Changed to double to avoid overflow
	std::string PathogenicCall(int rsid, char riskallele, float oddsratio);
	// Conservative buffer size change for future code beyond this program?
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
	unsigned int end_index_		 = 0;
	unsigned int allchecked_	 = 0;
	unsigned int matchcount_	 = 0;
	unsigned int failcheck_		 = 0;
	unsigned int merged_         = 0;
	unsigned int origloadcount_  = 0;
	unsigned int mergefile_      = 0;
	unsigned int loadCount_      = 0;
	unsigned int mismatch_rsids_ = 0;
	//Pathogenic value // Changed to double in case of float overflow! - 4/20/2026
	double total_beta_			= 0.0f;		// Sum of log odds
	double missing_beta_		= 0.0f;		// The Max for Missing or No-read data
	double max_beta_			= 0.0f;		// Maximum possible sum
	//Pathogenic value 
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
	std::string Pversion_	 = "1.2.0 \nfor educational and research purposes only!"; //SET **VERSION** HERE
	std::string PAbouttxt_	 = "Written by Robin Taylor. 2021 - 2026 \nReleased under GNU GPL v3.0"; //ABOUT INFO
	//Build detection
	struct BuildMarker {
		int rsid;
		int pos_grch37; // hg19
		int pos_grch38; // hg38
	};
	unsigned int illuminaU_  = 0;
	unsigned int illuminaT_  = 0;
	int FTDNADecode(std::string code);
	int f23andMeDecode(std::string code);
	void initMergeCopy(void);
	void revertMerge(void);
	//set of test values for NCBI build 37 vs 38 Offset differnces  {rsid,37offset,38offset}
	const BuildMarker markers[16] = {/* Add more markers with known shifts mainly chr 17*/
	{ 12184267, 715265, 779885 },
	{ 4575098,161155392,161185602 },
	{ 9303281,38074046,39917793 },
	{ 7219923,38074518,39918265 },
	{ 12603332,38082807,39926554 },
	{ 3744246,38084350,39928097 },
	{ 4795405,38088417,39932164 },
	{ 4794820,38089344,39933091 },
	{ 8079416,38092713,39936460 },
	{ 9907088,38035116,39878863 },
	{ 12452894,38036586,39880333 },
	{ 12232497,38040119,39880333 },
	{ 9901146,38043343,39883866 },
	{ 12950743,38049233, 39892980 },
	{ 7359623,38049589,39893336 },
	{ 8067378,38051348,39895095 }
	};
};
#endif
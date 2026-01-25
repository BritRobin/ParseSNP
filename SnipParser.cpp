/*
 * DNA Processor (SnipParser) - A portable C++ library for DNA data file processing
 * Copyright (C) 2026 [Robin Taylor]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "SnipParser.h"
#include <atomic>


// Use 'variant::index' to know the type strored in variant (zero-based index). 
//Read an Ancestory.com/23t0me and FtDNA RAW DNA file and create a 'standard array'
bool  SnipParser::Ancestory(wchar_t* fi_)
{
  
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        int loopbreak = 0;
        int noreadcount = 0;
        bool noY = true;
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;
            //START: reset loadcount_ and vector for next file for next file
            
            snp.clear();
            snp.resize(DNA_SNP_BUFFER_SIZE);
            //END: reset loadcount_ and vector for next file for next file
			wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            while (fs.getline(nbuffer, READ_LIMIT)) //read a line into a temorary buffer
            {   //More parser hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }

				//Reject UTF-8 BOM encoded files
                if (loadCount_ == 0 && (unsigned char)nbuffer[0] == 0xEF && (unsigned char)nbuffer[1] == 0xBB && (unsigned char)nbuffer[2] == 0xBF)
                {
                    fs.close();
                    // Reject UTF-8 BOM encoded files
                    return false;
                }

                int rdindex = 0;
                //GET RS Number numeric part only
                if (nbuffer[rdindex] == 'r' && nbuffer[rdindex +1] == 's' && isdigit((int)nbuffer[rdindex +2])) {
					char num[24];//fixed size 11/12/2025
                    int  nmindex = 0;
                    loopbreak = 0;
                    num[0] = '\0';
                    rdindex += 2;
                    while (isdigit((int)nbuffer[rdindex]) && rdindex < 23)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = '\0';
                    //First in the line is the RS number
                    snp[inx].rs = atoi(num);
                    //move past tab or spaces to next numeric data chromosone number
                    while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                    {//wrote whith braces for readablility
                        rdindex++;
                    }
                    //re-init 
                    nmindex = 0;
                    num[0] = '\0';

                    // 23 is the X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.
                    if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                    {
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        //second in the line is the chromosone number
                        num[nmindex] = '\0';
                        strcpy_s(snp[inx].ch, num);
                    }

                    //move past tab or spaces to next numeric data position number
                    while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                    {//wrote whith braces for readablility
                        rdindex++;
                    }
                    //read position
                    //re-init 
                    nmindex = 0;
                    num[0] = '\0';
                    while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = '\0';
                    //second in the line is the chromosone number
                    snp[inx].pos = atoi(num);

                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                    {
                        rdindex++;
                    }
                    snp[inx].a = nbuffer[rdindex];
                    rdindex++;
                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                    {
                        rdindex++;
                    }
                    //Is Char
                    snp[inx].b = nbuffer[rdindex];
                    //Determine Sex
                    if ((snp[inx].ch[0] == '2' && snp[inx].ch[1] == '4') && snp[inx].a == '0' && snp[inx].b == '0') noreadcount++;
                    if (noY == true && (snp[inx].ch[0] == '2' && snp[inx].ch[1] == '4'))
                    {
                        noY = false;
                    }
                    // SEX DETERMINATION LOGIC FOR ANCESTRYDNA FILES:
                    // ==============================================
                    // AncestryDNA files use special chromosome numbering:
                    // - 23 = X chromosome
                    // - 24 = Y chromosome  
                    // - 25 = PAR region
                    // - 26 = mtDNA
                    //
                    // Files are structured in ascending chromosome order, so:
                    // 1. If NO "24" entries appear => definitely FEMALE (no Y chromosome data)
                    // 2. If "24" entries exist but show '0'/'0' (no reads) => FEMALE
                    //    (Female samples still get Y positions in array but with null alleles)
                    // 3. If "24" entries exist with actual nucleotide alleles => MALE
                    //
                    // The `noreadcount > 15` check is a redundancy for robustness:
                    // - In theory, checking `noY` alone should suffice
                     // - In practice, it catches edge cases (corrupt files, mixed data)
                    // - 15 was chosen empirically as a safe threshold [But can be altered in the header file.
                    //
                    // ALTERNATIVE SIMPLIFIED LOGIC (equally valid):
                    // sex_ = (noY == true) ? 'F' : 'M';
                    // ==============================================
                    if (noreadcount > Y_CHROMOSOME_NO_READ_THRESHOLD || noY == true) sex_ = 'F';
                    else sex_ = 'M';
                    //Determine Sex
                    //increment the primary index
                    inx++;
                    //increment count of lines loaded
                    loadCount_++;
                }
                else 
                {  //i + 1 < searchEnd && i + 1 < READ_LIMIT
                    for (int indx = 0; indx < (READ_LIMIT - 4); indx++)
                    {
                        if ((nbuffer[indx] == 'U' && nbuffer[indx + 1] == 'I' && nbuffer[indx + 2] == 'L' && nbuffer[indx + 3] == 'D') || (nbuffer[indx] == 'u' && nbuffer[indx + 1] == 'i' && nbuffer[indx + 2] == 'l' && nbuffer[indx + 3] == 'd'))
                        {
                            int searchEnd = (indx + 12 < READ_LIMIT) ? (indx + 12) : (READ_LIMIT - 1);  // Don't exceed buffer 1/17/2026
                            for (int i = indx + 4; i <= searchEnd - 2; i++)  // Need space for i and i+1
                            {
                                if (isdigit((unsigned char)nbuffer[i]) && isdigit((unsigned char)nbuffer[i + 1])) //Logic Bug fix check BOTH chars are digits!
                                {
                                    char sub[3] = { nbuffer[i], nbuffer[i + 1], '\0' };
                                    NCBIBuild_ = sub;
                                    break;
                                }
                            }
                        }
                    }
                }
             loopbreak++;
             if (loopbreak == 2000) break;
            }
            fs.close();
        }
        else return false;
    }

    return true;
};

bool  SnipParser::MergeAncestory(wchar_t* fi_)
{ 
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[260] = { '\0' };
        int loopbreak = 0;
        //merge variables
        mergefile_ = 0;
        merged_ = allcecked_ = 0;
		//initialize both variables to current loadcount_
        end_index_ = loadCount_;
        origloadcount_ = loadCount_;
        //merge variables
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = loadCount_ + 1,rst = 0, rdindex = 0;
            //NO!!!: reset loadcount_ and vector for next file for next file
            wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            nbuffer[257] = '\0';        //more efficient
            char num[24] = { '\0' };    //fixed size 11/12/2025
            std::string vercheck;       //more efficient
            initMergeCopy();            //create merge subset
			while (fs.getline(nbuffer, READ_LIMIT) && !abortMerge_) //read a line into a temporary buffer and ensure not aborting merge
            {  //More parser hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }
                //Merge code
                vercheck = nbuffer;
                //Merge code
                rdindex = 0;
                //GET RS Number numeric part only
                if (nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's' && isdigit((int)nbuffer[rdindex + 2])) {
		            int  nmindex = 0;
                    loopbreak = 0;
                    num[0] = '\0';
                    rdindex += 2;
                    while (isdigit((int)nbuffer[rdindex]) && rdindex < 23)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = '\0';
                    mergefile_++; //get the total merge import count so a percentage difference can be calculated
                    //First in the line is the RS number
                    rst = atoi(num);
                       
                    if (rst > 0 && mergeRs(rst, vercheck))
                    {
                        snp[inx].rs = rst;
              
                        //move past tab or spaces to next numeric data chromosone number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';

                        // 23 is the X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.
                        if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                        {
                            while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                            {
                                num[nmindex] = nbuffer[rdindex];
                                rdindex++;
                                nmindex++;
                            }
                            //second in the line is the chromosone number
                            num[nmindex] = '\0';
                            strcpy_s(snp[inx].ch, num);
                        }

                        //move past tab or spaces to next numeric data position number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //read position
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //second in the line is the chromosone number
                        snp[inx].pos = atoi(num);

                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        snp[inx].a = nbuffer[rdindex];
                        rdindex++;
                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        //Is Char
                        snp[inx].b = nbuffer[rdindex];
                        //increment count of lines loaded
                        loadCount_++; //Fixed 12/31/2025
                    }//END MERGE SCOPE!
                    
                    //increment the primary index
                    inx++;
                }
                else
                {
                    for (int indx = 0; indx < READ_LIMIT; indx++)
                    {
                        if ((nbuffer[indx] == 'U' && nbuffer[indx + 1] == 'I' && nbuffer[indx + 2] == 'L' && nbuffer[indx + 3] == 'D') || (nbuffer[indx] == 'u' && nbuffer[indx + 1] == 'i' && nbuffer[indx + 2] == 'l' && nbuffer[indx + 3] == 'd'))
                        {
                            for (int i = indx + 3; i < indx + 12; i++)
                            {
                                if (isdigit((int)nbuffer[i]))
                                {
                                    char sub[3] = "";
                                    sub[0] = nbuffer[i];
                                    sub[1] = nbuffer[i + 1];
                                    sub[2] = '\0';
                                    NCBIBuild_ = sub;
                                    break;
                                }
                            }
                        }
                    }
                }
                loopbreak++;
                if (loopbreak == 2000 || loadCount_ == (DNA_SNP_BUFFER_SIZE - (origloadcount_ + 1))) break;
            }
            fs.close();
        }
        else return false;
    }
    //merge failed
    if (abortMerge_) {
        revertMerge();
        return false;
    }
    
    return true;
}

//FTDNA has VG numbers that may corespond to RS numbers
bool  SnipParser::FTDNA(wchar_t* fi_)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        int loopbreak = 0;
        int VG = 0; //illumina
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;
            int fdind = 0; //ftdna-illumina
            //START: reset loadcount_ and vector for next file for next file
            loadCount_ = 0;
            sex_ = 'F'; //Set sex to female will change if 'Y' is found!
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.clear();
            snp.resize(DNA_SNP_BUFFER_SIZE);
            wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            //END: reset loadcount_ and vecotr for next file for next file
            while (fs.getline(nbuffer, READ_LIMIT))//read a line into a temorary buffer
            {   //More parseer hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }
                //Reject UTF-8 BOM encoded files
                if (loadCount_ == 0 && (unsigned char)nbuffer[0] == 0xEF && (unsigned char)nbuffer[1] == 0xBB && (unsigned char)nbuffer[2] == 0xBF)
                {
                    fs.close();
                    // Reject UTF-8 BOM encoded files
                    return false;
                }
                fdind = 0;
                int rdindex = 2;
                if (nbuffer[fdind] == '\"') fdind++; //ftdna-illumina
                //GET RS Number numeric part only 

                if (((nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's') || (nbuffer[fdind] == 'V' && nbuffer[fdind + 1] == 'G')) && isdigit((int)nbuffer[fdind + 2]))
                {//ftdna-illumina
                    int ftdna = 0;
					char num[25];//fixed size 11/12/2025
                    int  nmindex = 0;
                    loopbreak = 0;
                    rdindex += fdind;//ftdna-illumina
                    num[0] = '\0';

                    if (nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's')
                    {
                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //First in the line is the RS number
                        snp[inx].rs = atoi(num);

                    }
                    else
                    {
                        //FTDNA MyHeritage name : VG01S1077  VG Chromosone number S numeric  rs77931234
                        nmindex = 0;
                        if (nbuffer[fdind] == 'V' && nbuffer[fdind + 1] == 'G')
                        {
                            fdind += 2; //Get first chromosone digit
                            num[nmindex] = nbuffer[fdind];
                            nmindex++;
                            fdind++; //Get second chromosone digit
                            num[nmindex] = nbuffer[fdind];
                            nmindex++;
                            fdind++;
                            fdind++; //Skip Alpha char
                        }
                        while (isdigit((int)nbuffer[fdind]) && fdind < 24)
                        {
                            num[nmindex] = nbuffer[fdind];
                            fdind++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        rdindex =fdind;                  //set read index
                        snp[inx].rs = FTDNADecode(num); //ver 0.3 beta separate decode function!
                    }
                 if (snp[inx].rs > 0) //stop line load of uniterpreted VG codes
                 {
                   //move past tab or spaces to next numeric data chromosone number
                   while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nbuffer[rdindex] != 'X' && nbuffer[rdindex] != 'Y' && nbuffer[rdindex] != 'x' && nbuffer[rdindex] != 'y')
                    {//wrote whith braces for readablility
                      rdindex++;
                    }
                    //re-init 
                    nmindex = 0;
                    num[0] = '\0';

                    if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                    {
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        //second in the line is the chromosone number
                        num[nmindex] = '\0';
                        strcpy_s(snp[inx].ch, num);
                    }
                    else {
                        snp[inx].ch[0] = nbuffer[rdindex]; //X & Y sinle char so why waste a strcpy call!
                        if (snp[inx].ch[0] == 'x') snp[inx].ch[0] = 'X'; //paranoia cass fix
                        if (snp[inx].ch[0] == 'y') snp[inx].ch[0] = 'Y'; //paranoia cass fix
                        snp[inx].ch[1] = '\0';

                    }
                    
                    //move past tab or spaces to next numeric data posotion number
                    while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                    {//wrote whith braces for readablility
                        rdindex++;
                    }
                    //read position
                        //re-init 
                    nmindex = 0;
                    num[0] = '\0';
                    while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = '\0';
                    //second in the line is the chromosone number
                    snp[inx].pos = atoi(num);
     

                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                    {
                        rdindex++;
                    }
                    snp[inx].a = nbuffer[rdindex];
                    //make '0' our standard for noread
                    if (snp[inx].a == '-') snp[inx].a = '0';
                    rdindex++;
                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                    {
                        rdindex++;
                    }
                    //Is Char
                    snp[inx].b = nbuffer[rdindex];
                    //make '0' our standard for noread
                    if (snp[inx].b == '-') snp[inx].b = '0';
                    //increment the primary index
                     //Move Sex check here
                    if (snp[inx].ch[0] == 'Y' && snp[inx].a != '0') {
                        sex_ = 'M';
                    }
                    inx++;
                    //increment count of lines loaded
                    loadCount_++;

                }
            } //FTDNA do not ref NCBI build
                loopbreak++;
                if (loopbreak == 2000) break;
            }
            fs.close();
        }
        else return false;
    }

    return true;
};
//MergeFTDNA
//FTDNA has VG numbers that may corespond to RS numbers
bool  SnipParser::MergeFTDNA(wchar_t* fi_)
{  
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        int loopbreak = 0;
        int VG = 0; //illumina
        //Open file for read 
        fs.open(fi_, std::ios::in);
        if (fs.is_open()) {
        //Check file was opened  
            int fdind = 0; //ftdna-illumina
            //merge variables
            mergefile_ = 0;
            merged_ = allcecked_ = 0;
            end_index_ = origloadcount_ = loadCount_;
            abortMerge_ = false;
            //merge variables
			//sex_ = 'F'; //Sex already set in original load
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.resize(DNA_SNP_BUFFER_SIZE);
            int inx = loadCount_ + 1, rst, rdindex = 0;  //merge code
            wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            nbuffer[257] = '\0';        //more efficient
            std::string vercheck;       //more efficient
            initMergeCopy();            //create merge subset
            //END:
      
			while (fs.getline(nbuffer, READ_LIMIT) && !abortMerge_) //read a line into a temorary buffer and ensure not aborting merge
            {   //More parseer hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }


                //Reject UTF-8 BOM encoded files
                if (loadCount_ == 0 && (unsigned char)nbuffer[0] == 0xEF && (unsigned char)nbuffer[1] == 0xBB && (unsigned char)nbuffer[2] == 0xBF)
                {
                    fs.close();
                    // Reject UTF-8 BOM encoded files
                    return false;
                }

                fdind = 0;
                //Merge code
                vercheck = nbuffer;
                //Merge code
                int rdindex = 2;
                if (nbuffer[fdind] == '\"') fdind++; //ftdna-illumina
                //GET RS Number numeric part only 

                if (((nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's') || (nbuffer[fdind] == 'V' && nbuffer[fdind + 1] == 'G')) && isdigit((int)nbuffer[fdind + 2]))
                {//ftdna-illumina
                    int ftdna = 0;
                    char num[25];
                    int  nmindex = 0;
                    loopbreak = 0;
                    rdindex += fdind;//ftdna-illumina
                    num[0] = '\0';

                    if (nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's')
                    {
                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //First in the line is the RS number
                        rst = atoi(num);

                    }
                    else
                    {
                        //FTDNA MyHeritage name : VG01S1077  VG Chromosone number S numeric  rs77931234
                        nmindex = 0;
                        if (nbuffer[fdind] == 'V' && nbuffer[fdind + 1] == 'G')
                        {
                            fdind += 2; //Get first chromosone digit
                            num[nmindex] = nbuffer[fdind];
                            nmindex++;
                            fdind++; //Get second chromosone digit
                            num[nmindex] = nbuffer[fdind];
                            nmindex++;
                            fdind++;
                            fdind++; //Skip Alpha char
                        }
                        while (isdigit((int)nbuffer[fdind]) && fdind < 24)
                        {
                            num[nmindex] = nbuffer[fdind];
                            fdind++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        rdindex = fdind;                  //set read index
                        rst = FTDNADecode(num); //ver 0.3 beta separate decode function!
                    }
                    if (rst > 0 && mergeRs(rst, vercheck)) //stop line load of uniterpreted VG codes
                    {
                        snp[inx].rs = rst;
                        //move past tab or spaces to next numeric data chromosone number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nbuffer[rdindex] != 'X' && nbuffer[rdindex] != 'Y' && nbuffer[rdindex] != 'x' && nbuffer[rdindex] != 'y')
                        {//wrote with braces for readablility
                            rdindex++;
                        }
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';

                        if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                        {
                            while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                            {
                                num[nmindex] = nbuffer[rdindex];
                                rdindex++;
                                nmindex++;
                            }
                            //second in the line is the chromosone number
                            num[nmindex] = '\0';
                            strcpy_s(snp[inx].ch, num);
                        }
                        else {
                            snp[inx].ch[0] = nbuffer[rdindex]; //X & Y sinle char so why waste a strcpy call!
                            if (snp[inx].ch[0] == 'x') snp[inx].ch[0] = 'X'; //paranoia cass fix
                            if (snp[inx].ch[0] == 'y') snp[inx].ch[0] = 'Y'; //paranoia cass fix
                            snp[inx].ch[1] = '\0';

                        }

                        //move past tab or spaces to next numeric data posotion number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //read position
                            //re-init 
                        nmindex = 0;
                        num[0] = '\0';
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //second in the line is the chromosone number
                        snp[inx].pos = atoi(num);

                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        snp[inx].a = nbuffer[rdindex];
                        //make '0' our standard for noread
                        if (snp[inx].a == '-') snp[inx].a = '0';
                        rdindex++;
                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        //Is Char
                        snp[inx].b = nbuffer[rdindex];
                        //make '0' our standard for noread
                        if (snp[inx].b == '-') snp[inx].b = '0';
                        //increment the primary index
                        inx++;
                        //increment count of lines loaded
                        loadCount_++;

                    }
                } //FTDNA do not ref NCBI build
                loopbreak++;
                if (loopbreak == 2000 || loadCount_ == (DNA_SNP_BUFFER_SIZE - (origloadcount_ + 1))) break;
            }
            fs.close();
        }
        else return false;
    }

    return true;
};

//23andMe function
//has VG numbers that may corespond to RS numbers
bool  SnipParser::f23andMe(wchar_t* fi_)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        int loopbreak = 0;
        bool singleAllele;
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;
            singleAllele = false;
            //START: reset loadcount_ and vector for next file for next file
            loadCount_ = 0;
            sex_ = 'F'; //Set sex to female will change if 'Y' is found!
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.clear();
            snp.resize(DNA_SNP_BUFFER_SIZE);
            wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            //END: reset loadcount_ and vector for next file for next file
            while (fs.getline(nbuffer, READ_LIMIT))//read a line into a temorary buffer
            {   //More parser hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }

                //Reject UTF-8 BOM encoded files
                if (loadCount_ == 0 && (unsigned char)nbuffer[0] == 0xEF && (unsigned char)nbuffer[1] == 0xBB && (unsigned char)nbuffer[2] == 0xBF)
                {
                    fs.close();
                    // Reject UTF-8 BOM encoded files
                    return false;
                }
                int rdindex = 0;
                //GET RS Number numeric part only 
                if (((nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's') || (nbuffer[rdindex] == 'i' && isdigit((int)nbuffer[rdindex + 1]) )) && isdigit((int)nbuffer[rdindex + 2]))
                {//ftdna-illumina
					char num[25];//fixed size 11/12/2025
                    int  nmindex = 0;
                    loopbreak = 0;
                    num[0] = '\0';

                    if (nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's')
                    {
                        rdindex += 2;
                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //First in the line is the RS number
                        snp[inx].rs = atoi(num);

                    }
                    else
                    {
                        //23toMe ixxxx codes ro rsxxx codes
                        int isw = 0;
                        nmindex = 0;
                        rdindex += 1; //Skip i
                                               
                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                       
                        snp[inx].rs = f23andMeDecode(num);//in example  VG01S1077 ftdna will equal numeric 11007
                    }

                    if (snp[inx].rs > 0) //stop line load of uniterpreted i codes
                    {
                        //move past tab or spaces to next numeric data chromosone number below bug fix 3/11/21
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nbuffer[rdindex] != 'X' && nbuffer[rdindex] != 'Y' && nbuffer[rdindex] != 'x' && nbuffer[rdindex] != 'y' && nbuffer[rdindex] != 'm' && nbuffer[rdindex] != 'M')
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';

                        if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                        {
                            while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                            {
                                num[nmindex] = nbuffer[rdindex];
                                rdindex++;
                                nmindex++;
                            }
                            //second in the line is the chromosone number
                            num[nmindex] = '\0';
                            strcpy_s(snp[inx].ch, num);
                        }
                        else {//Bug fix 3/10/21
                               if((nbuffer[rdindex] == 'M' && nbuffer[rdindex + 1] == 'T') || (nbuffer[rdindex] == 'm' && nbuffer[rdindex + 1] == 't'))
                                 {//know its uppercase but others use the format
                                  snp[inx].ch[0] = 'M';
                                  snp[inx].ch[1] = 'T';
                                  snp[inx].ch[2] = '\0';  //Bug fix 3/11/21
                                  singleAllele = true;
                                 } else
                                       {
                                        snp[inx].ch[0] = nbuffer[rdindex]; 
                                        if (snp[inx].ch[0] == 'x') snp[inx].ch[0] = 'X'; //paranoia cass fix
                                        if (snp[inx].ch[0] == 'y') snp[inx].ch[0] = 'Y'; //paranoia cass fix
                                        snp[inx].ch[1] = '\0';
                                        singleAllele = true;
                                       }

                        }

                        //move past tab or spaces to next numeric data posotion number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //read position
                            //re-init 
                        nmindex = 0;
                        num[0] = '\0';
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //second in the line is the chromosone number
                        snp[inx].pos = atoi(num);


                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        snp[inx].a = nbuffer[rdindex];
                        //make '0' our standard for noread
                        if (snp[inx].a == '-') snp[inx].a = '0';
                        rdindex++;
                        if (singleAllele)
                        {
                            snp[inx].b = '\0';
                        }
                        else
                        {
                            //As the last two values are not numeric we have to rely on the file still being TAB delimited
                            while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                                && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                            {
                                rdindex++;
                            }
                            //Is Char
                            snp[inx].b = nbuffer[rdindex];
                            //make '0' our standard for noread
                            if (snp[inx].b == '-') snp[inx].b = '0';
                        }
                        //Move Sex check here
                        if (snp[inx].ch[0] == 'Y' && snp[inx].a != '0') {
                            sex_ = 'M';
                        }
                        //increment the primary index
                        inx++;
                        //increment count of lines loaded
                        loadCount_++;

                    }
                }
                //does not ref ncbi build
                loopbreak++;
                if (loopbreak == 2000 || loadCount_ == (DNA_SNP_BUFFER_SIZE - (origloadcount_ + 1))) break;
            }
            fs.close();
        }
        else return false;
    }
    return true;
};
//Merge 23toMe
//23andMe function
//has VG numbers that may corespond to RS numbers
bool  SnipParser::Mergef23andMe(wchar_t* fi_)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        int loopbreak = 0;
        //merge variables
        mergefile_ = 0;
        merged_ = allcecked_ = 0;
        end_index_ = origloadcount_ = loadCount_;
        //merge variables
        bool singleAllele;
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            singleAllele = false;
			// sex_ = 'F'; //Wrong you are merging you alrady know the Sex of the subject
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.resize(DNA_SNP_BUFFER_SIZE);
            //NO!!!: reset loadcount_ and vector for next file for next file
            int inx = loadCount_ + 1, rst, rdindex = 0;  //merge code
            wcscpy_s(fileLoaded_, _countof(fileLoaded_), fi_); //store latest filename. Fixed for correct defined usage 1/24/2026
            nbuffer[257] = '\0';        //more efficient
            std::string vercheck;       //more efficient
            initMergeCopy();            //create merge subset
            //END:
			while (fs.getline(nbuffer, READ_LIMIT) && !abortMerge_)//read a line into a temorary buffer and ensure not aborting merge
            {   //More parser hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }
                //Merge code
                vercheck = nbuffer;
                //Merge code
                int rdindex = 0;
                //GET RS Number numeric part only 
                if (((nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's') || (nbuffer[rdindex] == 'i' && isdigit((int)nbuffer[rdindex + 1]))) && isdigit((int)nbuffer[rdindex + 2]))
                {//ftdna-illumina
                    char num[25];
                    int  nmindex = 0;
                    loopbreak = 0;
                    num[0] = '\0';

                    if (nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's')
                    {
                        rdindex += 2;
                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //First in the line is the RS number
                         rst = atoi(num);

                    }
                    else
                    {
                        //23toMe ixxxx codes ro rsxxx codes
                        int isw = 0;
                        nmindex = 0;
                        rdindex += 1; //Skip i

                        while (isdigit((unsigned char)nbuffer[rdindex]) && rdindex < sizeof(nbuffer) && nmindex < sizeof(num) - 1)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';

                        rst = f23andMeDecode(num);//in example  VG01S1077 ftdna will equal numeric 11007
                    }

                    if (rst > 0 && mergeRs(rst, vercheck)) //stop line load of uniterpreted i codes
                    {
                        snp[inx].rs = rst;
                        //move past tab or spaces to next numeric data chromosone number below bug fix 3/11/21
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nbuffer[rdindex] != 'X' && nbuffer[rdindex] != 'Y' && nbuffer[rdindex] != 'x' && nbuffer[rdindex] != 'y' && nbuffer[rdindex] != 'm' && nbuffer[rdindex] != 'M')
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';

                        if (isdigit((int)nbuffer[rdindex])) //if autosomal chr
                        {
                            while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT && nmindex < 3)//last clause is to prevent overflow on corrupt file
                            {
                                num[nmindex] = nbuffer[rdindex];
                                rdindex++;
                                nmindex++;
                            }
                            //second in the line is the chromosone number
                            num[nmindex] = '\0';
                            strcpy_s(snp[inx].ch, num);
                        }
                        else {//Bug fix 3/10/21
                            if ((nbuffer[rdindex] == 'M' && nbuffer[rdindex + 1] == 'T') || (nbuffer[rdindex] == 'm' && nbuffer[rdindex + 1] == 't'))
                            {//know its uppercase but others use the format
                                snp[inx].ch[0] = 'M';
                                snp[inx].ch[1] = 'T';
                                snp[inx].ch[2] = '\0';  //Bug fix 3/11/21
                                singleAllele = true;
                            }
                            else
                            {
                                snp[inx].ch[0] = nbuffer[rdindex];
                                if (snp[inx].ch[0] == 'x') snp[inx].ch[0] = 'X'; //paranoia cass fix
                                if (snp[inx].ch[0] == 'y') snp[inx].ch[0] = 'Y'; //paranoia cass fix
                                snp[inx].ch[1] = '\0';
                                singleAllele = true;
                            }

                        }

                        //move past tab or spaces to next numeric data posotion number
                        while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {//wrote whith braces for readablility
                            rdindex++;
                        }
                        //read position
                        //re-init 
                        nmindex = 0;
                        num[0] = '\0';
                        while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = '\0';
                        //second in the line is the chromosone number
                        snp[inx].pos = atoi(num);

                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                        {
                            rdindex++;
                        }
                        snp[inx].a = nbuffer[rdindex];
                        //make '0' our standard for noread
                        if (snp[inx].a == '-') snp[inx].a = '0';
                        rdindex++;
                        if (singleAllele)
                        {
                            snp[inx].b = '\0';
                        }
                        else
                        {
                            //As the last two values are not numeric we have to rely on the file still being TAB delimited
                            while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                                && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < READ_LIMIT)
                            {
                                rdindex++;
                            }
                            //Is Char
                            snp[inx].b = nbuffer[rdindex];
                            //make '0' our standard for noread
                            if (snp[inx].b == '-') snp[inx].b = '0';
                        }
                        //increment the primary index
                        inx++;
                        //increment count of lines loaded
                        loadCount_++;
                    }
                }
                //does not ref ncbi build
                loopbreak++;
                if (loopbreak == 2000) break;
            }
            fs.close();
        }
        else return false;
    }
    return true;
};

//Return the number of translated VG codes
unsigned int  SnipParser::IllumTransVG(void)
{
    return illuminaT_;
}
//Return the number of untranslated VG codes
unsigned int  SnipParser::IllumUntransVG(void)
{
    return illuminaU_;
}
//return the number of lines loaded
int SnipParser::SNPCount(void)
{
    return loadCount_;
}
//return sex
wchar_t SnipParser::sex(void)
{
    return sex_;
}
unsigned int SnipParser::merged(void)
{
    return merged_;
}

//make merge copy
void SnipParser::initMergeCopy(void)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    if (loadCount_ > 0) {//Code creates a copy of existing SNP data into snpM
        // Ensure snpM is large enough for existing data + some headroom
        unsigned int newSize = loadCount_ + 16;
        snpM.resize(newSize);

        // Copy existing SNP data
        for (unsigned int i = 0; i < loadCount_; i++) {
            snpM[i].rs = snp[i].rs;
            snpM[i].a = snp[i].a;
            snpM[i].b = snp[i].b;
        }

        // Initialize the extra capacity to safe defaults
        for (unsigned int i = loadCount_; i < newSize; i++) {
            snpM[i].rs = 0;
            snpM[i].a = '0';
            snpM[i].b = '0';
        }
    }
    else {
        // No data loaded - create empty buffer
        snpM.clear();
        snpM.resize(0);  // Or just leave it empty
    }
}

//returns if merge failed
bool SnipParser::MergeState(void)
{
    return abortMerge_;
}
/*Revert a failed merge*/
void SnipParser::revertMerge(void)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    //Reset rsid to '\0'
    for (unsigned int i = 1 + origloadcount_; i < loadCount_; i++)
    {
        snp[i].rs = '\0';
    }
    //reset load count
    loadCount_ = origloadcount_;

}
unsigned int SnipParser::MergeProcessed(void)
{
    return(allcecked_); //Return Merged FIXED 12/31/2025
}

/*Check the subject is the same to prevent the generation of garbage genetic files
Even though this is the most effiecent self resizing inlined loop I could write
the shear amount of comparisons involved in unsorted data makes this a slow job!! */
bool SnipParser::mergeRs(int code, const std::string& line) {
   std::lock_guard<std::mutex> lock(merge_mutex_);

    if (abortMerge_) return false;
    // SAFETY CHECK 1
	if (end_index_ == 0 || end_index_ >= DNA_SNP_BUFFER_SIZE) {//fixed bug 1/25/2026
        return true;  // Nothing to search or invalid
    }
    // SAFETY CHECK 2 - Use < not <=
    for (unsigned int i = 0; i < end_index_; ++i) {
        if (snpM[i].rs == code) //Does the code exist in the original dataset
        {//yes we already have it!
            allcecked_++;

            if (snpM[i].a == '0') return true;

            // ---- NEW: BACKWARDS SEARCH ----
            // Start from end of line
            const char* lineEnd = line.c_str() + line.length();
			const char* ptr = lineEnd - 1;//point to last char

            // Skip trailing whitespace
            while (ptr >= line.c_str() && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')) {
                ptr--;
            }

            // Check second allele FIRST (it's last in string)
            bool foundB = false;
            if (snpM[i].b != '-' && snpM[i].b != '0') {
                // Search backwards for second allele
                const char* searchPtr = ptr;
                while (searchPtr >= line.c_str()) {
                    if (*searchPtr == snpM[i].b) {
                        foundB = true;
                        ptr = searchPtr - 1;  // Move before found allele
                        break;
                    }
                    searchPtr--;
                }
                if (!foundB) failcheck_++;
            }
            else {
                // No second allele (male X chromosome, etc.)
                foundB = true;  // Consider it "found"
            }

            // Check first allele (search in remaining part)
            bool foundA = false;
            if (foundB || snpM[i].b == '-' || snpM[i].b == '0') {
                // Skip any whitespace between alleles
                while (ptr >= line.c_str() && (*ptr == ' ' || *ptr == '\t')) {
                    ptr--;
                }

                // Search for first allele
                const char* searchPtr = ptr;
                while (searchPtr >= line.c_str()) {
                    if (*searchPtr == snpM[i].a) {
                        foundA = true;
                        break;
                    }
                    searchPtr--;
                }
                if (!foundA) failcheck_++;
            }
            // ---- END BACKWARDS SEARCH ----

            // Move data (check bounds!)
            if (i != end_index_ - 1) {
                snpM[i] = snpM[end_index_ - 1];
            }
            end_index_--;

            if (allcecked_ > 1250 && ((allcecked_ >> 2) < failcheck_)) {
                abortMerge_ = true;
            }

            return false;
        }
    }
    merged_++;
    return true;
}

/*RS number search function passed the RS number to searcg for
and a structure of type s to place the data in if a match is found */
bool SnipParser::RsSearch(int *rs, char* chr1, char* chr2, char* chr3, char* chr4, int *pos, char *a, char *b)
{
    // Check you have SNP data loaded
	// Check we have SNP data loaded && valid load count
    std::lock_guard<std::mutex> lock(global_mutex_);
    if (loadCount_ > 0 && loadCount_ <= snp.size()) //fixed 1/17/2026
    { 
		//For Sanity check ensure '\0' termination
        
        // Search for RS numer int rs
        for (unsigned int i = 0; i < loadCount_;) //fixed 1/17/2026
        {
            if (snp[i].rs == *rs)
            {
                //match found! populate and return the structure
                snp[i].ch[3] = '\0'; //there should be a '\0' before this in all cases BUT this is the safety to ensure no buffer overrun
                *rs = snp[i].rs;         // RS number
                *chr1 = snp[i].ch[0];    // Chromosone array as string
                *chr2 = snp[i].ch[1];    // Chromosone array as string
                *chr3 = snp[i].ch[2];    // Chromosone array as string
                *chr4 = '\0';            // ALWAYS '\0' made more efficient(sp) 1/17/2026 
                *pos = snp[i].pos;       // Position
                *a = snp[i].a;           // first nucleotide
                *b = snp[i].b;           // first nucleotide
                
                //START: For Ancestory DNA files 23 is the X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.
                if (*chr1 == '2' && *chr2 == '3')
                {
                    *chr1 = 'X';
                    *chr2 = '\0';
                }
                if (*chr1 == '2' && *chr2 == '4')
                {
                    *chr1 = 'Y';
                    *chr2 = '\0';
                }
                if (*chr1 == '2' && *chr2 == '5')
                {
                    *chr1 = 'P';
                    *chr2 = 'A';
                    *chr3 = 'R';
                    *chr4 = '\0';
                }
                if (*chr1 == '2' && *chr2 == '6')
                {
                    *chr1 = 'm';
                    *chr2 = 't';
                    *chr3 = '\0';
                }
                //END: For Ancestory DNA files 23 is the X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.
                return true;
            }
            //inceament loop
            i++;
        }

    }
    return false;
}

//return version number
std::string SnipParser::PVer(void)
{
    return  Pversion_;
}

std::string SnipParser::PAbout(void)
{
    return PAbouttxt_;
}
/*return the NCBI Build the file was based on gives position a referance
  as well as being a clue to the age of the files contents */
std::string SnipParser::NCBIBuild(void)
{
    return NCBIBuild_;
}
/*Major Work in porgress*/
bool  SnipParser::AncestoryWriter(wchar_t* fi_)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    std::fstream  fs;
     {
         char timebufd[128], timebuft[128];
         std::string  lbuffer;
         std::time_t t = std::time(nullptr);
         int loopbreak = 0;
         // Convert the wchar_t string to a char* string. Record
         // the length of the original string and add 1 to it to
         // account for the terminating null character.
         size_t origsize = wcslen(fileLoaded_) + 1;
         size_t convertedChars = 0;
        //MS example code
         char strConcat[] = " (char *)";
         size_t strConcatsize = (strlen(strConcat) + 1) * 2;
         const size_t newsize = origsize * 2;
		 char* nstring = new char[newsize + strConcatsize](); //Wasn't being deleted 11/12/2025 delete[] nstring
         if(wcstombs_s(&convertedChars, nstring, newsize, fileLoaded_, _TRUNCATE) != 0) {
             delete[] nstring;
             return false;
            }
          //Open file for write 
         fs.open(fi_, std::ios::out);
         //Check file was opened  
         if (fs.is_open()) {
             int unsigned inx = 0;
             //Start:Write Header
             _tzset(); //set timezone
             _strtime_s(timebuft, 128);
             _strdate_s(timebufd, 128);
             /*Write an infomative header close to the orgiginal ver 1,0 ancestry header but without the ref genome as it will change file to file
             and with the program name ver and file creation date as well as the original input file name and path*/
             lbuffer = "#File Format AncestryDNA(c) raw data download\n";
             lbuffer += "#This file was generated by ParseSNP version: " + Pversion_ + " at " + timebufd + " " + timebuft + "\n";
             lbuffer += "#Forth line down is that below which some progarms may read to get format verion!!! Futureproofing\n";
             lbuffer += "#Data is formatted using AncestryDNA converter version: V1.0\n";
             lbuffer += "#Original rsID datafile: ";
             lbuffer += nstring;
             lbuffer += "\n";
             lbuffer += "Ancestory DNA  23 is X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.";
             lbuffer += "#Genetic data is provided below as five TAB delimited columns.\n" ;
             lbuffer += "#Each line corresponds to a SNP.Column one provides the SNP identifier rsID.\n";
             lbuffer += "#Columns two and three contain the chromosome and basepair position\n";
             lbuffer += "#of the SNP using a human reference build coordinates.Columns four and five\n";
             lbuffer += "#contain the two alleles observed at this SNP(genotype).The genotype is reported\n";
             lbuffer += "#on the forward(+) strand with respect to the human reference.\n";
             lbuffer += "   rsID    Chromosome  Position    Allele1 Allele2\n";
             const char* write_it = lbuffer.c_str();
             fs.write(write_it, lbuffer.length());
             //End:Write Header
             char c_num[50] = {'0'};//Added 0 padding as NULL terminator
             bool over22;
             std::string rsID, Position, Chromosome,allele1,allele2;
             while (inx < loadCount_)
             {   //For Ancestory DNA files 23 is the X, 24 is Y, 25 is the (Pseudoautosomal region) PAR region, and 26 is mtDNA.
                 over22 = false;
                 if (_itoa_s(snp[inx].rs, c_num, 10) == 0) rsID = (std::string) c_num;  //warning C6054 ignore _itoa_s returns a NULL terminated sting fix
                 else break;
                 //convert 23toMe
                 if (!strcmp(snp[inx].ch, "MT")) {
                     Chromosome = (std::string)"26";
                     over22 = true;
                 }  else
                     if (!strcmp(snp[inx].ch, "X")) {
                         Chromosome = (std::string)"23";
                         over22 = true;
                     } else if (!strcmp(snp[inx].ch, "Y")) {
                         Chromosome = (std::string)"24";
                         over22 = true;
                     } else                 
                        {   
                          Chromosome = (std::string)(snp[inx].ch);
                         }
                    
					 if (_itoa_s(snp[inx].pos, c_num, 10) == 0) Position = c_num; //fix 1/25/2026
                      else break;
                 if (over22)
                 { //ancestory's wierd to bases for X Y and MT where there is only 1
                   // '-' as been converted on load but I am leaving the '-' option for any future code that the conversion is omitted  
                     if (snp[inx].a == '-' || snp[inx].a == '0')
                     {
                         allele1 = allele2 = (std::string)"0";
                     }
                     else
                     {
                         allele1 = allele2 = snp[inx].a + '\0';
                     }
                 }
                 else {
                        if (snp[inx].a == '-' || snp[inx].a == '0') allele1 = (std::string)"0";
                             else allele1 = snp[inx].a + '\0';
                        if (snp[inx].b == '-' || snp[inx].b == '0') allele2 = (std::string)"0";
                             else allele2 = snp[inx].b + '\0';
                      }

                 lbuffer = "rs" + rsID + "\t" + Chromosome + "\t" + Position + "\t" + allele1 + "\t" + allele2 + "\n";
                 //write the line
                 const char* write_it = lbuffer.c_str();
                 fs.write(write_it, lbuffer.length());
                 inx++; //increment inx to the next values / line
             }
			 delete[] nstring;//11/12/2025
             fs.close();
             //got through the loop assuming file wrote correctly
             return true;
		 }
		 else {//Delete allocated memory
                 delete[] nstring;//11/12/2025
                 return false;
                }
     }
     return false;
}
//INTERNAL code generator not of use 
void  SnipParser::FConvert(void)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    wchar_t  codeIn_[] = L"f:\\icode.txt";
    wchar_t  codeOut_[] = L"f:\\ccode.txt";

    std::fstream  fs;
    std::fstream  fsOut;
    {
        char nbuffer[TOTAL_BUFFER_SIZE];
        std::string  lbuffer;
        char num[100];
        int loopbreak = 0;
        int fdind = 0;
        //Open file for read 
        fs.open(codeIn_, std::ios::in);
        //Open file for write
        fsOut.open(codeOut_, std::ios::out);
        //Check file was opened  
        if (fs.is_open() && fsOut.is_open()) {
            fdind = 0;
            while (fs.getline(nbuffer, READ_LIMIT))
            {   //More parser hardening
                if (fs.fail() && !fs.eof())
                {
                    // Line was too long — discard remainder
                    fs.clear();
                    fs.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    continue;
                }
                int rdindex = 0;
                int nmindex = 0;
                //move past tab or spaces to next numeric data posotion number
                while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                {//wrote with braces for readablility
                    rdindex++;
                }
                //read position
                //re-init 
                nmindex = 0;
                num[0] = '\0';
                while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                {
                    num[nmindex] = nbuffer[rdindex];
                    rdindex++;
                    nmindex++;
                }
                num[nmindex] = '\0';
                //START constructing the write line
                lbuffer = "case " + std::string(num) + ":" + char(13) + "  snp[inx].rs = ";
                //read second rsID number
                //move past tab or spaces to next numeric data position number
                while (!isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                {//wrote whith braces for readablility
                    rdindex++;
                }
                //read position
                //re-init 
                nmindex = 0;
                num[0] = '\0';
                while (isdigit((int)nbuffer[rdindex]) && rdindex < READ_LIMIT)
                {
                    num[nmindex] = nbuffer[rdindex];
                    rdindex++;
                    nmindex++;
                }
                num[nmindex] = '\0';
                //lbuffer= lbuffer + +=
                lbuffer += std::string(num) + ";" + char(13) +"  illuminaT_++;" + char(13) + "  break;" + char(13) ;
                const char* write_it = lbuffer.c_str();
                fsOut.write(write_it , lbuffer.length());
            }
        }
        fs.close();
        fsOut.close();
    }
}  
/* Searches all loaded data for matching rsID then checks for risk  allele.
If it is found it updates the odds ratio                       */
std::string SnipParser::PathogenicCall(int rsid, char riskallele, float oddsratio, float* sumoddsratio)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    float denominator = 0.0; //to prevent div by zero
    std::string result_message = "N/A";

    // Check you have SNP data loaded
    if (loadCount_ > 0)
    {
        // Search for RS numer int rs
        for (unsigned int i = 0; i <= loadCount_;)
        {
            if (snp[i].rs == rsid)
            {    
                char buffer[8];
                char check_Y[4];
                
                if (!strcmp(snp[i].ch, "23"))
                {
                    check_Y[0] = 'X';
                    check_Y[1] = '\0';
                }
                else
                {
                    check_Y[0] = snp[i].ch[0];
                    check_Y[1] = snp[i].ch[1];
                    check_Y[2] = snp[i].ch[2];
                    check_Y[3] = snp[i].ch[3];
                }
                buffer[0] = '[';
                buffer[1] = (char)snp[i].a;
                buffer[2] = '/';
                buffer[3] = (char)snp[i].b;
                buffer[4] = ']';
                buffer[5] = ' ';
                buffer[6] = '\0';
                result_message = buffer;
                //fixed logic in ver 0.9 Beta
				denominator = (float)((*sumoddsratio + 1) * (oddsratio + 1) - ((*sumoddsratio * oddsratio))); //protect from diversion by zero
                //Match of risk  allele on both chromosones!
                if (snp[i].a == riskallele && snp[i].b == riskallele)
                {
                    if (check_Y[0] == 'X' && sex_ == 'M') 
                    {
                        buffer[3] = '-';
                        result_message = buffer;
                        result_message += "Risk Heterozygous";
                        if (*sumoddsratio == 0.0) *sumoddsratio = oddsratio;
                        else if (denominator > 0.0) *sumoddsratio += (float)(*sumoddsratio * oddsratio) / denominator;
                        return result_message;
                    }
                    result_message += "Risk Homozygous!";
                    //from a post on statistics here https://stats.stackexchange.com/questions/187107/can-you-add-up-different-genes-odds-ratios-to-get-a-general-odds-ratio
					if (*sumoddsratio == 0.0) *sumoddsratio = oddsratio * (float)2.0; //for homozygous fix logic ver 0.9 Beta
					else  if (denominator > 0.0) *sumoddsratio += (float)((*sumoddsratio * oddsratio) / denominator) * (float)2.0; //for homozygous fix logic ver 0.9 Beta
                    return result_message;
                }
                //Match in one chromosone
                if (snp[i].a == riskallele || snp[i].b == riskallele)
                {
                    result_message += "Risk Heterozygous";
                    if (*sumoddsratio == 0.0) *sumoddsratio = oddsratio;
                    else if (denominator > 0.0) *sumoddsratio += (float)(*sumoddsratio * oddsratio) / denominator;
                    return result_message;
                }
                if (snp[i].a == '0' && snp[i].b == '0')  result_message += "N/A (No Read)";
                else result_message += "No Risk";
                return result_message;
            }
            //inceament loop
            i++;
        }

    }
    return result_message;
}
int SnipParser::FTDNADecode(std::string code)
{
    std::lock_guard<std::mutex> lock(global_mutex_);
    int rs = 0,num;

    num = atoi(code.c_str()); //in example  VG01S1077 ftdna will equal numeric 11007
    switch (num) {//rs equivelent ALL Illumina VG codes seem to realte to Cystic Fibrosis!
    case 11077:
        rs = 77931234;
        illuminaT_++;
        break;
    case 12269:
        rs = 77931234;
        illuminaT_++;
        break;
    case 11161:
        rs = 78396650;
        illuminaT_++;
        break;
    case 11154:
        rs = 74500255;
        illuminaT_++;
        break;
    case 11135:
        rs = 77829017;
        illuminaT_++;
        break;
    case 425864:
        rs = 75790268;
        break;
    case 526940:
        rs = 79967166;
        illuminaT_++;
        break;
    case 729384:
        rs = 77284892;
        illuminaT_++;
        break;
    case 729458:
        rs = 75961395;
        break;
    case 729628:
        rs = 78655421;
        illuminaT_++;
        break;
    case 744961:
        rs = 78756941;
        illuminaT_++;
        break;
    case 744986:
        rs = 77188391;
        illuminaT_++;
        break;
    case 745007:
        rs = 78440224;
        illuminaT_++;
        break;
    case 752452:
        rs = 77409459;
        illuminaT_++;
        break;
    case 729244:
        rs = 77409459;
        illuminaT_++;
        break;
    case 729268:
        rs = 79282516;
        illuminaT_++;
        break;
    case 745090:
        rs = 76713772;
        illuminaT_++;
        break;
    case 729293:
        rs = 75527207;
        illuminaT_++;
        break;
    case 729297:
        rs = 74597325;
        illuminaT_++;
        break;
    case 729341:
        rs = 75115087;
        illuminaT_++;
        break;
    case 752241:
        rs = 121908787;
        illuminaT_++;
        break;
    case 729361:
        rs = 79633941;
        illuminaT_++;
        break;
    case 750448:
        rs = 121908767;
        illuminaT_++;
        break;
    case 745132:
        rs = 76151804;
        illuminaT_++;
        break;
    case 729391:
        rs = 78194216;
        illuminaT_++;
        break;
    case 729406:
        rs = 78802634;
        illuminaT_++;
        break;
    case 729419:
        rs = 75541969;
        illuminaT_++;
        break;
    case 729422:
        rs = 79850223;
        illuminaT_++;
        break;
    case 729424:
        rs = 74767530;
        illuminaT_++;
        break;
    case 729435:
        rs = 75389940;
        illuminaT_++;
        break;
    case 752449:
        rs = 75039782;
        illuminaT_++;
        break;
    case 729444:
        rs = 74503330;
        illuminaT_++;
        break;
    case 729451:
        rs = 77010898;
        illuminaT_++;
        break;
    case 932163:
        rs = 77375493;
        illuminaT_++;
        break;
    case 931849:
        rs = 79833450;
        illuminaT_++;
        break;
    case 931531:
        rs = 77718928;
        illuminaT_++;
        break;
    case 952451:
        rs = 78340951;
        illuminaT_++;
        break;
    case 115372:
        rs = 33951465;
        illuminaT_++;
        break;
    case 128248:
        rs = 62644499;
        illuminaT_++;
        break;
    case 128239:
        rs = 5030859;
        illuminaT_++;
        break;
    case 128233:
        rs = 5030857;
        illuminaT_++;
        break;
    case 128222:
        rs = 5030856;
        illuminaT_++;
        break;
    case 128220:
        rs = 62516101;
        break;
    case 128217:
        rs = 62642937;
        illuminaT_++;
        break;
    case 128204:
        rs = 62516095;
        illuminaT_++;
        break;
    case 128194:
        rs = 62508646;
        illuminaT_++;
        break;
    case 128480:
        rs = 5030853;
        illuminaT_++;
        break;
    case 128479:
        rs = 62642933;
        illuminaT_++;
        break;
    case 128467:
        rs = 5030851;
        illuminaT_++;
        break;
    case 128449:
        rs = 62514952;
        illuminaT_++;
        break;
    case 128434:
        rs = 5030850;
        illuminaT_++;
        break;
    case 128424:
        rs = 62644503;
        illuminaT_++;
        break;
    case 128405:
        rs = 5030846;
        illuminaT_++;
        break;
    case 128402:
        rs = 62508730;
        illuminaT_++;
        break;
    case 128320:
        rs = 5030843;
        illuminaT_++;
        break;
    case 128298:
        rs = 76296470;
        illuminaT_++;
        break;
    case 128296:
        rs = 62642929;
        illuminaT_++;
        break;
    case 128223:
        rs = 62642926;
        illuminaT_++;
        break;
    case 1352444:
        rs = 76151636;
        illuminaT_++;
        break;
    case 1512228:
        rs = 78310315;
        illuminaT_++;
        break;
    case 1716802:
        rs = 28940574;
        illuminaT_++;
        break;
    case 2123270:
        rs = 5742905;
        illuminaT_++;
        break;
		//V4 codes thanks to DeepSeek Python script!
    case 3000001:
        rs = 113993960;  // RS113993960
        illuminaT_++;
        break;
    case 3000014:
        rs = 372706460;  // RS372706460
        illuminaT_++;
        break;
    case 3000029:
        rs = 375151448;  // RS375151448
        illuminaT_++;
        break;
    case 3000037:
        rs = 796993763;  // RS796993763
        illuminaT_++;
        break;
    case 3000065:
        rs = 796727509;  // RS796727509
        illuminaT_++;
        break;
    case 3000078:
        rs = 372362604;  // RS372362604
        illuminaT_++;
        break;
    case 3000206:
        rs = 17215500;  // RS17215500
        illuminaT_++;
        break;
    case 3000564:
        rs = 41464546;  // RS41464546
        illuminaT_++;
        break;
    case 3000954:
        rs = 28358282;  // RS28358282
        illuminaT_++;
        break;
    case 3001096:
        rs = 118203888;  // RS118203888
        illuminaT_++;
        break;
    case 3001474:
        rs = 3021088;  // RS3021088
        illuminaT_++;
        break;
    case 3002432:
        rs = 1799963;  // RS1799963
        illuminaT_++;
        break;
    case 3002449:
        rs = 1800123;  // RS1800123
        illuminaT_++;
        break;
    case 3002455:
        rs = 1800386;  // RS1800386
        illuminaT_++;
        break;
    case 3002468:
        rs = 1800730;  // RS1800730
        illuminaT_++;
        break;
    case 3002484:
        rs = 1801086;  // RS1801086
        illuminaT_++;
        break;
    case 3002485:
        rs = 1801166;  // RS1801166
        illuminaT_++;
        break;
    case 3002486:
        rs = 1801175;  // RS1801175
        illuminaT_++;
        break;
    case 3002507:
        rs = 1805009;  // RS1805009
        illuminaT_++;
        break;
    case 3002517:
        rs = 1805137;  // RS1805137
        illuminaT_++;
        break;
    case 3002721:
        rs = 121917808;  // RS121917808
        illuminaT_++;
        break;
    case 3002758:
        rs = 28933979;  // RS28933979
        illuminaT_++;
        break;
    case 3002759:
        rs = 76992529;  // RS76992529
        illuminaT_++;
        break;
    case 3002773:
        rs = 118192163;  // RS118192163
        illuminaT_++;
        break;
    case 3002796:
        rs = 104894724;  // RS104894724
        illuminaT_++;
        break;
    case 3002797:
        rs = 121964895;  // RS121964895
        illuminaT_++;
        break;
    case 3002808:
        rs = 79761867;  // RS79761867
        illuminaT_++;
        break;
    case 3003043:
        rs = 28940285;  // RS28940285
        illuminaT_++;
        break;
    case 3003074:
        rs = 28941776;  // RS28941776
        illuminaT_++;
        break;
    case 3003090:
        rs = 28942081;  // RS28942081
        illuminaT_++;
        break;
    case 3003093:
        rs = 28942084;  // RS28942084
        illuminaT_++;
        break;
    case 3003094:
        rs = 28942085;  // RS28942085
        illuminaT_++;
        break;
    case 3003391:
        rs = 5030821;  // RS5030821
        illuminaT_++;
        break;
    case 3003392:
        rs = 5030823;  // RS5030823
        illuminaT_++;
        break;
    case 3003397:
        rs = 5030843;  // RS5030843
        illuminaT_++;
        break;
    case 3003398:
        rs = 5030846;  // RS5030846
        illuminaT_++;
        break;
    case 3003399:
        rs = 5030847;  // RS5030847
        illuminaT_++;
        break;
    case 3003400:
        rs = 5030850;  // RS5030850
        illuminaT_++;
        break;
    case 3003401:
        rs = 5030851;  // RS5030851
        illuminaT_++;
        break;
    case 3003403:
        rs = 5030856;  // RS5030856
        illuminaT_++;
        break;
    case 3003404:
        rs = 5030859;  // RS5030859
        illuminaT_++;
        break;
    case 3003405:
        rs = 5030860;  // RS5030860
        illuminaT_++;
        break;
    case 3003411:
        rs = 5030869;  // RS5030869
        illuminaT_++;
        break;
    case 4000101:
        rs = 796483281;  // RS796483281
        illuminaT_++;
        break;
    case 4000146:
        rs = 796827870;  // RS796827870
        illuminaT_++;
        break;
    case 4000163:
        rs = 773700419;  // RS773700419
        illuminaT_++;
        break;
    case 4000164:
        rs = 368154770;  // RS368154770
        illuminaT_++;
        break;
    case 4000166:
        rs = 375296284;  // RS375296284
        illuminaT_++;
        break;
    case 4000171:
        rs = 368057017;  // RS368057017
        illuminaT_++;
        break;
    case 4000178:
        rs = 112707890;  // RS112707890
        illuminaT_++;
        break;
    case 4000186:
        rs = 113822196;  // RS113822196
        illuminaT_++;
        break;
    case 4000189:
        rs = 113686221;  // RS113686221
        illuminaT_++;
        break;
    case 4000291:
        rs = 74551128;  // RS74551128
        illuminaT_++;
        break;
    case 4000292:
        rs = 121908745;  // RS121908745
        illuminaT_++;
        break;
    case 4000294:
        rs = 75961395;  // RS75961395
        illuminaT_++;
        break;
    case 4000295:
        rs = 78655421;  // RS78655421
        illuminaT_++;
        break;
    case 4000296:
        rs = 121909011;  // RS121909011
        illuminaT_++;
        break;
    case 4000297:
        rs = 77932196;  // RS77932196
        illuminaT_++;
        break;
    case 4000299:
        rs = 77646904;  // RS77646904
		illuminaT_++;
        break;
    case 4000300:
        rs = 113993959;  // RS113993959
        illuminaT_++;
        break;
    case 4000301:
        rs = 121908755;  // RS121908755
        illuminaT_++;
        break;
    case 4000302:
        rs = 121909005;  // RS121909005
        illuminaT_++;
        break;
    case 4000305:
        rs = 75527207;  // RS75527207
        illuminaT_++;
        break;
    case 4000306:
        rs = 74597325;  // RS74597325
        illuminaT_++;
        break;
    case 4000307:
        rs = 80055610;  // RS80055610
        illuminaT_++;
        break;
    case 4000308:
        rs = 74767530;  // RS74767530
        illuminaT_++;
        break;
    case 4000309:
        rs = 77010898;  // RS77010898
        illuminaT_++;
        break;
    case 4000311:
        rs = 80034486;  // RS80034486
        illuminaT_++;
        break;
    case 4000313:
        rs = 121908769;  // RS121908769
        illuminaT_++;
        break;

    default:illuminaU_++; //untranslaned lines
        break;
    }
    return rs;
}
int SnipParser::f23andMeDecode(std::string code)
{ 
    std::lock_guard<std::mutex> lock(global_mutex_);
    int num, rs = 0;

    num = atoi(code.c_str());
    switch (num) {//convert Illumina Internal codes to rsID equivelents
    case 3003626:  //CCR5 delta 32 mutation. imparts HIV1 immunity if Hetrozogous (but makes you more vunerable to flaviviruses and no immunity to HIV2
        rs = 333;
        illuminaT_++;
        break;    // All codes beyond this point are ver. 5 codes 
    case 4000478:
        rs = 118203925;
        illuminaT_++;
        break;
    case 4000479:
        rs = 62516095;
        illuminaT_++;
        break;
    case 4000481:
        rs = 62508588;
        illuminaT_++;
        break;
    case 4000635:
        rs = 121434453;
        illuminaT_++;
        break;
    case 4000834:
        rs = 199476104;
        illuminaT_++;
        break;
    case 4990151:
        rs = 111033258;
        illuminaT_++;
        break;
    case 5000002:
        rs = 121908362;
        illuminaT_++;
        break;
    case 5000003:
        rs = 111033244;
        illuminaT_++;
        break;
    case 5000043:
        rs = 200511261;
        illuminaT_++;
        break;
    case 5000047:
        rs = 369925690;
        illuminaT_++;
        break;
    case 5000119:
        rs = 879254572;
        illuminaT_++;
        break;
    case 5000064:
        rs = 80338945;
        illuminaT_++;
        break;
    case 5000092:
        rs = 121907972;
        illuminaT_++;
        break;
    case 5000121:
        rs = 199774121;
        illuminaT_++;
        break;
    case 5000142:
        rs = 749038326;
        illuminaT_++;
        break;
    case 5000663:
        rs = 3810141;
        illuminaT_++;
        break;
    case 5000693:
        rs = 111033199;
        illuminaT_++;
        break;
    case 5000696:
        rs = 111033307;
        illuminaT_++;
        break;
    case 5001996:
        rs = 80338948;
        illuminaT_++;
        break;
    case 5002001:
        rs = 104894396;
        illuminaT_++;
        break;
    case 5002096:
        rs = 267607213;
        illuminaT_++;
        break;
    case 5003116:
        rs = 121434282;
        illuminaT_++;
        break;
    case 5003117:
        rs = 121434281;
        illuminaT_++;
        break;
    case 5003700:
        rs = 121964990;
        illuminaT_++;
        break;
    case 5004213:
        rs = 121918070;
        illuminaT_++;
        break;
    case 5004778:
        rs = 74315455;
        illuminaT_++;
        break;
    case 5004781:
        rs = 74315458;
        illuminaT_++;
        break;
    case 5004790:
        rs = 74315467;
        illuminaT_++;
        break;
    case 5004829:
        rs = 121907959;
        illuminaT_++;
        break;
    case 5005735:
        rs = 104893931;
        illuminaT_++;
        break;
    case 5005799:
        rs = 119103251;
        illuminaT_++;
        break;
    case 5005800:
        rs = 119103252;
        illuminaT_++;
        break;
    case 5005927:
        rs = 80338680;
        illuminaT_++;
        break;
    case 5006058:
        rs = 121908758;
        illuminaT_++;
        break;
    case 5006076:
        rs = 121908754;
        illuminaT_++;
        break;
    case 5006092:
        rs = 121908766;
        illuminaT_++;
        break;
    case 5006103:
        rs = 78802634;
        illuminaT_++;
        break;
    case 5006112:
        rs = 75115087;
        illuminaT_++;
        break;
    case 5007145:
        rs = 137853096;
        illuminaT_++;
        break;
    case 5007146:
        rs = 137853097;
        illuminaT_++;
        break;
    case 5008215:
        rs = 76917243;
        illuminaT_++;
        break;
    case 5008436:
        rs = 137852327;
        illuminaT_++;
        break;
    case 5008440:
        rs = 137852330;
        illuminaT_++;
        break;
    case 5008441:
        rs = 267606835;
        illuminaT_++;
        break;
    case 5008450:
        rs = 137852339;
        illuminaT_++;
        break;
    case 5008462:
        rs = 137852348;
        illuminaT_++;
        break;
    case 5008753:
        rs = 193922376;
        illuminaT_++;
        break;
    case 5009095:
        rs = 397507789;
        illuminaT_++;
        break;
    case 5009097:
        rs = 397507788;
        illuminaT_++;
        break;
    case 5009099:
        rs = 397507785;
        illuminaT_++;
        break;
    case 5009107:
        rs = 397507777;
        illuminaT_++;
        break;
    case 5009119:
        rs = 397507756;
        illuminaT_++;
        break;
    case 5009120:
        rs = 397507753;
        illuminaT_++;
        break;
    case 5009136:
        rs = 397507728;
        illuminaT_++;
        break;
    case 5009151:
        rs = 276174838;
        illuminaT_++;
        break;
    case 5009161:
        rs = 397507692;
        illuminaT_++;
        break;
    case 5009162:
        rs = 886040499;
        illuminaT_++;
        break;
    case 5009163:
        rs = 397507688;
        illuminaT_++;
        break;
    case 5009165:
        rs = 397507684;
        illuminaT_++;
        break;
    case 5009173:
        rs = 80359388;
        illuminaT_++;
        break;
    case 5009174:
        rs = 397507673;
        illuminaT_++;
        break;
    case 5009175:
        rs = 397507672;
        illuminaT_++;
        break;
    case 5009179:
        rs = 397507659;
        illuminaT_++;
        break;
    case 5009215:
        rs = 397507605;
        illuminaT_++;
        break;
    case 5009223:
        rs = 397507591;
        illuminaT_++;
        break;
    case 5009244:
        rs = 397507739;
        illuminaT_++;
        break;
    case 5009256:
        rs = 1801426;
        illuminaT_++;
        break;
    case 5009299:
        rs = 11571746;
        illuminaT_++;
        break;
    case 5009331:
        rs = 55716624;
        illuminaT_++;
        break;
    case 5009334:
        rs = 169547;
        illuminaT_++;
        break;
    case 5010413:
        rs = 397507940;
        illuminaT_++;
        break;
    case 5010426:
        rs = 397507986;
        illuminaT_++;
        break;
    case 5010451:
        rs = 397507420;
        illuminaT_++;
        break;
    case 5010462:
        rs = 80359771;
        illuminaT_++;
        break;
    case 5010474:
        rs = 397507830;
        illuminaT_++;
        break;
    case 5010479:
        rs = 80359266;
        illuminaT_++;
        break;
    case 5010486:
        rs = 397507613;
        illuminaT_++;
        break;
    case 5010497:
        rs = 80359380;
        illuminaT_++;
        break;
    case 5010509:
        rs = 397507724;
        illuminaT_++;
        break;
    case 5010511:
        rs = 397507732;
        illuminaT_++;
        break;
    case 5010512:
        rs = 276174847;
        illuminaT_++;
        break;
    case 5010527:
        rs = 397507812;
        illuminaT_++;
        break;
    case 5010530:
        rs = 397507826;
        illuminaT_++;
        break;
    case 5010544:
        rs = 397507874;
        illuminaT_++;
        break;
    case 5010569:
        rs = 397507419;
        illuminaT_++;
        break;
    case 5010578:
        rs = 276174852;
        illuminaT_++;
        break;
    case 5010743:
        rs = 368505753;
        illuminaT_++;
        break;
    case 5010780:
        rs = 74467662;
        illuminaT_++;
        break;
    case 5010801:
        rs = 397508461;
        illuminaT_++;
        break;
    case 5010930:
        rs = 80282562;
        illuminaT_++;
        break;
    case 5010947:
        rs = 78440224;
        illuminaT_++;
        break;
    case 5010980:
        rs = 397508778;
        illuminaT_++;
        break;
    case 5011073:
        rs = 79031340;
        illuminaT_++;
        break;
    case 5011184:
        rs = 79031340;
        illuminaT_++;
        break;
    case 5011243:
        rs = 397508211;
        illuminaT_++;
        break;
    case 5011404:
        rs = 397508296;
        illuminaT_++;
        break;
    case 5011446:
        rs = 121908759;
        illuminaT_++;
        break;
    case 5011516:
        rs = 397508353;
        illuminaT_++;
        break;
    case 5011556:
        rs = 397515498;
        illuminaT_++;
        break;
    case 5011672:
        rs = 397508442;
        illuminaT_++;
        break;
    case 5011775:
        rs = 397508500;
        illuminaT_++;
        break;
    case 5011826:
        rs = 202179988;
        illuminaT_++;
        break;
    case 5011985:
        rs = 387906362;
        illuminaT_++;
        break;
    case 5012121:
        rs = 193922525;
        illuminaT_++;
        break;
    case 5012129:
        rs = 397508675;
        illuminaT_++;
        break;
    case 5012573:
        rs = 515726215;
        illuminaT_++;
        break;
    case 5012575:
        rs = 121908428;
        illuminaT_++;
        break;
    case 5012578:
        rs = 281865117;
        illuminaT_++;
        break;
    case 5012610:
        rs = 398124502;
        illuminaT_++;
        break;
    case 5012612:
        rs = 137852949;
        illuminaT_++;
        break;
    case 5012616:
        rs = 80338848;
        illuminaT_++;
        break;
    case 5012618:
        rs = 28939086;
        illuminaT_++;
        break;
    case 5012622:
        rs = 137852696;
        illuminaT_++;
        break;
    case 5012623:
        rs = 137852695;
        illuminaT_++;
        break;
    case 5012624:
        rs = 137852700;
        illuminaT_++;
        break;
    case 5012628:
        rs = 80356708;
        illuminaT_++;
        break;
    case 5012629:
        rs = 180177309;
        illuminaT_++;
        break;
    case 5012634:
        rs = 80338794;
        illuminaT_++;
        break;
    case 5012660:
        rs = 28937590;
        illuminaT_++;
        break;
    case 5012664:
        rs = 78340951;
        illuminaT_++;
        break;
    case 5012665:
        rs = 387906225;
        illuminaT_++;
        break;
    case 5012669:
        rs = 80356680;
        illuminaT_++;
        break;
    case 5012671:
        rs = 80356681;
        illuminaT_++;
        break;
    case 5012672:
        rs = 80356682;
        illuminaT_++;
        break;
    case 5012678:
        rs = 386833969;
        illuminaT_++;
        break;
    case 5012679:
        rs = 80338701;
        illuminaT_++;
        break;
    case 5012680:
        rs = 28936415;
        illuminaT_++;
        break;
    case 5012683:
        rs = 121918008;
        illuminaT_++;
        break;
    case 5012686:
        rs = 387906525;
        illuminaT_++;
        break;
    case 5012688:
        rs = 61750420;
        illuminaT_++;
        break;
    case 5012709:
        rs = 35887622;
        illuminaT_++;
        break;
    case 5012749:
        rs = 119466000;
        illuminaT_++;
        break;
    case 5012758:
        rs = 121434280;
        illuminaT_++;
        break;
    case 5012759:
        rs = 77931234;
        illuminaT_++;
        break;
    case 5012760:
        rs = 373715782;
        illuminaT_++;
        break;
    case 5012761:
        rs = 200724875;
        illuminaT_++;
        break;
    case 5012765:
        rs = 28940893;
        illuminaT_++;
        break;
    case 5012770:
        rs = 587776650;
        illuminaT_++;
        break;
    case 5012799:
        rs = 28941770;
        illuminaT_++;
        break;
    case 5012804:
        rs = 111033260;
        illuminaT_++;
        break;
    case 5012861:
        rs = 80338898;
        illuminaT_++;
        break;
    case 5012862:
        rs = 80338899;
        illuminaT_++;
        break;
    case 5012864:
        rs = 121965075;
        illuminaT_++;
        break;
    case 5012865:
        rs = 80338901;
        illuminaT_++;
        break;
    case 5012867:
        rs = 80338895;
        illuminaT_++;
        break;
    case 5012878:
        rs = 80356489;
        illuminaT_++;
        break;
    case 5012880:
        rs = 80356491;
        illuminaT_++;
        break;
    case 5037820:
        rs = 267608126;
        illuminaT_++;
        break;
    case 5037831:
        rs = 863225414;
        illuminaT_++;
        break;
    case 5037832:
        rs = 786201855;
        illuminaT_++;
        break;
    case 5037835:
        rs = 267608101;
        illuminaT_++;
        break;
    case 5037850:
        rs = 267608037;
        illuminaT_++;
        break;
    case 5037858:
        rs = 267608050;
        illuminaT_++;
        break;
    case 5037866:
        rs = 267608114;
        illuminaT_++;
        break;
    case 5037874:
        rs = 267608090;
        illuminaT_++;
        break;
    case 5037877:
        rs = 267608085;
        illuminaT_++;
        break;
    case 5037884:
        rs = 267608072;
        illuminaT_++;
        break;
    case 5037895:
        rs = 193922343;
        illuminaT_++;
        break;
    case 5041058:
        rs = 72658154;
        illuminaT_++;
        break;
    case 5046667:
        rs = 876660428;
        illuminaT_++;
        break;
    case 5047658:
        rs = 63750215;
        illuminaT_++;
        break;
    case 5048984:
        rs = 72551348;
        illuminaT_++;
        break;
    case 5049013:
        rs = 139607673;
        illuminaT_++;
        break;
    case 5050964:
        rs = 199474658;
        illuminaT_++;
        break;
    case 5050984:
        rs = 199474657;
        illuminaT_++;
        break;
    case 5050994:
        rs = 267606617;
        illuminaT_++;
        break;
    case 5053826:
        rs = 79031340;
        illuminaT_++;
        break;
    case 5053827:
        rs = 77409459;
        illuminaT_++;
        break;
    case 5053833:
        rs = 75115087;
        illuminaT_++;
        break;
    case 5053834:
        rs = 79633941;
        illuminaT_++;
        break;
    case 5053836:
        rs = 78194216;
        illuminaT_++;
        break;
    case 5053839:
        rs = 78802634;
        illuminaT_++;
        break;
    case 5053851:
        rs = 78440224;
        illuminaT_++;
        break;
    case 5053895:
        rs = 76151636;
        illuminaT_++;
        break;
    case 6007566:
        rs = 6151429;
        illuminaT_++;
        break;
    case 6007689:
        rs = 398123126;
        illuminaT_++;
        break;
    case 6007710:
        rs = 367543005;
        illuminaT_++;
        break;
    case 6009255:
        rs = 515726176;
        illuminaT_++;
        break;
    case 6009259:
        rs = 515726174;
        illuminaT_++;
        break;
    case 6009276:
        rs = 515726175;
        illuminaT_++;
        break;
    case 6014100:
        rs = 879254382;
        illuminaT_++;
        break;
    case 6014141:
        rs = 201573863;
        illuminaT_++;
        break;
    case 6014161:
        rs = 370471092;
        illuminaT_++;
        break;
    case 6014217:
        rs = 773658037;
        illuminaT_++;
        break;
    case 6014229:
        rs = 544453230;
        illuminaT_++;
        break;
    case 6014299:
        rs = 879254662;
        illuminaT_++;
        break;
    case 6014548:
        rs = 879255045;
        illuminaT_++;
        break;
    case 6014573:
        rs = 875989911;
        illuminaT_++;
        break;
    case 6015375:
        rs = 992677795;
        illuminaT_++;
        break;
    case 6016654:
        rs = 200511261;
        illuminaT_++;
        break;
    case 6024270:
        rs = 774159791;
        illuminaT_++;
        break;
    case 700014:
        rs = 879255033;
        illuminaT_++;
        break;
    case 700020:
        rs = 879253834;
        illuminaT_++;
        break;
    case 700076:
        rs = 767388144;
        illuminaT_++;
        break;
    case 700184:
        rs = 367543006;
        illuminaT_++;
        break;
    case 700566:
        rs = 887126161;
        illuminaT_++;
        break;
    case 700568:
        rs = 863225434;
        illuminaT_++;
        break;
    case 700621:
        rs = 1135401729;
        illuminaT_++;
        break;
    case 700645:
        rs = 781986930;
        illuminaT_++;
        break;
    case 700947:
        rs = 1085307466;
        illuminaT_++;
        break;
    case 700994:
        rs = 879254740;
        illuminaT_++;
        break;
    case 701087:
        rs = 879255213;
        illuminaT_++;
        break;
    case 701112:
        rs = 879255281;
        illuminaT_++;
        break;
    case 701164:
        rs = 879254412;
        illuminaT_++;
        break;
    case 701180:
        rs = 879254761;
        illuminaT_++;
        break;
    case 701345:
        rs = 879254817;
        illuminaT_++;
        break;
    case 701347:
        rs = 879254886;
        illuminaT_++;
        break;
    case 701362:
        rs = 879254434;
        illuminaT_++;
        break;
    case 701371:
        rs = 879254884;
        illuminaT_++;
        break;
    case 701383:
        rs = 879254443;
        illuminaT_++;
        break;
    case 701409:
        rs = 1419032418;
        illuminaT_++;
        break;
    case 701414:
        rs = 879254410;
        illuminaT_++;
        break;
    case 701450:
        rs = 879254841;
        illuminaT_++;
        break;
    case 701496:
        rs = 879255105;
        illuminaT_++;
        break;
    case 701505:
        rs = 879254521;
        illuminaT_++;
        break;
    case 701811:
        rs = 879254708;
        illuminaT_++;
        break;
    case 702110:
        rs = 879254474;
        illuminaT_++;
        break;
    case 702255:
        rs = 1461905374;
        illuminaT_++;
        break;
    case 702310:
        rs = 879254899;
        illuminaT_++;
        break;
    case 702329:
        rs = 781319410;
        illuminaT_++;
        break;
    case 702373:
        rs = 879254651;
        illuminaT_++;
        break;
    case 702455:
        rs = 879255282;
        illuminaT_++;
        break;
    case 702480:
        rs = 879254687;
        illuminaT_++;
        break;
    case 702563:
        rs = 879254404;
        illuminaT_++;
        break;
    case 702565:
        rs = 879255026;
        illuminaT_++;
        break;
    case 703107:
        rs = 879254393;
        illuminaT_++;
        break;
    case 703167:
        rs = 1553137543;
        illuminaT_++;
        break;
    case 703267:
        rs = 879254397;
        illuminaT_++;
        break;
    case 703316:
        rs = 879255311;
        illuminaT_++;
        break;
    case 703322:
        rs = 879254574;
        illuminaT_++;
        break;
    case 703359:
        rs = 879254702;
        illuminaT_++;
        break;
    case 703379:
        rs = 879255029;
        illuminaT_++;
        break;
    case 703467:
        rs = 879254673;
        illuminaT_++;
        break;
    case 703601:
        rs = 879254653;
        illuminaT_++;
        break;
    case 703652:
        rs = 879254896;
        illuminaT_++;
        break;
    case 703668:
        rs = 879254435;
        illuminaT_++;
        break;
    case 703692:
        rs = 879254614;
        illuminaT_++;
        break;
    case 703728:
        rs = 879254710;
        illuminaT_++;
        break;
    case 703733:
        rs = 879255320;
        illuminaT_++;
        break;
    case 703821:
        rs = 879254666;
        illuminaT_++;
        break;
    case 703873:
        rs = 879254923;
        illuminaT_++;
        break;
    case 703916:
        rs = 876659708;
        illuminaT_++;
        break;
    case 703960:
        rs = 879255224;
        illuminaT_++;
        break;
    case 703963:
        rs = 879254491;
        illuminaT_++;
        break;
    case 704105:
        rs = 1555803643;
        illuminaT_++;
        break;
    case 704128:
        rs = 879254436;
        illuminaT_++;
        break;
    case 704190:
        rs = 879254958;
        illuminaT_++;
        break;
    case 704225:
        rs = 879254384;
        illuminaT_++;
        break;
    case 704231:
        rs = 397515991;
        illuminaT_++;
        break;
    case 704270:
        rs = 397516073;
        illuminaT_++;
        break;
    case 704424:
        rs = 879254123;
        illuminaT_++;
        break;
    case 704512:
        rs = 879255175;
        illuminaT_++;
        break;
    case 704677:
        rs = 879254582;
        illuminaT_++;
        break;
    case 704827:
        rs = 879254573;
        illuminaT_++;
        break;
    case 704874:
        rs = 879254888;
        illuminaT_++;
        break;
    case 704887:
        rs = 879254821;
        illuminaT_++;
        break;
    case 704902:
        rs = 879254922;
        illuminaT_++;
        break;
    case 704916:
        rs = 879254552;
        illuminaT_++;
        break;
    case 704986:
        rs = 879253965;
        illuminaT_++;
        break;
    case 704992:
        rs = 879254771;
        illuminaT_++;
        break;
    case 705007:
        rs = 879254799;
        illuminaT_++;
        break;
    case 705032:
        rs = 879254569;
        illuminaT_++;
        break;
    case 705059:
        rs = 875989898;
        illuminaT_++;
        break;
    case 705119:
        rs = 879254591;
        illuminaT_++;
        break;
    case 705179:
        rs = 879254538;
        illuminaT_++;
        break;
    case 705234:
        rs = 1060499931;
        illuminaT_++;
        break;
    case 705258:
        rs = 879254413;
        illuminaT_++;
        break;
    case 705313:
        rs = 879254701;
        illuminaT_++;
        break;
    case 705428:
        rs = 879254388;
        illuminaT_++;
        break;
    case 705498:
        rs = 879254138;
        illuminaT_++;
        break;
    case 705502:
        rs = 879254885;
        illuminaT_++;
        break;
    case 705645:
        rs = 879254440;
        illuminaT_++;
        break;
    case 705657:
        rs = 879254581;
        illuminaT_++;
        break;
    case 705698:
        rs = 879254882;
        illuminaT_++;
        break;
    case 705710:
        rs = 879254594;
        illuminaT_++;
        break;
    case 705843:
        rs = 879254387;
        illuminaT_++;
        break;
    case 705867:
        rs = 879254512;
        illuminaT_++;
        break;
    case 705904:
        rs = 80357634;
        illuminaT_++;
        break;
    case 705932:
        rs = 879254402;
        illuminaT_++;
        break;
    case 706122:
        rs = 879254027;
        illuminaT_++;
        break;
    case 706178:
        rs = 879254116;
        illuminaT_++;
        break;
    case 706241:
        rs = 879254952;
        illuminaT_++;
        break;
    case 706291:
        rs = 879255031;
        illuminaT_++;
        break;
    case 706299:
        rs = 879254575;
        illuminaT_++;
        break;
    case 706312:
        rs = 879254768;
        illuminaT_++;
        break;
    case 706314:
        rs = 879254122;
        illuminaT_++;
        break;
    case 706344:
        rs = 730881471;
        illuminaT_++;
        break;
    case 706367:
        rs = 879254736;
        illuminaT_++;
        break;
    case 706371:
        rs = 879254737;
        illuminaT_++;
        break;
    case 706379:
        rs = 879254530;
        illuminaT_++;
        break;
    case 706465:
        rs = 879254748;
        illuminaT_++;
        break;
    case 706566:
        rs = 879254675;
        illuminaT_++;
        break;
    case 706617:
        rs = 879254494;
        illuminaT_++;
        break;
    case 706659:
        rs = 879254111;
        illuminaT_++;
        break;
    case 706716:
        rs = 879254980;
        illuminaT_++;
        break;
    case 706888:
        rs = 879254580;
        illuminaT_++;
        break;
    case 709033:
        rs = 864622557;
        illuminaT_++;
        break;
    case 709034:
        rs = 864622558;
        illuminaT_++;
        break;
    case 709063:
        rs = 80359264;
        illuminaT_++;
        break;
    case 709064:
        rs = 864622609;
        illuminaT_++;
        break;
    case 709068:
        rs = 864622073;
        illuminaT_++;
        break;
    case 709069:
        rs = 864622082;
        illuminaT_++;
        break;
    case 709070:
        rs = 864622200;
        illuminaT_++;
        break;
    case 709076:
        rs = 864622277;
        illuminaT_++;
        break;
    case 709466:
        rs = 150074056;
        illuminaT_++;
        break;
    case 712972:
        rs = 864622651;
        illuminaT_++;
        break;
    case 723120:
        rs = 864622480;
        illuminaT_++;
        break;
    case 723181:
        rs = 864622257;
        illuminaT_++;
        break;
    case 723189:
        rs = 864622161;
        illuminaT_++;
        break;
    case 723191:
        rs = 864622431;
        illuminaT_++;
        break;
    case 723194:
        rs = 864622509;
        illuminaT_++;
        break;
    case 723202:
        rs = 864622192;
        illuminaT_++;
        break;
    case 723205:
        rs = 864622481;
        illuminaT_++;
        break;
    case 723234:
        rs = 864622707;
        illuminaT_++;
        break;
	case 3000001: //23andMe RS IDs V3
        rs = 113993960;  // RS113993960
        illuminaT_++;
        break;
    case 3000014:
        rs = 372706460;  // RS372706460
        illuminaT_++;
        break;
    case 3000029:
        rs = 375151448;  // RS375151448
        illuminaT_++;
        break;
    case 3000037:
        rs = 796993763;  // RS796993763
        illuminaT_++;
        break;
    case 3000065:
        rs = 796727509;  // RS796727509
        illuminaT_++;
        break;
    case 3000078:
        rs = 372362604;  // RS372362604
        illuminaT_++;
        break;
    case 3000206:
        rs = 17215500;  // RS17215500
        illuminaT_++;
        break;
    case 3000564:
        rs = 41464546;  // RS41464546
        illuminaT_++;
        break;
    case 3000954:
        rs = 28358282;  // RS28358282
        illuminaT_++;
        break;
    case 3001096:
        rs = 118203888;  // RS118203888
        illuminaT_++;
        break;
    case 3001474:
        rs = 3021088;  // RS3021088
        illuminaT_++;
        break;
    case 3002432:
        rs = 1799963;  // RS1799963
        illuminaT_++;
        break;
    case 3002449:
        rs = 1800123;  // RS1800123
        illuminaT_++;
        break;
    case 3002455:
        rs = 1800386;  // RS1800386
        illuminaT_++;
        break;
    case 3002468:
        rs = 1800730;  // RS1800730
        illuminaT_++;
        break;
    case 3002484:
        rs = 1801086;  // RS1801086
        illuminaT_++;
        break;
    case 3002485:
        rs = 1801166;  // RS1801166
        illuminaT_++;
        break;
    case 3002486:
        rs = 1801175;  // RS1801175
        illuminaT_++;
        break;
    case 3002507:
        rs = 1805009;  // RS1805009
        illuminaT_++;
        break;
    case 3002517:
        rs = 1805137;  // RS1805137
        illuminaT_++;
        break;
    case 3002721:
        rs = 121917808;  // RS121917808
        illuminaT_++;
        break;
    case 3002758:
        rs = 28933979;  // RS28933979
        illuminaT_++;
        break;
    case 3002759:
        rs = 76992529;  // RS76992529
        illuminaT_++;
        break;
    case 3002773:
        rs = 118192163;  // RS118192163
        illuminaT_++;
        break;
    case 3002796:
        rs = 104894724;  // RS104894724
        illuminaT_++;
        break;
    case 3002797:
        rs = 121964895;  // RS121964895
        illuminaT_++;
        break;
    case 3002808:
        rs = 79761867;  // RS79761867
        illuminaT_++;
        break;
    case 3003043:
        rs = 28940285;  // RS28940285
        illuminaT_++;
        break;
    case 3003074:
        rs = 28941776;  // RS28941776
        illuminaT_++;
        break;
    case 3003090:
        rs = 28942081;  // RS28942081
        illuminaT_++;
        break;
    case 3003093:
        rs = 28942084;  // RS28942084
        illuminaT_++;
        break;
    case 3003094:
        rs = 28942085;  // RS28942085
        illuminaT_++;
        break;
    case 3003387:
        rs = 5030808;  // RS5030808
        illuminaT_++;
        break;
    case 3003389:
        rs = 5030818;  // RS5030818
        illuminaT_++;
        break;
    case 3003391:
        rs = 5030821;  // RS5030821
        illuminaT_++;
        break;
    case 3003392:
        rs = 5030823;  // RS5030823
        illuminaT_++;
        break;
    case 3003397:
        rs = 5030843;  // RS5030843
        illuminaT_++;
        break;
    case 3003398:
        rs = 5030846;  // RS5030846
        illuminaT_++;
        break;
    case 3003399:
        rs = 5030847;  // RS5030847
        illuminaT_++;
        break;
    case 3003400:
        rs = 5030850;  // RS5030850
        illuminaT_++;
        break;
    case 3003401:
        rs = 5030851;  // RS5030851
        illuminaT_++;
        break;
    case 3003403:
        rs = 5030856;  // RS5030856
        illuminaT_++;
        break;
    case 3003404:
        rs = 5030859;  // RS5030859
        illuminaT_++;
        break;
    case 3003405:
        rs = 5030860;  // RS5030860
        illuminaT_++;
        break;
    case 3003411:
        rs = 5030869;  // RS5030869
        illuminaT_++;
        break;
    case 4000101:
        rs = 796483281;  // RS796483281
        illuminaT_++;
        break;
    case 4000146:
        rs = 796827870;  // RS796827870
        illuminaT_++;
        break;
    case 4000163:
        rs = 773700419;  // RS773700419
        illuminaT_++;
        break;
    case 4000164:
        rs = 368154770;  // RS368154770
        illuminaT_++;
        break;
    case 4000166:
        rs = 375296284;  // RS375296284
        illuminaT_++;
        break;
    case 4000171:
        rs = 368057017;  // RS368057017
        illuminaT_++;
        break;
    case 4000178:
        rs = 112707890;  // RS112707890
        illuminaT_++;
        break;
    case 4000186:
        rs = 113822196;  // RS113822196
        illuminaT_++;
        break;
    case 4000189:
        rs = 113686221;  // RS113686221
        illuminaT_++;
        break;
    case 4000291:
        rs = 74551128;  // RS74551128
        illuminaT_++;
        break;
    case 4000292:
        rs = 121908745;  // RS121908745
        illuminaT_++;
        break;
    case 4000294:
        rs = 75961395;  // RS75961395
        illuminaT_++;
        break;
    case 4000295:
        rs = 78655421;  // RS78655421
        illuminaT_++;
        break;
    case 4000296:
        rs = 121909011;  // RS121909011
        illuminaT_++;
        break;
    case 4000297:
        rs = 77932196;  // RS77932196
        illuminaT_++;
        break;
    case 4000299:
        rs = 77646904;  // RS77646904
        illuminaT_++;
        break;
    case 4000300:
        rs = 113993959;  // RS113993959
        illuminaT_++;
        break;
    case 4000301:
        rs = 121908755;  // RS121908755
        illuminaT_++;
        break;
    case 4000302:
        rs = 121909005;  // RS121909005
        illuminaT_++;
        break;
    case 4000305:
        rs = 75527207;  // RS75527207
        illuminaT_++;
        break;
    case 4000306:
        rs = 74597325;  // RS74597325
        illuminaT_++;
        break;
    case 4000307:
        rs = 80055610;  // RS80055610
        illuminaT_++;
        break;
    case 4000308:
        rs = 74767530;  // RS74767530
        illuminaT_++;
        break;
    case 4000309:
        rs = 77010898;  // RS77010898
        illuminaT_++;
        break;
    case 3002703:
        rs = 137854448;  // RS137854448
        illuminaT_++;
        break;
    case 4000339:
        rs = 144467873;  // RS144467873
        illuminaT_++;
        break;
    case 4000340:
        rs = 773328511;  // RS773328511
        illuminaT_++;
        break;
    case 4000341:
        rs = 879254867;  // RS879254867
        illuminaT_++;
        break;
    case 4000342:
        rs = 879254866;  // RS879254866
        illuminaT_++;
        break;
    case 4000343:
        rs = 879255059;  // RS879255059
        illuminaT_++;
        break;
    case 4000345:
        rs = 150021927;  // RS150021927
        illuminaT_++;
        break;
    case 4000346:
        rs = 121908024;  // RS121908024
        illuminaT_++;
        break;
    case 4000347:
        rs = 121908029;  // RS121908029
        illuminaT_++;
        break;
    case 4000349:
        rs = 121908038;  // RS121908038
        illuminaT_++;
        break;
    case 4000353:
        rs = 775924858;  // RS775924858
        illuminaT_++;
        break;
    case 4000354:
        rs = 121908033;  // RS121908033
        illuminaT_++;
        break;
    case 4000355:
        rs = 121908026;  // RS121908026
        illuminaT_++;
        break;
    case 4000357:
        rs = 121908028;  // RS121908028
        illuminaT_++;
        break;
    case 4000362:
        rs = 121908035;  // RS121908035
        illuminaT_++;
        break;
    case 4000366:
        rs = 121908025;  // RS121908025
        illuminaT_++;
        break;
    case 4000367:
        rs = 769318035;  // RS769318035
        illuminaT_++;
        break;
    case 4000368:
        rs = 769737896;  // RS769737896
        illuminaT_++;
        break;
    case 4000369:
        rs = 879254800;  // RS879254800
        illuminaT_++;
        break;
    case 4000372:
        rs = 370777955;  // RS370777955
        illuminaT_++;
        break;
    case 4000374:
        rs = 137929307;  // RS137929307
        illuminaT_++;
        break;
    case 4000457:
        rs = 80357919;  // RS80357919
        illuminaT_++;
        break;
    case 4000459:
        rs = 80357711;  // RS80357711
        illuminaT_++;
        break;
    case 4000461:
        rs = 80357662;  // RS80357662
        illuminaT_++;
        break;
    case 4000462:
        rs = 555607708;  // RS555607708
        illuminaT_++;
        break;
    case 4000464:
        rs = 80359874;  // RS80359874
        illuminaT_++;
        break;
	//ALL RSID's from SNPedia Illumina to dbSNP mapping file included above as of 12/9/2025
    default:
        illuminaU_++; //untranslaned lines
        break;
    }
    return rs;
}
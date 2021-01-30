#include "SnipParser.h"
//SNP in human genome 10430639. 
struct ST
{
    int rs;       // RS number
    char ch[4];   // Chromosone
    int pos;      // Position
    char a;       // first nucleotide
    char b;       // second nucleotide
} sname;

std::vector<ST> snp(10430639);

// Use 'variant::index' to know the type strored in variant (zero-based index). 
//Read an Ancestory.com RAW DNA file and create a 'standard array'

bool  SnipParser::Ancestory(wchar_t* fi_ )
{


    //std::fstream  fs(fi_, std::ios_base::in | std::ios_base::binary);
    std::fstream  fs;
    if (!fs.bad())
    {
        char nbuffer[256];
        int loopbreak = 0;

        //START: reset loadcount_ and vecotr for next file for next file
        loadCount_ = 0; 
        snp.clear();
        snp.resize(10430639);
        //END: reset loadcount_ and vecotr for next file for next file

        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;
         
            while (!fs.eof())
            {
                loopbreak = 0;
                //read a line into a temorary buffer
                fs.getline(nbuffer, 256);
                int rdindex = 2;
                //GET RS Number numeric part only
                if (nbuffer[0] == 'r' && nbuffer[1] == 's' && isdigit(nbuffer[2])) {
                    char num[20];
                    int  nmindex = 0;
                    num[0] = NULL;
                    while (isdigit(nbuffer[rdindex]) && rdindex < 256)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = NULL;
                    //First in the line is the RS number
                    snp[inx].rs = atoi(num);
                    //move past tab or spaces to next numeric data chromosone number
                    while (!isdigit(nbuffer[rdindex]) && rdindex < 256 && nbuffer[rdindex] != 'X' && nbuffer[rdindex] != 'Y' && nbuffer[rdindex] != 'x' && nbuffer[rdindex] != 'y')
                    {//wrote whith braces for readablility
                        rdindex++;
                    }
                    //re-init 
                    nmindex = 0;
                    num[0] = NULL;

                    if (isdigit(nbuffer[rdindex])) //if autosomal chr
                    {
                        while (isdigit(nbuffer[rdindex]) && rdindex < 256 && nmindex < 2)//last clause is to prevent overflow on corrupt file
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        //second in the line is the chromosone number
                        num[nmindex] = NULL;
                        strcpy_s(snp[inx].ch, num);
                    }
                    else {
                        snp[inx].ch[0] = nbuffer[rdindex]; //X & Y sinle char so why waste a strcpy call!
                        if (snp[inx].ch[0] == 'x') snp[inx].ch[0] = 'X'; //paranoia cass fix
                        if (snp[inx].ch[0] == 'y') snp[inx].ch[0] = 'Y'; //paranoia cass fix
                        snp[inx].ch[1] = NULL;

                    }

                    //move past tab or spaces to next numeric data posotion number
                    while (!isdigit(nbuffer[rdindex]) && rdindex < 256)
                    {//wrote whith braces for readablility
                        rdindex++;
                    }
                    //read position
                        //re-init 
                    nmindex = 0;
                    num[0] = NULL;
                    while (isdigit(nbuffer[rdindex]) && rdindex < 256)
                    {
                        num[nmindex] = nbuffer[rdindex];
                        rdindex++;
                        nmindex++;
                    }
                    num[nmindex] = NULL;
                    //second in the line is the chromosone number
                    snp[inx].pos = atoi(num);

                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                         && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && rdindex < 256)
                    {
                        rdindex++;
                    }
                    snp[inx].a = nbuffer[rdindex];
                    rdindex++;
                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && rdindex < 256)
                    {
                        rdindex++;
                    }
                    //Is Char
                    snp[inx].b = nbuffer[rdindex];
                    //increment the primary index
                    inx++;
                    //increment count of lines loaded
                    loadCount_++;
                }
                loopbreak++;
                if (loopbreak == 2000) break;
            }
            fs.close();
        }
        else false;
    }

    return true;
};


//return the number of lines loaded
int SnipParser::SNPCount(void)
{
    return loadCount_;
}

/*RS number search function passed the RS number to searcg for
and a structure of type s to place the data in if a match is found */
bool SnipParser::RsSearch(int *rs, char* chr1, char* chr2, char* chr3, char* chr4, int *pos, char *a, char *b)
{

    // Check you have SNP data loaded
    if (loadCount_ > 0)
    { 
        // Search for RS numer int rs
        for (unsigned int i = 0; i <=loadCount_;)
        {
            if (snp[i].rs == *rs)
            {
                //match found! populate and return the structure
                *rs = snp[i].rs;    // RS number
                *chr1 = snp[i].ch[0];    // Chromosone array as string
                *chr2 = snp[i].ch[1];    // Chromosone array as string
                *chr3 = snp[i].ch[2];    // Chromosone array as string
                *chr4 = snp[i].ch[3];    // Chromosone array as string
                *pos = snp[i].pos;  // Position
                *a = snp[i].a;      // first nucleotide
                *b = snp[i].b;      // first nucleotide
                return true;
            }
            //inceament loop
            i++;
        }

    }
    return false;
}


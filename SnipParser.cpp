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
//Read an Ancestory.com/23t0me and FtDNA RAW DNA file and create a 'standard array'

bool  SnipParser::Ancestory(wchar_t* fi_)
{
     
    //std::fstream  fs(fi_, std::ios_base::in | std::ios_base::binary);
    std::fstream  fs;
    if (!fs.bad())
    {
        char nbuffer[256];
        int loopbreak = 0;
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;
            int fdind = 0; //ftdna-illumina
            //START: reset loadcount_ and vecotr for next file for next file
            loadCount_ = 0;
            snp.clear();
            snp.resize(10430639);
            //END: reset loadcount_ and vecotr for next file for next file
            while (!fs.eof())
            {
                fdind = 0;
                //read a line into a temorary buffer
                fs.getline(nbuffer, 256);
                int rdindex = 2;
                if (nbuffer[fdind] == '\"') fdind++; //ftdna-illumina
                //GET RS Number numeric part only
                if (nbuffer[fdind] == 'r' && nbuffer[fdind+1] == 's' && isdigit(nbuffer[fdind+2])) {//ftdna-illumina
                    char num[20];
                    int  nmindex = 0;
                    loopbreak = 0;
                    rdindex += fdind;//ftdna-illumina
                    num[0] = NULL;
                    while (isdigit(nbuffer[rdindex]) && rdindex < 23)
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
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < 256)
                    {
                        rdindex++;
                    }
                    snp[inx].a = nbuffer[rdindex];
                    rdindex++;
                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '0' && nbuffer[rdindex] != '-' && rdindex < 256)
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

//FTDNA has VG numbers that may corespond to RS numbers
bool  SnipParser::FTDNA(wchar_t* fi_)
{


    //std::fstream  fs(fi_, std::ios_base::in | std::ios_base::binary);
    std::fstream  fs;
    if (!fs.bad())
    {
        char nbuffer[256];
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
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.clear();
            snp.resize(10430639);
            //END: reset loadcount_ and vecotr for next file for next file
            while (!fs.eof())
            {
                fdind = 0;
                //read a line into a temorary buffer
                fs.getline(nbuffer, 256);
                int rdindex = 2;
                if (nbuffer[fdind] == '\"') fdind++; //ftdna-illumina
                //GET RS Number numeric part only 

                if (((nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's') || (nbuffer[fdind] == 'V' && nbuffer[fdind + 1] == 'G')) && isdigit(nbuffer[fdind + 2]))
                {//ftdna-illumina
                    int ftdna = 0;
                    char num[20];
                    int  nmindex = 0;
                    loopbreak = 0;
                    rdindex += fdind;//ftdna-illumina
                    num[0] = NULL;

                    if (nbuffer[fdind] == 'r' && nbuffer[fdind + 1] == 's')
                    {
                        while (isdigit(nbuffer[rdindex]) && rdindex < 24)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = NULL;
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
                        while (isdigit(nbuffer[fdind]) && fdind < 24)
                        {
                            num[nmindex] = nbuffer[fdind];
                            fdind++;
                            nmindex++;
                        }
                        num[nmindex] = NULL;
                        rdindex =fdind;  //set read index
                        ftdna = atoi(num); //in example  VG01S1077 ftdna will equal numeric 11007

                        switch (ftdna) {//rs equivelent ALL Illumina VG codes seem to realte to Cystic Fibrosis!
                        case 11077:
                            snp[inx].rs = 77931234;
                            illuminaT_++;
                            break;
                        case 12269:
                            snp[inx].rs = 77931234;
                            illuminaT_++;
                            break;
                        case 11161:
                            snp[inx].rs = 78396650;
                            illuminaT_++;
                            break;
                        case 11154:
                            snp[inx].rs = 74500255;
                            illuminaT_++;
                            break;
                        case 11135:
                            snp[inx].rs = 77829017;
                            illuminaT_++;
                            break;
                        case 425864:
                            snp[inx].rs = 75790268;
                            break;
                        case 526940:
                            snp[inx].rs = 79967166;
                            illuminaT_++;
                            break;
                        case 729384:
                            snp[inx].rs = 77284892;
                            illuminaT_++;
                            break;
                        case 729458:
                            snp[inx].rs = 75961395;
                            break;
                        case 729628:
                            snp[inx].rs = 78655421;
                            illuminaT_++;
                            break;
                        case 744961:
                            snp[inx].rs = 78756941;
                            illuminaT_++;
                            break;
                        case 744986:
                            snp[inx].rs = 77188391;
                            illuminaT_++;
                            break;
                        case 745007:
                            snp[inx].rs = 78440224;
                            illuminaT_++;
                            break;
                        case 752452:
                            snp[inx].rs = 77409459;
                            illuminaT_++;
                            break;
                        case 729244:
                            snp[inx].rs = 77409459;
                            illuminaT_++;
                            break;
                        case 729268:
                            snp[inx].rs = 79282516;
                            illuminaT_++;
                            break;
                        case 745090:
                            snp[inx].rs = 76713772;
                            illuminaT_++;
                            break;
                        case 729293:
                            snp[inx].rs = 75527207;
                            illuminaT_++;
                            break;
                        case 729297:
                            snp[inx].rs = 74597325;
                            illuminaT_++;
                            break;
                        case 729341:
                            snp[inx].rs = 75115087;
                            illuminaT_++;
                            break;
                        case 752241:
                            snp[inx].rs = 121908787;
                            illuminaT_++;
                            break;
                        case 729361:
                            snp[inx].rs = 79633941;
                            illuminaT_++;
                            break;
                        case 750448:
                            snp[inx].rs = 121908767;
                            illuminaT_++;
                            break;
                        case 745132:
                            snp[inx].rs = 76151804;
                            illuminaT_++;
                            break;
                        case 729391:
                            snp[inx].rs = 78194216;
                            illuminaT_++;
                            break;
                        case 729406:
                            snp[inx].rs = 78802634;
                            illuminaT_++;
                            break;
                        case 729419:
                            snp[inx].rs = 75541969;
                            illuminaT_++;
                            break;
                        case 729422:
                            snp[inx].rs = 79850223;
                            illuminaT_++;
                            break;
                        case 729424:
                            snp[inx].rs = 74767530;
                            illuminaT_++;
                            break;
                        case 729435:
                            snp[inx].rs = 75389940;
                            illuminaT_++;
                            break;
                        case 752449:
                            snp[inx].rs = 75039782;
                            illuminaT_++;
                            break;
                        case 729444:
                            snp[inx].rs = 74503330;
                            illuminaT_++;
                            break;
                        case 729451:
                            snp[inx].rs = 77010898;
                            illuminaT_++;
                            break;
                        case 932163:
                            snp[inx].rs = 77375493;
                            illuminaT_++;
                            break;
                        case 931849:
                            snp[inx].rs = 79833450;
                            illuminaT_++;
                            break;
                        case 931531:
                            snp[inx].rs = 77718928;
                            illuminaT_++;
                            break;
                        case 952451:
                            snp[inx].rs = 78340951;
                            illuminaT_++;
                            break;
                        case 115372:
                            snp[inx].rs = 33951465;
                            illuminaT_++;
                            break;
                        case 128248:
                            snp[inx].rs = 62644499;
                            illuminaT_++;
                            break;
                        case 128239:
                            snp[inx].rs = 5030859;
                            illuminaT_++;
                            break;
                        case 128233:
                            snp[inx].rs = 5030857;
                            illuminaT_++;
                            break;
                        case 128222:
                            snp[inx].rs = 5030856;
                            illuminaT_++;
                            break;
                        case 128220:
                            snp[inx].rs = 62516101;
                            break;
                        case 128217:
                            snp[inx].rs = 62642937;
                            illuminaT_++;
                            break;
                        case 128204:
                            snp[inx].rs = 62516095;
                            illuminaT_++;
                            break;
                        case 128194:
                            snp[inx].rs = 62508646;
                            illuminaT_++;
                            break;
                        case 128480:
                            snp[inx].rs = 5030853;
                            illuminaT_++;
                            break;
                        case 128479:
                            snp[inx].rs = 62642933;
                            illuminaT_++;
                            break;
                        case 128467:
                            snp[inx].rs = 5030851;
                            illuminaT_++;
                            break;
                        case 128449:
                            snp[inx].rs = 62514952;
                            illuminaT_++;
                            break;
                        case 128434:
                            snp[inx].rs = 5030850;
                            illuminaT_++;
                            break;
                        case 128424:
                            snp[inx].rs = 62644503;
                            illuminaT_++;
                            break;
                        case 128405:
                            snp[inx].rs = 5030846;
                            illuminaT_++;
                            break;
                        case 128402:
                            snp[inx].rs = 62508730;
                            illuminaT_++;
                            break;
                        case 128320:
                            snp[inx].rs = 5030843;
                            illuminaT_++;
                            break;
                        case 128298:
                            snp[inx].rs = 76296470;
                            illuminaT_++;
                            break;
                        case 128296:
                            snp[inx].rs = 62642929;
                            illuminaT_++;
                            break;
                        case 128223:
                            snp[inx].rs = 62642926;
                            illuminaT_++;
                            break;
                        case 1352444:
                            snp[inx].rs = 76151636;
                            illuminaT_++;
                            break;
                        case 1512228:
                            snp[inx].rs = 78310315;
                            illuminaT_++;
                            break;         
                        case 1716802:
                            snp[inx].rs = 28940574;
                            illuminaT_++;
                            break;
                        case 2123270:
                            snp[inx].rs = 5742905;
                            illuminaT_++;
                            break;
                         default:illuminaU_++; //untranslaned lines
                            break;
                        }

                    }
                 if (snp[inx].rs > 0) //stop line load of uniterpreted VG codes
                 {

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
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < 256)
                    {
                        rdindex++;
                    }
                    snp[inx].a = nbuffer[rdindex];
                    rdindex++;
                    //As the last two values are not numeric we have to rely on the file still being TAB delimited
                    while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                        && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < 256)
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

//23andMe function

//FTDNA has VG numbers that may corespond to RS numbers
bool  SnipParser::f23andMe(wchar_t* fi_)
{


    //std::fstream  fs(fi_, std::ios_base::in | std::ios_base::binary);
    std::fstream  fs;
    if (!fs.bad())
    {
        char nbuffer[256];
        int loopbreak = 0;
       
        //Open file for read 
        fs.open(fi_, std::ios::in);
        //Check file was opened  
        if (fs.is_open()) {
            int inx = 0;

            //START: reset loadcount_ and vector for next file for next file
            loadCount_ = 0;
            //Illumina unloaded count
            illuminaU_ = illuminaT_ = 0; //Reset Transaled / Untransalated counts
            snp.clear();
            snp.resize(10430639);
            //END: reset loadcount_ and vecotr for next file for next file
            while (!fs.eof())
            {
                //read a line into a temorary buffer
                fs.getline(nbuffer, 256);
                int rdindex = 0;

                //GET RS Number numeric part only 

                if (((nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's') || (nbuffer[rdindex] == 'i' && isdigit(nbuffer[rdindex + 1]) )) && isdigit(nbuffer[rdindex + 2]))
                {//ftdna-illumina
                   
                    char num[20];
                    int  nmindex = 0;
                    loopbreak = 0;
                    num[0] = NULL;

                    if (nbuffer[rdindex] == 'r' && nbuffer[rdindex + 1] == 's')
                    {
                        rdindex += 2;
                        while (isdigit(nbuffer[rdindex]) && rdindex < 24)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = NULL;
                        //First in the line is the RS number
                        snp[inx].rs = atoi(num);

                    }
                    else
                    {
                        //23toMe ixxxx codes ro rsxxx codes
                        int isw = 0;
                        nmindex = 0;
                        rdindex += 1; //Skip i
                                               
                        while (isdigit(nbuffer[rdindex]) && rdindex < 24)
                        {
                            num[nmindex] = nbuffer[rdindex];
                            rdindex++;
                            nmindex++;
                        }
                        num[nmindex] = NULL;
                       
                        isw = atoi(num); //in example  VG01S1077 ftdna will equal numeric 11007

                        switch (isw) {//covert Illumina Internatl codes to rsID equivenelts
                        case 3003626:  //CCR5 delta 32 mutation. imparts HIV1 immunity if Hetrozogous (but makes you more vunerable to flaviviruses and no immunity to HIV2
                            snp[inx].rs = 333;
                            illuminaT_++;
                            break;    // All codes beyond this point are ver. 5 codes 
                        case 4000478:
                            snp[inx].rs = 118203925;
                            illuminaT_++;
                            break;
                        case 4000479:
                            snp[inx].rs = 62516095;
                            illuminaT_++;
                            break;
                        case 4000481:
                            snp[inx].rs = 62508588;
                            illuminaT_++;
                            break;
                        case 4000635:
                            snp[inx].rs = 121434453;
                            illuminaT_++;
                            break;
                        case 4000834:
                            snp[inx].rs = 199476104;
                            illuminaT_++;
                            break;
                        case 4990151:
                            snp[inx].rs = 111033258;
                            illuminaT_++;
                            break;
                        case 5000002:
                            snp[inx].rs = 121908362;
                            illuminaT_++;
                            break;
                        case 5000003:
                            snp[inx].rs = 111033244;
                            illuminaT_++;
                            break;
                        case 5000043:
                            snp[inx].rs = 200511261;
                            illuminaT_++;
                            break;
                        case 5000047:
                            snp[inx].rs = 369925690;
                            illuminaT_++;
                            break;
                        case 5000119:
                            snp[inx].rs = 879254572;
                            illuminaT_++;
                            break;
                        case 5000064:
                            snp[inx].rs = 80338945;
                            illuminaT_++;
                            break;
                        case 5000092:
                            snp[inx].rs = 121907972;
                            illuminaT_++;
                            break;
                        case 5000121:
                            snp[inx].rs = 199774121;
                            illuminaT_++;
                            break;
                        case 5000142:
                            snp[inx].rs = 749038326;
                            illuminaT_++;
                            break;
                        case 5000663:
                            snp[inx].rs = 3810141;
                            illuminaT_++;
                            break;
                        case 5000693:
                            snp[inx].rs = 111033199;
                            illuminaT_++;
                            break;
                        case 5000696:
                            snp[inx].rs = 111033307;
                            illuminaT_++;
                            break;
                        case 5001996:
                            snp[inx].rs = 80338948;
                            illuminaT_++;
                            break;
                        case 5002001:
                            snp[inx].rs = 104894396;
                            illuminaT_++;
                            break;
                        case 5002096:
                            snp[inx].rs = 267607213;
                            illuminaT_++;
                            break;
                        case 5003116:
                            snp[inx].rs = 121434282;
                            illuminaT_++;
                            break;
                        case 5003117:
                            snp[inx].rs = 121434281;
                            illuminaT_++;
                            break;
                        case 5003700:
                            snp[inx].rs = 121964990;
                            illuminaT_++;
                            break;
                        case 5004213:
                            snp[inx].rs = 121918070;
                            illuminaT_++;
                            break;
                        case 5004778:
                            snp[inx].rs = 74315455;
                            illuminaT_++;
                            break;
                        case 5004781:
                            snp[inx].rs = 74315458;
                            illuminaT_++;
                            break;
                        case 5004790:
                            snp[inx].rs = 74315467;
                            illuminaT_++;
                            break;
                        case 5004829:
                            snp[inx].rs = 121907959;
                            illuminaT_++;
                            break;
                        case 5005735:
                            snp[inx].rs = 104893931;
                            illuminaT_++;
                            break;
                        case 5005799:
                            snp[inx].rs = 119103251;
                            illuminaT_++;
                            break;
                        case 5005800:
                            snp[inx].rs = 119103252;
                            illuminaT_++;
                            break;
                        case 5005927:
                            snp[inx].rs = 80338680;
                            illuminaT_++;
                            break;
                        case 5006058:
                            snp[inx].rs = 121908758;
                            illuminaT_++;
                            break;
                        case 5006076:
                            snp[inx].rs = 121908754;
                            illuminaT_++;
                            break;
                        case 5006092:
                            snp[inx].rs = 121908766;
                            illuminaT_++;
                            break;
                        case 5006103:
                            snp[inx].rs = 78802634;
                            illuminaT_++;
                            break;
                        case 5006112:
                            snp[inx].rs = 75115087;
                            illuminaT_++;
                            break;
                        case 5007145:
                            snp[inx].rs = 137853096;
                            illuminaT_++;
                            break;
                        case 5007146:
                            snp[inx].rs = 137853097;
                            illuminaT_++;
                            break;
                        case 5008215:
                            snp[inx].rs = 76917243;
                            illuminaT_++;
                            break;
                        case 5008436:
                            snp[inx].rs = 137852327;
                            illuminaT_++;
                            break;
                        case 5008440:
                            snp[inx].rs = 137852330;
                            illuminaT_++;
                            break;
                        case 5008441:
                            snp[inx].rs = 267606835;
                            illuminaT_++;
                            break;
                        case 5008450:
                            snp[inx].rs = 137852339;
                            illuminaT_++;
                            break;
                        case 5008462:
                            snp[inx].rs = 137852348;
                            illuminaT_++;
                            break;
                        case 5008753:
                            snp[inx].rs = 193922376;
                            illuminaT_++;
                            break;
                        case 5009095:
                            snp[inx].rs = 397507789;
                            illuminaT_++;
                            break;
                        case 5009097:
                            snp[inx].rs = 397507788;
                            illuminaT_++;
                            break;
                        case 5009099:
                            snp[inx].rs = 397507785;
                            illuminaT_++;
                            break;
                        case 5009107:
                            snp[inx].rs = 397507777;
                            illuminaT_++;
                            break;
                        case 5009119:
                            snp[inx].rs = 397507756;
                            illuminaT_++;
                            break;
                        case 5009120:
                            snp[inx].rs = 397507753;
                            illuminaT_++;
                            break;
                        case 5009136:
                            snp[inx].rs = 397507728;
                            illuminaT_++;
                            break;
                        case 5009151:
                            snp[inx].rs = 276174838;
                            illuminaT_++;
                            break;
                        case 5009161:
                            snp[inx].rs = 397507692;
                            illuminaT_++;
                            break;
                        case 5009162:
                            snp[inx].rs = 886040499;
                            illuminaT_++;
                            break;
                        case 5009163:
                            snp[inx].rs = 397507688;
                            illuminaT_++;
                            break;
                        case 5009165:
                            snp[inx].rs = 397507684;
                            illuminaT_++;
                            break;
                        case 5009173:
                            snp[inx].rs = 80359388;
                            illuminaT_++;
                            break;
                        case 5009174:
                            snp[inx].rs = 397507673;
                            illuminaT_++;
                            break;
                        case 5009175:
                            snp[inx].rs = 397507672;
                            illuminaT_++;
                            break;
                        case 5009179:
                            snp[inx].rs = 397507659;
                            illuminaT_++;
                            break;
                        case 5009215:
                            snp[inx].rs = 397507605;
                            illuminaT_++;
                            break;
                        case 5009223:
                            snp[inx].rs = 397507591;
                            illuminaT_++;
                            break;
                        case 5009244:
                            snp[inx].rs = 397507739;
                            illuminaT_++;
                            break;
                        case 5009256:
                            snp[inx].rs = 1801426;
                            illuminaT_++;
                            break;
                        case 5009299:
                            snp[inx].rs = 11571746;
                            illuminaT_++;
                            break;
                        case 5009331:
                            snp[inx].rs = 55716624;
                            illuminaT_++;
                            break;
                        case 5009334:
                            snp[inx].rs = 169547;
                            illuminaT_++;
                            break;
                        case 5010413:
                            snp[inx].rs = 397507940;
                            illuminaT_++;
                            break;
                        case 5010426:
                            snp[inx].rs = 397507986;
                            illuminaT_++;
                            break;
                        case 5010451:
                            snp[inx].rs = 397507420;
                            illuminaT_++;
                            break;
                        case 5010462:
                            snp[inx].rs = 80359771;
                            illuminaT_++;
                            break;
                        case 5010474:
                            snp[inx].rs = 397507830;
                            illuminaT_++;
                            break;
                        case 5010479:
                            snp[inx].rs = 80359266;
                            illuminaT_++;
                            break;
                        case 5010486:
                            snp[inx].rs = 397507613;
                            illuminaT_++;
                            break;
                        case 5010497:
                            snp[inx].rs = 80359380;
                            illuminaT_++;
                            break;
                        case 5010509:
                            snp[inx].rs = 397507724;
                            illuminaT_++;
                            break;
                        case 5010511:
                            snp[inx].rs = 397507732;
                            illuminaT_++;
                            break;
                        case 5010512:
                            snp[inx].rs = 276174847;
                            illuminaT_++;
                            break;
                        case 5010527:
                            snp[inx].rs = 397507812;
                            illuminaT_++;
                            break;
                        case 5010530:
                            snp[inx].rs = 397507826;
                            illuminaT_++;
                            break;
                        case 5010544:
                            snp[inx].rs = 397507874;
                            illuminaT_++;
                            break;
                        case 5010569:
                            snp[inx].rs = 397507419;
                            illuminaT_++;
                            break;
                        case 5010578:
                            snp[inx].rs = 276174852;
                            illuminaT_++;
                            break;
                        case 5010743:
                            snp[inx].rs = 368505753;
                            illuminaT_++;
                            break;
                        case 5010780:
                            snp[inx].rs = 74467662;
                            illuminaT_++;
                            break;
                        case 5010801:
                            snp[inx].rs = 397508461;
                            illuminaT_++;
                            break;
                        case 5010930:
                            snp[inx].rs = 80282562;
                            illuminaT_++;
                            break;
                        case 5010947:
                            snp[inx].rs = 78440224;
                            illuminaT_++;
                            break;
                        case 5010980:
                            snp[inx].rs = 397508778;
                            illuminaT_++;
                            break;
                        case 5011073:
                            snp[inx].rs = 79031340;
                            illuminaT_++;
                            break;
                        case 5011184:
                            snp[inx].rs = 79031340;
                            illuminaT_++;
                            break;
                        case 5011243:
                            snp[inx].rs = 397508211;
                            illuminaT_++;
                            break;
                        case 5011404:
                            snp[inx].rs = 397508296;
                            illuminaT_++;
                            break;
                        case 5011446:
                            snp[inx].rs = 121908759;
                            illuminaT_++;
                            break;
                        case 5011516:
                            snp[inx].rs = 397508353;
                            illuminaT_++;
                            break;
                        case 5011556:
                            snp[inx].rs = 397515498;
                            illuminaT_++;
                            break;
                        case 5011672:
                            snp[inx].rs = 397508442;
                            illuminaT_++;
                            break;
                        case 5011775:
                            snp[inx].rs = 397508500;
                            illuminaT_++;
                            break;
                        case 5011826:
                            snp[inx].rs = 202179988;
                            illuminaT_++;
                            break;
                        case 5011985:
                            snp[inx].rs = 387906362;
                            illuminaT_++;
                            break;
                        case 5012121:
                            snp[inx].rs = 193922525;
                            illuminaT_++;
                            break;
                        case 5012129:
                            snp[inx].rs = 397508675;
                            illuminaT_++;
                            break;
                        case 5012573:
                            snp[inx].rs = 515726215;
                            illuminaT_++;
                            break;
                        case 5012575:
                            snp[inx].rs = 121908428;
                            illuminaT_++;
                            break;
                        case 5012578:
                            snp[inx].rs = 281865117;
                            illuminaT_++;
                            break;
                        case 5012610:
                            snp[inx].rs = 398124502;
                            illuminaT_++;
                            break;
                        case 5012612:
                            snp[inx].rs = 137852949;
                            illuminaT_++;
                            break;
                        case 5012616:
                            snp[inx].rs = 80338848;
                            illuminaT_++;
                            break;
                        case 5012618:
                            snp[inx].rs = 28939086;
                            illuminaT_++;
                            break;
                        case 5012622:
                            snp[inx].rs = 137852696;
                            illuminaT_++;
                            break;
                        case 5012623:
                            snp[inx].rs = 137852695;
                            illuminaT_++;
                            break;
                        case 5012624:
                            snp[inx].rs = 137852700;
                            illuminaT_++;
                            break;
                        case 5012628:
                            snp[inx].rs = 80356708;
                            illuminaT_++;
                            break;
                        case 5012629:
                            snp[inx].rs = 180177309;
                            illuminaT_++;
                            break;
                        case 5012634:
                            snp[inx].rs = 80338794;
                            illuminaT_++;
                            break;
                        case 5012660:
                            snp[inx].rs = 28937590;
                            illuminaT_++;
                            break;
                        case 5012664:
                            snp[inx].rs = 78340951;
                            illuminaT_++;
                            break;
                        case 5012665:
                            snp[inx].rs = 387906225;
                            illuminaT_++;
                            break;
                        case 5012669:
                            snp[inx].rs = 80356680;
                            illuminaT_++;
                            break;
                        case 5012671:
                            snp[inx].rs = 80356681;
                            illuminaT_++;
                            break;
                        case 5012672:
                            snp[inx].rs = 80356682;
                            illuminaT_++;
                            break;
                        case 5012678:
                            snp[inx].rs = 386833969;
                            illuminaT_++;
                            break;
                        case 5012679:
                            snp[inx].rs = 80338701;
                            illuminaT_++;
                            break;
                        case 5012680:
                            snp[inx].rs = 28936415;
                            illuminaT_++;
                            break;
                        case 5012683:
                            snp[inx].rs = 121918008;
                            illuminaT_++;
                            break;
                        case 5012686:
                            snp[inx].rs = 387906525;
                            illuminaT_++;
                            break;
                        case 5012688:
                            snp[inx].rs = 61750420;
                            illuminaT_++;
                            break;
                        case 5012709:
                            snp[inx].rs = 35887622;
                            illuminaT_++;
                            break;
                        case 5012749:
                            snp[inx].rs = 119466000;
                            illuminaT_++;
                            break;
                        case 5012758:
                            snp[inx].rs = 121434280;
                            illuminaT_++;
                            break;
                        case 5012759:
                            snp[inx].rs = 77931234;
                            illuminaT_++;
                            break;
                        case 5012760:
                            snp[inx].rs = 373715782;
                            illuminaT_++;
                            break;
                        case 5012761:
                            snp[inx].rs = 200724875;
                            illuminaT_++;
                            break;
                        case 5012765:
                            snp[inx].rs = 28940893;
                            illuminaT_++;
                            break;
                        case 5012770:
                            snp[inx].rs = 587776650;
                            illuminaT_++;
                            break;
                        case 5012799:
                            snp[inx].rs = 28941770;
                            illuminaT_++;
                            break;
                        case 5012804:
                            snp[inx].rs = 111033260;
                            illuminaT_++;
                            break;
                        case 5012861:
                            snp[inx].rs = 80338898;
                            illuminaT_++;
                            break;
                        case 5012862:
                            snp[inx].rs = 80338899;
                            illuminaT_++;
                            break;
                        case 5012864:
                            snp[inx].rs = 121965075;
                            illuminaT_++;
                            break;
                        case 5012865:
                            snp[inx].rs = 80338901;
                            illuminaT_++;
                            break;
                        case 5012867:
                            snp[inx].rs = 80338895;
                            illuminaT_++;
                            break;
                        case 5012878:
                            snp[inx].rs = 80356489;
                            illuminaT_++;
                            break;
                        case 5012880:
                            snp[inx].rs = 80356491;
                            illuminaT_++;
                            break;
                        case 5037820:
                            snp[inx].rs = 267608126;
                            illuminaT_++;
                            break;
                        case 5037831:
                            snp[inx].rs = 863225414;
                            illuminaT_++;
                            break;
                        case 5037832:
                            snp[inx].rs = 786201855;
                            illuminaT_++;
                            break;
                        case 5037835:
                            snp[inx].rs = 267608101;
                            illuminaT_++;
                            break;
                        case 5037850:
                            snp[inx].rs = 267608037;
                            illuminaT_++;
                            break;
                        case 5037858:
                            snp[inx].rs = 267608050;
                            illuminaT_++;
                            break;
                        case 5037866:
                            snp[inx].rs = 267608114;
                            illuminaT_++;
                            break;
                        case 5037874:
                            snp[inx].rs = 267608090;
                            illuminaT_++;
                            break;
                        case 5037877:
                            snp[inx].rs = 267608085;
                            illuminaT_++;
                            break;
                        case 5037884:
                            snp[inx].rs = 267608072;
                            illuminaT_++;
                            break;
                        case 5037895:
                            snp[inx].rs = 193922343;
                            illuminaT_++;
                            break;
                        case 5041058:
                            snp[inx].rs = 72658154;
                            illuminaT_++;
                            break;
                        case 5046667:
                            snp[inx].rs = 876660428;
                            illuminaT_++;
                            break;
                        case 5047658:
                            snp[inx].rs = 63750215;
                            illuminaT_++;
                            break;
                        case 5048984:
                            snp[inx].rs = 72551348;
                            illuminaT_++;
                            break;
                        case 5049013:
                            snp[inx].rs = 139607673;
                            illuminaT_++;
                            break;
                        case 5050964:
                            snp[inx].rs = 199474658;
                            illuminaT_++;
                            break;
                        case 5050984:
                            snp[inx].rs = 199474657;
                            illuminaT_++;
                            break;
                        case 5050994:
                            snp[inx].rs = 267606617;
                            illuminaT_++;
                            break;
                        case 5053826:
                            snp[inx].rs = 79031340;
                            illuminaT_++;
                            break;
                        case 5053827:
                            snp[inx].rs = 77409459;
                            illuminaT_++;
                            break;
                        case 5053833:
                            snp[inx].rs = 75115087;
                            illuminaT_++;
                            break;
                        case 5053834:
                            snp[inx].rs = 79633941;
                            illuminaT_++;
                            break;
                        case 5053836:
                            snp[inx].rs = 78194216;
                            illuminaT_++;
                            break;
                        case 5053839:
                            snp[inx].rs = 78802634;
                            illuminaT_++;
                            break;
                        case 5053851:
                            snp[inx].rs = 78440224;
                            illuminaT_++;
                            break;
                        case 5053895:
                            snp[inx].rs = 76151636;
                            illuminaT_++;
                            break;
                        case 6007566:
                            snp[inx].rs = 6151429;
                            illuminaT_++;
                            break;
                        case 6007689:
                            snp[inx].rs = 398123126;
                            illuminaT_++;
                            break;
                        case 6007710:
                            snp[inx].rs = 367543005;
                            illuminaT_++;
                            break;
                        case 6009255:
                            snp[inx].rs = 515726176;
                            illuminaT_++;
                            break;
                        case 6009259:
                            snp[inx].rs = 515726174;
                            illuminaT_++;
                            break;
                        case 6009276:
                            snp[inx].rs = 515726175;
                            illuminaT_++;
                            break;
                        case 6014100:
                            snp[inx].rs = 879254382;
                            illuminaT_++;
                            break;
                        case 6014141:
                            snp[inx].rs = 201573863;
                            illuminaT_++;
                            break;
                        case 6014161:
                            snp[inx].rs = 370471092;
                            illuminaT_++;
                            break;
                        case 6014217:
                            snp[inx].rs = 773658037;
                            illuminaT_++;
                            break;
                        case 6014229:
                            snp[inx].rs = 544453230;
                            illuminaT_++;
                            break;
                        case 6014299:
                            snp[inx].rs = 879254662;
                            illuminaT_++;
                            break;
                        case 6014548:
                            snp[inx].rs = 879255045;
                            illuminaT_++;
                            break;
                        case 6014573:
                            snp[inx].rs = 875989911;
                            illuminaT_++;
                            break;
                        case 6015375:
                            snp[inx].rs = 992677795;
                            illuminaT_++;
                            break;
                        case 6016654:
                            snp[inx].rs = 200511261;
                            illuminaT_++;
                            break;
                        case 6024270:
                            snp[inx].rs = 774159791;
                            illuminaT_++;
                            break;
                        case 700014:
                            snp[inx].rs = 879255033;
                            illuminaT_++;
                            break;
                        case 700020:
                            snp[inx].rs = 879253834;
                            illuminaT_++;
                            break;
                        case 700076:
                            snp[inx].rs = 767388144;
                            illuminaT_++;
                            break;
                        case 700184:
                            snp[inx].rs = 367543006;
                            illuminaT_++;
                            break;
                        case 700566:
                            snp[inx].rs = 887126161;
                            illuminaT_++;
                            break;
                        case 700568:
                            snp[inx].rs = 863225434;
                            illuminaT_++;
                            break;
                        case 700621:
                            snp[inx].rs = 1135401729;
                            illuminaT_++;
                            break;
                        case 700645:
                            snp[inx].rs = 781986930;
                            illuminaT_++;
                            break;
                        case 700947:
                            snp[inx].rs = 1085307466;
                            illuminaT_++;
                            break;
                        case 700994:
                            snp[inx].rs = 879254740;
                            illuminaT_++;
                            break;
                        case 701087:
                            snp[inx].rs = 879255213;
                            illuminaT_++;
                            break;
                        case 701112:
                            snp[inx].rs = 879255281;
                            illuminaT_++;
                            break;
                        case 701164:
                            snp[inx].rs = 879254412;
                            illuminaT_++;
                            break;
                        case 701180:
                            snp[inx].rs = 879254761;
                            illuminaT_++;
                            break;
                        case 701345:
                            snp[inx].rs = 879254817;
                            illuminaT_++;
                            break;
                        case 701347:
                            snp[inx].rs = 879254886;
                            illuminaT_++;
                            break;
                        case 701362:
                            snp[inx].rs = 879254434;
                            illuminaT_++;
                            break;
                        case 701371:
                            snp[inx].rs = 879254884;
                            illuminaT_++;
                            break;
                        case 701383:
                            snp[inx].rs = 879254443;
                            illuminaT_++;
                            break;
                        case 701409:
                            snp[inx].rs = 1419032418;
                            illuminaT_++;
                            break;
                        case 701414:
                            snp[inx].rs = 879254410;
                            illuminaT_++;
                            break;
                        case 701450:
                            snp[inx].rs = 879254841;
                            illuminaT_++;
                            break;
                        case 701496:
                            snp[inx].rs = 879255105;
                            illuminaT_++;
                            break;
                        case 701505:
                            snp[inx].rs = 879254521;
                            illuminaT_++;
                            break;
                        case 701811:
                            snp[inx].rs = 879254708;
                            illuminaT_++;
                            break;
                        case 702110:
                            snp[inx].rs = 879254474;
                            illuminaT_++;
                            break;
                        case 702255:
                            snp[inx].rs = 1461905374;
                            illuminaT_++;
                            break;
                        case 702310:
                            snp[inx].rs = 879254899;
                            illuminaT_++;
                            break;
                        case 702329:
                            snp[inx].rs = 781319410;
                            illuminaT_++;
                            break;
                        case 702373:
                            snp[inx].rs = 879254651;
                            illuminaT_++;
                            break;
                        case 702455:
                            snp[inx].rs = 879255282;
                            illuminaT_++;
                            break;
                        case 702480:
                            snp[inx].rs = 879254687;
                            illuminaT_++;
                            break;
                        case 702563:
                            snp[inx].rs = 879254404;
                            illuminaT_++;
                            break;
                        case 702565:
                            snp[inx].rs = 879255026;
                            illuminaT_++;
                            break;
                        case 703107:
                            snp[inx].rs = 879254393;
                            illuminaT_++;
                            break;
                        case 703167:
                            snp[inx].rs = 1553137543;
                            illuminaT_++;
                            break;
                        case 703267:
                            snp[inx].rs = 879254397;
                            illuminaT_++;
                            break;
                        case 703316:
                            snp[inx].rs = 879255311;
                            illuminaT_++;
                            break;
                        case 703322:
                            snp[inx].rs = 879254574;
                            illuminaT_++;
                            break;
                        case 703359:
                            snp[inx].rs = 879254702;
                            illuminaT_++;
                            break;
                        case 703379:
                            snp[inx].rs = 879255029;
                            illuminaT_++;
                            break;
                        case 703467:
                            snp[inx].rs = 879254673;
                            illuminaT_++;
                            break;
                        case 703601:
                            snp[inx].rs = 879254653;
                            illuminaT_++;
                            break;
                        case 703652:
                            snp[inx].rs = 879254896;
                            illuminaT_++;
                            break;
                        case 703668:
                            snp[inx].rs = 879254435;
                            illuminaT_++;
                            break;
                        case 703692:
                            snp[inx].rs = 879254614;
                            illuminaT_++;
                            break;
                        case 703728:
                            snp[inx].rs = 879254710;
                            illuminaT_++;
                            break;
                        case 703733:
                            snp[inx].rs = 879255320;
                            illuminaT_++;
                            break;
                        case 703821:
                            snp[inx].rs = 879254666;
                            illuminaT_++;
                            break;
                        case 703873:
                            snp[inx].rs = 879254923;
                            illuminaT_++;
                            break;
                        case 703916:
                            snp[inx].rs = 876659708;
                            illuminaT_++;
                            break;
                        case 703960:
                            snp[inx].rs = 879255224;
                            illuminaT_++;
                            break;
                        case 703963:
                            snp[inx].rs = 879254491;
                            illuminaT_++;
                            break;
                        case 704105:
                            snp[inx].rs = 1555803643;
                            illuminaT_++;
                            break;
                        case 704128:
                            snp[inx].rs = 879254436;
                            illuminaT_++;
                            break;
                        case 704190:
                            snp[inx].rs = 879254958;
                            illuminaT_++;
                            break;
                        case 704225:
                            snp[inx].rs = 879254384;
                            illuminaT_++;
                            break;
                        case 704231:
                            snp[inx].rs = 397515991;
                            illuminaT_++;
                            break;
                        case 704270:
                            snp[inx].rs = 397516073;
                            illuminaT_++;
                            break;
                        case 704424:
                            snp[inx].rs = 879254123;
                            illuminaT_++;
                            break;
                        case 704512:
                            snp[inx].rs = 879255175;
                            illuminaT_++;
                            break;
                        case 704677:
                            snp[inx].rs = 879254582;
                            illuminaT_++;
                            break;
                        case 704827:
                            snp[inx].rs = 879254573;
                            illuminaT_++;
                            break;
                        case 704874:
                            snp[inx].rs = 879254888;
                            illuminaT_++;
                            break;
                        case 704887:
                            snp[inx].rs = 879254821;
                            illuminaT_++;
                            break;
                        case 704902:
                            snp[inx].rs = 879254922;
                            illuminaT_++;
                            break;
                        case 704916:
                            snp[inx].rs = 879254552;
                            illuminaT_++;
                            break;
                        case 704986:
                            snp[inx].rs = 879253965;
                            illuminaT_++;
                            break;
                        case 704992:
                            snp[inx].rs = 879254771;
                            illuminaT_++;
                            break;
                        case 705007:
                            snp[inx].rs = 879254799;
                            illuminaT_++;
                            break;
                        case 705032:
                            snp[inx].rs = 879254569;
                            illuminaT_++;
                            break;
                        case 705059:
                            snp[inx].rs = 875989898;
                            illuminaT_++;
                            break;
                        case 705119:
                            snp[inx].rs = 879254591;
                            illuminaT_++;
                            break;
                        case 705179:
                            snp[inx].rs = 879254538;
                            illuminaT_++;
                            break;
                        case 705234:
                            snp[inx].rs = 1060499931;
                            illuminaT_++;
                            break;
                        case 705258:
                            snp[inx].rs = 879254413;
                            illuminaT_++;
                            break;
                        case 705313:
                            snp[inx].rs = 879254701;
                            illuminaT_++;
                            break;
                        case 705428:
                            snp[inx].rs = 879254388;
                            illuminaT_++;
                            break;
                        case 705498:
                            snp[inx].rs = 879254138;
                            illuminaT_++;
                            break;
                        case 705502:
                            snp[inx].rs = 879254885;
                            illuminaT_++;
                            break;
                        case 705645:
                            snp[inx].rs = 879254440;
                            illuminaT_++;
                            break;
                        case 705657:
                            snp[inx].rs = 879254581;
                            illuminaT_++;
                            break;
                        case 705698:
                            snp[inx].rs = 879254882;
                            illuminaT_++;
                            break;
                        case 705710:
                            snp[inx].rs = 879254594;
                            illuminaT_++;
                            break;
                        case 705843:
                            snp[inx].rs = 879254387;
                            illuminaT_++;
                            break;
                        case 705867:
                            snp[inx].rs = 879254512;
                            illuminaT_++;
                            break;
                        case 705904:
                            snp[inx].rs = 80357634;
                            illuminaT_++;
                            break;
                        case 705932:
                            snp[inx].rs = 879254402;
                            illuminaT_++;
                            break;
                        case 706122:
                            snp[inx].rs = 879254027;
                            illuminaT_++;
                            break;
                        case 706178:
                            snp[inx].rs = 879254116;
                            illuminaT_++;
                            break;
                        case 706241:
                            snp[inx].rs = 879254952;
                            illuminaT_++;
                            break;
                        case 706291:
                            snp[inx].rs = 879255031;
                            illuminaT_++;
                            break;
                        case 706299:
                            snp[inx].rs = 879254575;
                            illuminaT_++;
                            break;
                        case 706312:
                            snp[inx].rs = 879254768;
                            illuminaT_++;
                            break;
                        case 706314:
                            snp[inx].rs = 879254122;
                            illuminaT_++;
                            break;
                        case 706344:
                            snp[inx].rs = 730881471;
                            illuminaT_++;
                            break;
                        case 706367:
                            snp[inx].rs = 879254736;
                            illuminaT_++;
                            break;
                        case 706371:
                            snp[inx].rs = 879254737;
                            illuminaT_++;
                            break;
                        case 706379:
                            snp[inx].rs = 879254530;
                            illuminaT_++;
                            break;
                        case 706465:
                            snp[inx].rs = 879254748;
                            illuminaT_++;
                            break;
                        case 706566:
                            snp[inx].rs = 879254675;
                            illuminaT_++;
                            break;
                        case 706617:
                            snp[inx].rs = 879254494;
                            illuminaT_++;
                            break;
                        case 706659:
                            snp[inx].rs = 879254111;
                            illuminaT_++;
                            break;
                        case 706716:
                            snp[inx].rs = 879254980;
                            illuminaT_++;
                            break;
                        case 706888:
                            snp[inx].rs = 879254580;
                            illuminaT_++;
                            break;
                        case 709033:
                            snp[inx].rs = 864622557;
                            illuminaT_++;
                            break;
                        case 709034:
                            snp[inx].rs = 864622558;
                            illuminaT_++;
                            break;
                        case 709063:
                            snp[inx].rs = 80359264;
                            illuminaT_++;
                            break;
                        case 709064:
                            snp[inx].rs = 864622609;
                            illuminaT_++;
                            break;
                        case 709068:
                            snp[inx].rs = 864622073;
                            illuminaT_++;
                            break;
                        case 709069:
                            snp[inx].rs = 864622082;
                            illuminaT_++;
                            break;
                        case 709070:
                            snp[inx].rs = 864622200;
                            illuminaT_++;
                            break;
                        case 709076:
                            snp[inx].rs = 864622277;
                            illuminaT_++;
                            break;
                        case 709466:
                            snp[inx].rs = 150074056;
                            illuminaT_++;
                            break;
                        case 712972:
                            snp[inx].rs = 864622651;
                            illuminaT_++;
                            break;
                        case 723120:
                            snp[inx].rs = 864622480;
                            illuminaT_++;
                            break;
                        case 723181:
                            snp[inx].rs = 864622257;
                            illuminaT_++;
                            break;
                        case 723189:
                            snp[inx].rs = 864622161;
                            illuminaT_++;
                            break;
                        case 723191:
                            snp[inx].rs = 864622431;
                            illuminaT_++;
                            break;
                        case 723194:
                            snp[inx].rs = 864622509;
                            illuminaT_++;
                            break;
                        case 723202:
                            snp[inx].rs = 864622192;
                            illuminaT_++;
                            break;
                        case 723205:
                            snp[inx].rs = 864622481;
                            illuminaT_++;
                            break;
                        case 723234:
                            snp[inx].rs = 864622707;
                            illuminaT_++;
                            break;

                        default:illuminaU_++; //untranslaned lines
                            break;
                        }

                    }
                    if (snp[inx].rs > 0) //stop line load of uniterpreted i codes
                    {
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
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < 256)
                        {
                            rdindex++;
                        }
                        snp[inx].a = nbuffer[rdindex];
                        rdindex++;
                        //As the last two values are not numeric we have to rely on the file still being TAB delimited
                        while (nbuffer[rdindex] != 'A' && nbuffer[rdindex] != 'C' && nbuffer[rdindex] != 'G' && nbuffer[rdindex] != 'T'
                            && nbuffer[rdindex] != 'D' && nbuffer[rdindex] != 'I' && nbuffer[rdindex] != '-' && rdindex < 256)
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

void  SnipParser::FConvert(void)
{


    //std::fstream  fs(fi_, std::ios_base::in | std::ios_base::binary);
   /* wchar_t* fiIn_;
    wchar_t* fiOut_; */
    wchar_t  codeIn_[] = L"f:\\icode.txt";
    wchar_t  codeOut_[] = L"f:\\ccode.txt";


    std::fstream  fs;
    std::fstream  fsOut;
    if (!fs.bad() && !fsOut.bad())
    {
        char nbuffer[256];
        std::string  lbuffer;
        char num[100];
        int loopbreak = 0;
        int fdind = 0;
        //Open file for read 
        fs.open(codeIn_, std::ios::in);
        //Open file for read 
        fsOut.open(codeOut_, std::ios::out);
        //Check file was opened  
        if (fs.is_open() && fsOut.is_open()) {
            fdind = 0;

            while (!fs.eof())
            {
                int rdindex = 0;
                int nmindex = 0;
                fs.getline(nbuffer, 256);
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
                //START constructing the write line
                lbuffer = "case " + std::string(num) + ":" + char(13) + "  snp[inx].rs = ";
                //read second rsID number
                //move past tab or spaces to next numeric data position number
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
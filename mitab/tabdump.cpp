/**********************************************************************
 * $Id: tabdump.cpp,v 1.3 1999-12-14 02:24:41 daniel Exp $
 *
 * Name:     tabdump.cpp
 * Project:  MapInfo TAB format Read/Write library
 * Language: C++
 * Purpose:  Test various part of the lib., and generate binary dumps.
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, Daniel Morissette
 *
 * All rights reserved.  This software may be copied or reproduced, in
 * all or in part, without the prior written consent of its author,
 * Daniel Morissette (danmo@videotron.ca).  However, any material copied
 * or reproduced must bear the original copyright notice (above), this 
 * original paragraph, and the original disclaimer (below).
 * 
 * The entire risk as to the results and performance of the software,
 * supporting text and other information contained in this file
 * (collectively called the "Software") is with the user.  Although 
 * considerable efforts have been used in preparing the Software, the 
 * author does not warrant the accuracy or completeness of the Software.
 * In no event will the author be liable for damages, including loss of
 * profits or consequential damages, arising out of the use of the 
 * Software.
 * 
 **********************************************************************
 *
 * $Log: tabdump.cpp,v $
 * Revision 1.3  1999-12-14 02:24:41  daniel
 * Use IMapInfoFile::SmartOpen()
 *
 * Revision 1.2  1999/09/20 14:27:49  warmerda
 * Use access of "rb" instead of "r".
 *
 * Revision 1.1  1999/07/12 04:18:26  daniel
 * Initial checkin
 *
 **********************************************************************/


#include "mitab.h"
#include <ctype.h>

static int DumpMapFileBlocks(const char *pszFname);
static int DumpMapFileObjects(const char *pszFname);

static int DumpIndFileObjects(const char *pszFname);

static int DumpTabFile(const char *pszFname);


#define TABTEST_USAGE "Usage: tabtest -a|-all     <filename>\n" \
                      "       tabtest -b|-blocks  <filename>\n" \
                      "       tabtest -o|-objects <filename>\n"

/**********************************************************************
 *                          main()
 *
 * This program is used to dump binary files (default behavior), and to
 * test some parts of the lib. during the development process. 
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszFname;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<3)
    {
        printf("%s", TABTEST_USAGE);
        return 1;
    }
    else
    {
        pszFname = argv[2];
    }
    
/*---------------------------------------------------------------------
 *      With option -blocks <filename>
 *      Open file, and dump each block sequentially.
 *--------------------------------------------------------------------*/
    if (EQUALN(argv[1], "-blocks", 2))
    {

        if (strstr(pszFname, ".map") != NULL ||
            strstr(pszFname, ".MAP") != NULL)
        {
            DumpMapFileBlocks(pszFname);
        }
        else if (strstr(pszFname, ".ind") != NULL ||
                 strstr(pszFname, ".IND") != NULL)
        {
            DumpIndFileObjects(pszFname);
        }
        else if (strstr(pszFname, ".otherextension") != NULL)
        {
            ;
        }
    }
/*---------------------------------------------------------------------
 *      With option -objects <filename>
 *      Open file, and all geogr. objects.
 *--------------------------------------------------------------------*/
    else if (EQUALN(argv[1], "-objects", 2))
    {

        if (strstr(pszFname, ".map") != NULL ||
            strstr(pszFname, ".MAP") != NULL)
        {
            DumpMapFileObjects(pszFname);
        }
        else if (strstr(pszFname, ".tab") != NULL ||
                 strstr(pszFname, ".TAB") != NULL)
        {
            DumpTabFile(pszFname);
        }
        else if (strstr(pszFname, ".ind") != NULL ||
                 strstr(pszFname, ".IND") != NULL)
        {
            DumpIndFileObjects(pszFname);
        }
    }
/*---------------------------------------------------------------------
 *      With option -all <filename>
 *      Dump the whole TAB dataset (all supported files)
 *--------------------------------------------------------------------*/
    else if (EQUALN(argv[1], "-all", 2))
    {

        if (strstr(pszFname, ".tab") != NULL ||
            strstr(pszFname, ".TAB") != NULL)
        {
            DumpTabFile(pszFname);
        }
    }
/*---------------------------------------------------------------------
 *     With option -otheroption <filename> ... 
 *     ... do something else ...
 *--------------------------------------------------------------------*/
    else if (EQUALN(argv[1], "-otheroption", 2))
    {
        ;
    }
    else
    {
        printf("Cannot process file %s\n\n", pszFname);
        printf("%s", TABTEST_USAGE);
        
        return 1;
    }

    return 0;
}


/**********************************************************************
 *                          DumpMapFileBlocks()
 *
 * Read and dump a .MAP file... simply dump all blocks sequentially.
 **********************************************************************/
static int DumpMapFileBlocks(const char *pszFname)
{
    FILE        *fp;
    TABRawBinBlock *poBlock;
    int         nOffset = 0;
    VSIStatBuf  sStatBuf;

    /*---------------------------------------------------------------------
     * Try to open source file
     * Note: we use stat() to fetch the file size.
     *--------------------------------------------------------------------*/
    if ( VSIStat(pszFname, &sStatBuf) == -1 )
    {
        printf("stat() failed for %s\n", pszFname);
        return -1;
    }

    fp = fopen(pszFname, "rb");
    if (fp == NULL)
    {
        printf("Failed to open %s\n", pszFname);
        return -1;
    }


    /*---------------------------------------------------------------------
     * Read/Dump blocks until EOF is reached
     *--------------------------------------------------------------------*/
    while (nOffset < sStatBuf.st_size )
    {
        poBlock = TABCreateMAPBlockFromFile(fp, nOffset, 512);

        if (poBlock)
        {
            poBlock->Dump();
            printf("\n");
            delete poBlock;
        }
        else
        {
            // An error happened (could be EOF)... abort now.
            break;
        }

        nOffset += 512;
    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    fclose(fp);

    return 0;
}


/**********************************************************************
 *                          DumpMapFileObjects()
 *
 * Open a .MAP file and print all the geogr. objects found.
 **********************************************************************/
static int DumpMapFileObjects(const char *pszFname)
{
    TABMAPFile  oMAPFile;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if (oMAPFile.Open(pszFname, "rb") != 0)
    {
        printf("Failed to open %s\n", pszFname);
        return -1;
    }

    oMAPFile.Dump();

    /*---------------------------------------------------------------------
     * Read/Dump objects until EOF is reached
     *--------------------------------------------------------------------*/
    while ( 0 )
    {

    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    oMAPFile.Close();

    return 0;
}

/**********************************************************************
 *                          DumpTabFile()
 *
 * Open a .TAB file and print all the geogr. objects found.
 **********************************************************************/
static int DumpTabFile(const char *pszFname)
{
    IMapInfoFile  *poFile;
    int      nFeatureId;
    TABFeature *poFeature;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if ((poFile = IMapInfoFile::SmartOpen(pszFname)) == NULL)
    {
        printf("Failed to open %s\n", pszFname);
        return -1;
    }

    poFile->Dump();

    /*---------------------------------------------------------------------
     * Read/Dump objects until EOF is reached
     *--------------------------------------------------------------------*/
    nFeatureId = -1;
    while ( (nFeatureId = poFile->GetNextFeatureId(nFeatureId)) != -1 )
    {
        poFeature = poFile->GetFeatureRef(nFeatureId);
        if (poFeature)
        {
//            poFeature->DumpReadable(stdout);
            printf("\nFeature %d:\n", nFeatureId);
            poFeature->DumpMID();
            poFeature->DumpMIF();
        }
        else
            break;      // GetFeatureRef() failed: Abort the loop
    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    poFile->Close();

    delete poFile;

    return 0;
}


/**********************************************************************
 *                          DumpIndFileObjects()
 *
 * Read and dump a .IND file
 **********************************************************************/
static int DumpIndFileObjects(const char *pszFname)
{
    TABINDFile  oINDFile;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if (oINDFile.Open(pszFname, "rb") != 0)
    {
        printf("Failed to open %s\n", pszFname);
        return -1;
    }

    oINDFile.Dump();

    /*---------------------------------------------------------------------
     * Read/Dump objects until EOF is reached
     *--------------------------------------------------------------------*/
    while ( 0 )
    {

    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    oINDFile.Close();

    return 0;
}


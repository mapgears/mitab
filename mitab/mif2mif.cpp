/**********************************************************************
 * $Id: mif2mif.cpp,v 1.2 2000-01-15 22:30:43 daniel Exp $
 *
 * Name:     tab2tab.cpp
 * Project:  MapInfo TAB format Read/Write library
 * Language: C++
 * Purpose:  Copy features from a .(MID/MIF) dataset to a new one.
 * Author:   Stephane Villeneuve, stephane.v@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, 2000, Stephane Villeneuve
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************
 *
 * $Log: mif2mif.cpp,v $
 * Revision 1.2  2000-01-15 22:30:43  daniel
 * Switch to MIT/X-Consortium OpenSource license
 *
 * Revision 1.1  1999/11/08 19:16:22  stephane
 * first revision
 *
 * Revision 1.1  1999/11/08 04:16:07  stephane
 * First Revision
 *
 **********************************************************************/


#include "mitab.h"
#include <ctype.h>

static int Mif2Mif(const char *pszSrcFname, const char *pszDstFname);


#define MIF2MIF_USAGE "Usage: %s <src_filename> <dst_filename>\n"

/**********************************************************************
 *                          main()
 *
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszSrcFname, *pszDstFname;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<3)
    {
        printf(MIF2MIF_USAGE,argv[0]);
        return 1;
    }
    else
    {
        pszSrcFname = argv[1];
        pszDstFname = argv[2];
    }
    

    return Mif2Mif(pszSrcFname, pszDstFname);
}


/**********************************************************************
 *                          Mid2Mid()
 *
 * Copy features from source dataset to a new dataset
 **********************************************************************/
static int Mif2Mif(const char *pszSrcFname, const char *pszDstFname)
{
    MIFFile  oSrcFile, oDstFile;
    int      nFeatureId;
    TABFeature *poFeature;
    double dXMin, dYMin, dXMax, dYMax;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if (oSrcFile.Open(pszSrcFname, "rb") != 0)
    {
        printf("Failed to open %s\n", pszSrcFname);
        return -1;
    }

    /*---------------------------------------------------------------------
     * Try to open destination file
     *--------------------------------------------------------------------*/
    if (oDstFile.Open(pszDstFname, "wb") != 0)
    {
        printf("Failed to open %s\n", pszDstFname);
        return -1;
    }

    //  Set bounds
    if (oSrcFile.GetBounds(dXMin, dYMin, dXMax, dYMax) == 0)
        oDstFile.SetBounds(dXMin, dYMin, dXMax, dYMax);

    // Pass Proj. info directly
    TABProjInfo sProjInfo;
    if (oSrcFile.GetProjInfo(&sProjInfo) == 0)
        oDstFile.SetProjInfo(&sProjInfo);

    /*---------------------------------------------------------------------
     * Copy objects until EOF is reached
     *--------------------------------------------------------------------*/
    nFeatureId = -1;
    while ( (nFeatureId = oSrcFile.GetNextFeatureId(nFeatureId)) != -1 )
    {
        poFeature = oSrcFile.GetFeatureRef(nFeatureId);
        if (poFeature)
        {
//            poFeature->DumpReadable(stdout);
//            poFeature->DumpMIF();
            oDstFile.SetFeature(poFeature);
        }
        else
            break;      // GetFeatureRef() failed: Abort the loop
    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    oDstFile.Close();

    oSrcFile.Close();

    return 0;
}



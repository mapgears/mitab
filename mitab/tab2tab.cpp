/**********************************************************************
 * $Id: tab2tab.cpp,v 1.6 2000-02-28 17:13:48 daniel Exp $
 *
 * Name:     tab2tab.cpp
 * Project:  MapInfo TAB format Read/Write library
 * Language: C++
 * Purpose:  Copy features from a .TAB dataset to a new one.
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, 2000, Daniel Morissette
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
 * $Log: tab2tab.cpp,v $
 * Revision 1.6  2000-02-28 17:13:48  daniel
 * Support for creating TABViews, and pass complete indexed field information
 *
 * Revision 1.5  2000/01/15 22:30:45  daniel
 * Switch to MIT/X-Consortium OpenSource license
 *
 * Revision 1.4  1999/12/14 02:24:20  daniel
 * *** empty log message ***
 *
 * Revision 1.3  1999/10/06 13:25:25  daniel
 * Pass file bounds
 *
 * Revision 1.2  1999/10/01 03:43:36  daniel
 * Pass ProjInfo
 *
 * Revision 1.1  1999/09/26 14:59:38  daniel
 * Implemented write support
 *
 **********************************************************************/


#include "mitab.h"
#include <ctype.h>

static int Tab2Tab(const char *pszSrcFname, const char *pszDstFname);


#define TAB2TAB_USAGE "Usage: tab2tab <src_filename> <dst_filename>\n"

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
        printf("%s", TAB2TAB_USAGE);
        return 1;
    }
    else
    {
        pszSrcFname = argv[1];
        pszDstFname = argv[2];
    }
    

    return Tab2Tab(pszSrcFname, pszDstFname);
}


/**********************************************************************
 *                          Tab2Tab()
 *
 * Copy features from source dataset to a new dataset
 **********************************************************************/
static int Tab2Tab(const char *pszSrcFname, const char *pszDstFname)
{
    IMapInfoFile *poSrcFile = NULL, *poDstFile = NULL;
    int      nFeatureId, iField;
    TABFeature *poFeature;
    double dXMin, dYMin, dXMax, dYMax;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if ((poSrcFile = IMapInfoFile::SmartOpen(pszSrcFname)) == NULL)
    {
        printf("Failed to open %s\n", pszSrcFname);
        return -1;
    }

    OGRFeatureDefn *poDefn = poSrcFile->GetLayerDefn();

    /*---------------------------------------------------------------------
     * Find out if the file contains at least 1 unique field... if so we will
     * create a TABView insterad of a TABFile
     *--------------------------------------------------------------------*/
    GBool    bFoundUniqueField = FALSE;
    for(iField=0; iField< poDefn->GetFieldCount(); iField++)
    {
        if (poSrcFile->IsFieldUnique(iField))
            bFoundUniqueField = TRUE;
    }

    if (bFoundUniqueField)
        poDstFile = new TABView;
    else
        poDstFile = new TABFile;

    /*---------------------------------------------------------------------
     * Try to open destination file
     *--------------------------------------------------------------------*/
    if (poDstFile->Open(pszDstFname, "wb") != 0)
    {
        printf("Failed to open %s\n", pszDstFname);
        return -1;
    }

    //  Set bounds
    if (poSrcFile->GetBounds(dXMin, dYMin, dXMax, dYMax) == 0)
        poDstFile->SetBounds(dXMin, dYMin, dXMax, dYMax);

    // Pass Proj. info directly
    // TABProjInfo sProjInfo;
    // if (poSrcFile->GetProjInfo(&sProjInfo) == 0)
    //     poDstFile->SetProjInfo(&sProjInfo);

    OGRSpatialReference *poSR;

    poSR = poSrcFile->GetSpatialRef();
    if( poSR != NULL )
    {
        poDstFile->SetSpatialRef( poSR );
    }

    /*---------------------------------------------------------------------
     * Pass compplete fields information
     *--------------------------------------------------------------------*/
    for(iField=0; iField< poDefn->GetFieldCount(); iField++)
    {
        OGRFieldDefn *poFieldDefn = poDefn->GetFieldDefn(iField);

        poDstFile->AddFieldNative(poFieldDefn->GetNameRef(),
                                  poSrcFile->GetNativeFieldType(iField),
                                  poFieldDefn->GetWidth(),
                                  poFieldDefn->GetPrecision(),
                                  poSrcFile->IsFieldIndexed(iField),
                                  poSrcFile->IsFieldUnique(iField));
    }

    /*---------------------------------------------------------------------
     * Copy objects until EOF is reached
     *--------------------------------------------------------------------*/
    nFeatureId = -1;
    while ( (nFeatureId = poSrcFile->GetNextFeatureId(nFeatureId)) != -1 )
    {
        poFeature = poSrcFile->GetFeatureRef(nFeatureId);
        if (poFeature)
        {
//            poFeature->DumpReadable(stdout);
//            poFeature->DumpMIF();
            poDstFile->SetFeature(poFeature);
        }
        else
            break;      // GetFeatureRef() failed: Abort the loop
    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    poDstFile->Close();

    poSrcFile->Close();
    delete poSrcFile;

    return 0;
}



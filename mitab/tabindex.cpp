/**********************************************************************
 * $Id: tabindex.cpp,v 1.3 2004-06-04 19:09:12 dmorissette Exp $
 *
 * Name:     tabindex.cpp
 * Project:  MapInfo TAB format Read/Write library
 * Language: C++
 * Purpose:  Create an index in an existing TAB file.
 * Author:   Daniel Morissette, morissette@dmsolutions.ca
 *
 **********************************************************************
 * Copyright (c) 1999-2001, Daniel Morissette
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
 * $Log: tabindex.cpp,v $
 * Revision 1.3  2004-06-04 19:09:12  dmorissette
 * Updated ref to lib's homepage in usage instructions
 *
 * Revision 1.2  2001/05/01 04:34:19  daniel
 * Fixed usage message.
 *
 * Revision 1.1  2001/05/01 03:40:25  daniel
 * Added tabindex program to create indexes in existing IND files.
 *
 **********************************************************************/


#include "mitab.h"
#include <ctype.h>

static int CreateIndex(const char *pszSrcFname, const char *pszField);


/**********************************************************************
 *                          main()
 *
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszSrcFname, *pszFieldName;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<3)
    {
        printf("\nTABINDEX - MITAB Version %s\n\n", MITAB_VERSION);
        printf("Usage: tabindex <tab_filename> <field_to_index>\n");
        printf("    Create index for specified field in an existing TAB dataset.\n\n");
        printf("For the latest version of this program and of the library, see: \n");
        printf("    http://mitab.maptools.org/\n\n");
        return 1;
    }
    else
    {
        pszSrcFname = argv[1];
        pszFieldName = argv[2];
    }
    

    return CreateIndex(pszSrcFname, pszFieldName);
}


/**********************************************************************
 *                          CreateIndex()
 *
 * Create index for specified field in an existing TAB dataset.
 **********************************************************************/
static int CreateIndex(const char *pszSrcFname, const char *pszField)
{
    IMapInfoFile *poSrcFile = NULL;
    int         nFeatureId, iField;
    TABFeature *poFeature;
    TABINDFile *poINDFile;

    /*---------------------------------------------------------------------
     * Try to open source file
     *--------------------------------------------------------------------*/
    if ((poSrcFile = IMapInfoFile::SmartOpen(pszSrcFname)) == NULL)
    {
        printf("Failed to open %s\n", pszSrcFname);
        return -1;
    }

    if (poSrcFile->GetFileClass() != TABFC_TABFile)
    {
        printf("Indexes cannot be added to this type of TAB datasets\n");
        poSrcFile->Close();
        delete poSrcFile;
        return -1;
    }

    /*---------------------------------------------------------------------
     * Make sure field exists and is not already indexed
     *--------------------------------------------------------------------*/
    OGRFeatureDefn *poDefn = poSrcFile->GetLayerDefn();
    if ( poDefn == NULL ||
         (iField = poDefn->GetFieldIndex(pszField)) == -1 ||
         poSrcFile->IsFieldIndexed(iField))
    {
        printf("Cannot create index: field '%s' not found or is already indexed.\n", 
               pszField);
        poSrcFile->Close();
        delete poSrcFile;
        return -1;
    }

    /*---------------------------------------------------------------------
     * Things seem OK... open IND file for update
     * (Note that TABINDFile automagically adjusts file extension)
     *--------------------------------------------------------------------*/
    poINDFile = new TABINDFile; 
    if ( poINDFile->Open(pszSrcFname, "r+", TRUE) != 0 &&
         poINDFile->Open(pszSrcFname, "w", TRUE) != 0)
    {
        printf("Unable to create IND file for %s.\n", pszSrcFname);
        delete poINDFile;
        poSrcFile->Close();
        delete poSrcFile;
        return -1;
    }

    int nNewIndexNo = -1;
    OGRFieldDefn *poFieldDefn = poDefn->GetFieldDefn(iField);
    TABFieldType eFieldType = poSrcFile->GetNativeFieldType(iField);
    if (poFieldDefn == NULL ||
        (nNewIndexNo = poINDFile->CreateIndex(eFieldType,
                                              poFieldDefn->GetWidth()) ) < 1)
    {
        // Failed... an error has already been reported.
        delete poINDFile;
        poSrcFile->Close();
        delete poSrcFile;
        return -1;
    }

    printf("Index number %d will be created for field %s...\n\n"
           "This program does not update the TAB header file (yet!) so you \n"
           "should edit %s and add 'Index %d' at the end of the definition \n"
           "of field %s.\n\n",
           nNewIndexNo, pszField, pszSrcFname, nNewIndexNo, pszField);

    /*---------------------------------------------------------------------
     * Add index entries until we reach EOF
     *--------------------------------------------------------------------*/
    nFeatureId = -1;
    while ( (nFeatureId = poSrcFile->GetNextFeatureId(nFeatureId)) != -1 )
    {
        poFeature = poSrcFile->GetFeatureRef(nFeatureId);
        if (poFeature)
        {
            GByte *pKey = NULL;
            switch(eFieldType)
            {
              case TABFChar:
                pKey = poINDFile->BuildKey(nNewIndexNo, 
                                        poFeature->GetFieldAsString(iField));
                break;
              case TABFInteger:
              case TABFSmallInt:
                pKey = poINDFile->BuildKey(nNewIndexNo, 
                                        poFeature->GetFieldAsInteger(iField));
                break;
              case TABFDecimal:
              case TABFFloat:
              case TABFLogical:
                pKey = poINDFile->BuildKey(nNewIndexNo, 
                                        poFeature->GetFieldAsDouble(iField));
                break;
              default:
              case TABFDate:
                CPLAssert(FALSE); // Unsupported for now.
            }

            if (poINDFile->AddEntry(nNewIndexNo, pKey, nFeatureId) != 0)
                return -1;
        }
        else
            break;      // GetFeatureRef() failed: Abort the loop
    }

    /*---------------------------------------------------------------------
     * Cleanup and exit.
     *--------------------------------------------------------------------*/
    poINDFile->Close();
    delete poINDFile;

    poSrcFile->Close();
    delete poSrcFile;

    MITABFreeCoordSysTable();

    return 0;
}



/**********************************************************************
 * $Id: mitab_idfile.cpp,v 1.2 1999-09-16 02:39:16 daniel Exp $
 *
 * Name:     mitab_idfile.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABIDFile class used to handle
 *           reading/writing of the .ID file attached to a .MAP file
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
 * $Log: mitab_idfile.cpp,v $
 * Revision 1.2  1999-09-16 02:39:16  daniel
 * Completed read support for most feature types
 *
 * Revision 1.1  1999/07/12 04:18:24  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"
#include "mitab_utils.h"

/*=====================================================================
 *                      class TABIDFile
 *====================================================================*/


/**********************************************************************
 *                   TABIDFile::TABIDFile()
 *
 * Constructor.
 **********************************************************************/
TABIDFile::TABIDFile()
{
    m_fp = NULL;
    m_pszFname = NULL;
    m_poIDBlock = NULL;
    m_nMaxId = -1;
}

/**********************************************************************
 *                   TABIDFile::~TABIDFile()
 *
 * Destructor.
 **********************************************************************/
TABIDFile::~TABIDFile()
{
    Close();
}

/**********************************************************************
 *                   TABIDFile::Open()
 *
 * Open a .ID file, and initialize the structures to be ready to read
 * objects from it.
 *
 * If the filename that is passed in contains a .MAP extension then
 * the extension will be changed to .ID before trying to open the file.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABIDFile::Open(const char *pszFname, const char *pszAccess)
{
    int         nLen;

    if (m_fp)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: object already contains an open file");
        return -1;
    }

    /*-----------------------------------------------------------------
     * Validate access mode
     *----------------------------------------------------------------*/
    if (EQUALN(pszAccess, "r", 1))
    {
        m_eAccessMode = TABRead;
    }
    else if (EQUALN(pszAccess, "w", 1))
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Open() failed: write access not implemented yet!");
        return -1;

        m_eAccessMode = TABWrite;
    }
    else
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: access mode \"%s\" not supported", pszAccess);
        return -1;
    }

    /*-----------------------------------------------------------------
     * Change .MAP extension to .ID if necessary
     *----------------------------------------------------------------*/
    m_pszFname = CPLStrdup(pszFname);

    nLen = strlen(m_pszFname);
    if (nLen > 4 && strcmp(m_pszFname+nLen-4, ".MAP")==0)
        strcpy(m_pszFname+nLen-4, ".ID");
    else if (nLen > 4 && strcmp(m_pszFname+nLen-4, ".map")==0)
        strcpy(m_pszFname+nLen-4, ".id");

    /*-----------------------------------------------------------------
     * Change .MAP extension to .ID if necessary
     *----------------------------------------------------------------*/
#ifndef _WIN32
    TABAdjustFilenameExtension(m_pszFname);
#endif

    /*-----------------------------------------------------------------
     * Open file for reading
     *----------------------------------------------------------------*/
    m_fp = VSIFOpen(m_pszFname, pszAccess);

    if (m_fp == NULL)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed for %s", m_pszFname);
        CPLFree(m_pszFname);
        m_pszFname = NULL;
        return -1;
    }

    /*---------------------------------------------------------------------
     * Establish the number of object IDs from the size of the file
     *--------------------------------------------------------------------*/
    VSIStatBuf  sStatBuf;
    if ( VSIStat(m_pszFname, &sStatBuf) == -1 )
    {
        CPLError(CE_Failure, CPLE_FileIO, 
                 "stat() failed for %s\n", m_pszFname);
        Close();
        return -1;
    }

    m_nMaxId = sStatBuf.st_size/4;
    m_nBlockSize = MIN(1024, m_nMaxId*4);

    /*-----------------------------------------------------------------
     * Read the first block from the file
     *----------------------------------------------------------------*/
    m_poIDBlock = new TABRawBinBlock;
    if (m_poIDBlock->ReadFromFile(m_fp, 0, m_nBlockSize) != 0)
    {
        // CPLError() has already been called.
        Close();
        return -1;
    }

    return 0;
}

/**********************************************************************
 *                   TABIDFile::Close()
 *
 * Close current file, and release all memory used.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABIDFile::Close()
{
    if (m_fp == NULL)
        return 0;

    //__TODO__ Commit the latest changes to the file...
    
    // Delete all structures 
    delete m_poIDBlock;
    m_poIDBlock = NULL;

    // Close file
    VSIFClose(m_fp);
    m_fp = NULL;

    CPLFree(m_pszFname);
    m_pszFname = NULL;

    return 0;
}


/**********************************************************************
 *                   TABIDFile::GetObjPtr()
 *
 * Return the offset in the .MAP file where the map object with the
 * specified id is located.
 *
 * Note that object ids are positive and start at 1.
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
GInt32 TABIDFile::GetObjPtr(GInt32 nObjId)
{
    if (m_poIDBlock == NULL)
        return -1;

    if (nObjId < 1 || nObjId > m_nMaxId)
    {
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "GetObjPtr(): Invalid object ID %d (valid range is [1..%d])",
                 nObjId, m_nMaxId);
        return -1;
    }

    if (m_poIDBlock->GotoByteInFile( (nObjId-1)*4 ) != 0)
        return -1;

    return m_poIDBlock->ReadInt32();
}


/**********************************************************************
 *                   TABIDFile::GetMaxObjId()
 *
 * Return the value of the biggest valid object id.
 *
 * Note that object ids are positive and start at 1.
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
GInt32 TABIDFile::GetMaxObjId()
{
    return m_nMaxId;
}


/**********************************************************************
 *                   TABIDFile::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABIDFile::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABIDFile::Dump() -----\n");

    if (m_fp == NULL)
    {
        fprintf(fpOut, "File is not opened.\n");
    }
    else
    {
        fprintf(fpOut, "File is opened: %s\n", m_pszFname);
        fprintf(fpOut, "Current index block follows ...\n\n");
        m_poIDBlock->Dump(fpOut);
        fprintf(fpOut, "... end of index block.\n\n");

    }

    fflush(fpOut);
}

#endif // DEBUG






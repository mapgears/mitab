/**********************************************************************
 * $Id: mitab_indfile.cpp,v 1.1 1999-11-20 15:49:07 daniel Exp $
 *
 * Name:     mitab_indfile.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABINDFile class used to handle
 *           read-only access to .IND file (table field indexes) 
 *           attached to a .DAT file
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
 * $Log: mitab_indfile.cpp,v $
 * Revision 1.1  1999-11-20 15:49:07  daniel
 * Initial version
 *
 **********************************************************************/

#include "mitab.h"
#include "mitab_utils.h"

/*=====================================================================
 *                      class TABINDFile
 *====================================================================*/

#define IND_MAGIC_COOKIE  0xe8f8

/**********************************************************************
 *                   TABINDFile::TABINDFile()
 *
 * Constructor.
 **********************************************************************/
TABINDFile::TABINDFile()
{
    m_fp = NULL;
    m_pszFname = NULL;
    m_eAccessMode = TABRead;
    m_numIndexes = 0;
    m_papoIndexRootNodes = NULL;

}

/**********************************************************************
 *                   TABINDFile::~TABINDFile()
 *
 * Destructor.
 **********************************************************************/
TABINDFile::~TABINDFile()
{
    Close();
}

/**********************************************************************
 *                   TABINDFile::Open()
 *
 * Open a .IND file, read the header and the root nodes for all the
 * field indexes, and be ready to search the indexes.
 *
 * If the filename that is passed in contains a .DAT extension then
 * the extension will be changed to .IND before trying to open the file.
 *
 * Note that we pass a pszAccess flag, but only read access is supported
 * for now (and there are no plans to support write.)
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABINDFile::Open(const char *pszFname, const char *pszAccess)
{
    int         nLen;

    if (m_fp)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: object already contains an open file");
        return -1;
    }

    /*-----------------------------------------------------------------
     * Validate access mode and make sure we use binary access.
     * Note that we support only read access.
     *----------------------------------------------------------------*/
    if (EQUALN(pszAccess, "r", 1))
    {
        m_eAccessMode = TABRead;
        pszAccess = "rb";
    }
    else
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed: access mode \"%s\" not supported", pszAccess);
        return -1;
    }

    /*-----------------------------------------------------------------
     * Change .DAT extension to .IND if necessary
     *----------------------------------------------------------------*/
    m_pszFname = CPLStrdup(pszFname);

    nLen = strlen(m_pszFname);
    if (nLen > 4 && strcmp(m_pszFname+nLen-4, ".DAT")==0)
        strcpy(m_pszFname+nLen-4, ".IND");
    else if (nLen > 4 && strcmp(m_pszFname+nLen-4, ".DAT")==0)
        strcpy(m_pszFname+nLen-4, ".ind");

#ifndef _WIN32
    TABAdjustFilenameExtension(m_pszFname);
#endif

    /*-----------------------------------------------------------------
     * Open file
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

    /*-----------------------------------------------------------------
     * Read the header block
     *----------------------------------------------------------------*/
    TABRawBinBlock *poHeaderBlock;
    poHeaderBlock = new TABRawBinBlock(m_eAccessMode, TRUE);
    if (poHeaderBlock->ReadFromFile(m_fp, 0, 512) != 0)
    {
        // CPLError() has already been called.
        Close();
        return -1;
    }

    poHeaderBlock->GotoByteInBlock(0);
    int nMagicCookie = poHeaderBlock->ReadInt16();
    if (nMagicCookie != IND_MAGIC_COOKIE)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "%s: Invalid Magic Cookie: got 0x%04.4x expected 0x%04.4x",
                 m_pszFname, nMagicCookie, IND_MAGIC_COOKIE);
        delete poHeaderBlock;
        Close();
        return -1;
    }

    poHeaderBlock->GotoByteInBlock(12);
    m_numIndexes = poHeaderBlock->ReadInt16();
    if (m_numIndexes < 1 || m_numIndexes > 29)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Invalid number of indexes (%d) in file %s",
                 m_numIndexes, m_pszFname);
        delete poHeaderBlock;
        Close();
        return -1;
    }

    /*-----------------------------------------------------------------
     * Alloc and init the array of index root nodes.
     *----------------------------------------------------------------*/
    m_papoIndexRootNodes = (TABINDNode**)CPLCalloc(m_numIndexes,
                                                   sizeof(TABINDNode*));

    for(int iIndex=0; iIndex<m_numIndexes; iIndex++)
    {
        /*-------------------------------------------------------------
         * Read next index definition
         *------------------------------------------------------------*/
        GInt32 nRootNodePtr = poHeaderBlock->ReadInt32();
        poHeaderBlock->ReadInt16();   // skip... max. num of entries per node
        int nTreeDepth = poHeaderBlock->ReadByte();
        int nKeyLength = poHeaderBlock->ReadByte();
        poHeaderBlock->GotoByteRel(8); // skip next 8 bytes;

        /*-------------------------------------------------------------
         * And init root node for this index.
         *------------------------------------------------------------*/
        m_papoIndexRootNodes[iIndex] = new TABINDNode;
        if (m_papoIndexRootNodes[iIndex]->InitNode(m_fp, nRootNodePtr,
                                                   nKeyLength, nTreeDepth)!= 0)
        {
            // CPLError has already been called
            delete poHeaderBlock;
            Close();
            return -1;
        }
    }

    /*-----------------------------------------------------------------
     * OK, we won't need the header block any more... free it.
     *----------------------------------------------------------------*/
    delete poHeaderBlock;

    return 0;
}

/**********************************************************************
 *                   TABINDFile::Close()
 *
 * Close current file, and release all memory used.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABINDFile::Close()
{
    if (m_fp == NULL)
        return 0;

    // Delete array of indexes
    for (int iIndex=0; m_papoIndexRootNodes && iIndex<m_numIndexes; iIndex++)
    {
        if (m_papoIndexRootNodes[iIndex])
            delete m_papoIndexRootNodes[iIndex];
    }
    CPLFree(m_papoIndexRootNodes);
    m_papoIndexRootNodes = NULL;
    m_numIndexes = 0;

    // Close file
    VSIFClose(m_fp);
    m_fp = NULL;

    CPLFree(m_pszFname);
    m_pszFname = NULL;

    return 0;
}


/**********************************************************************
 *                   TABINDFile::Search()
 *
 * Search one of the indexes for a key value.  
 *
 * Note that index numbers are positive values starting at 1.
 *
 * Return value:
 *  - the key's corresponding record number in the .DAT file (greater than 0)
 *  - 0 if the key was not found
 *  - or -1 if an error happened
 **********************************************************************/
GInt32 TABINDFile::Search(int nIndexNumber, GByte *pKeyValue)
{
    if (m_fp == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "TABINDFile::Search(): File has not been opened yet!");
        return -1;
    }

    if (nIndexNumber < 1 || nIndexNumber > m_numIndexes ||
        m_papoIndexRootNodes == NULL || 
        m_papoIndexRootNodes[nIndexNumber-1] == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "No field index number %d in %s: Valid range is [1..%d].",
                 nIndexNumber, m_pszFname, m_numIndexes);
        return -1;
    }

    return m_papoIndexRootNodes[nIndexNumber-1]->Search(pKeyValue);
}



/**********************************************************************
 *                   TABINDFile::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABINDFile::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABINDFile::Dump() -----\n");

    if (m_fp == NULL)
    {
        fprintf(fpOut, "File is not opened.\n");
    }
    else
    {
        fprintf(fpOut, "File is opened: %s\n", m_pszFname);
        fprintf(fpOut, "   m_numIndexes   = %d\n", m_numIndexes);
        for(int i=0; 
            i<m_numIndexes && m_papoIndexRootNodes && m_papoIndexRootNodes[i];
            i++)
        {
            fprintf(fpOut, "  ----- Index # %d -----\n", i+1);
            m_papoIndexRootNodes[i]->Dump(fpOut);
        }

    }

    fflush(fpOut);
}

#endif // DEBUG





/*=====================================================================
 *                      class TABINDNode
 *====================================================================*/

/**********************************************************************
 *                   TABINDNode::TABINDNode()
 *
 * Constructor.
 **********************************************************************/
TABINDNode::TABINDNode()
{
    m_fp = NULL;
    m_poCurChildNode = NULL;
    m_nCurChildNodeIndex = 0;
    m_nSubTreeDepth = 0;
    m_nKeyLength = 0;
    m_poDataBlock = NULL;
    m_numEntriesInNode = 0;
    m_nPrevNodePtr = 0;
    m_nNextNodePtr = 0;

    m_eAccessMode = TABRead;
}

/**********************************************************************
 *                   TABINDNode::~TABINDNode()
 *
 * Destructor.
 **********************************************************************/
TABINDNode::~TABINDNode()
{
    if (m_poCurChildNode)
        delete m_poCurChildNode;

    if (m_poDataBlock)
        delete m_poDataBlock;
}

/**********************************************************************
 *                   TABINDNode::InitNode()
 *
 * Init a node... this function can be used either to initialize a new
 * node, or to make it point to a new data block in the file.
 *
 * This call will read the data from the file at the specified location
 * if necessary, and leave the object ready to be searched.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABINDNode::InitNode(FILE *fp, int nBlockPtr, 
                         int nKeyLength, int nSubTreeDepth)
{
    /*-----------------------------------------------------------------
     * If the block already points to the right block, then don't do 
     * anything here.
     *----------------------------------------------------------------*/
    if (m_fp == fp && m_nCurDataBlockPtr == nBlockPtr)
        return 0;

    // Keep track of some info
    m_fp = fp;
    m_nKeyLength = nKeyLength;
    m_nSubTreeDepth = nSubTreeDepth;
    m_nCurDataBlockPtr = nBlockPtr;

    // And invalidate the reference to the current child if there was one
    m_nCurChildNodeIndex = -1;

    /*-----------------------------------------------------------------
     * Read the data block from the file
     *----------------------------------------------------------------*/
    if (m_poDataBlock == NULL)
        m_poDataBlock = new TABRawBinBlock(m_eAccessMode, TRUE);

    if (m_poDataBlock->ReadFromFile(m_fp, m_nCurDataBlockPtr, 512) != 0)
    {
        // CPLError() has already been called.
        return -1;
    }

    m_poDataBlock->GotoByteInBlock(0);
    m_numEntriesInNode = m_poDataBlock->ReadInt32();
    m_nPrevNodePtr = m_poDataBlock->ReadInt32();
    m_nNextNodePtr = m_poDataBlock->ReadInt32();

    // m_poDataBlock is now positioned at the beginning of the key entries

    return 0;
}


/**********************************************************************
 *                   TABINDNode::GotoNodePtr()
 *
 * Move to the specified node ptr, and read the new node data from the file.
 *
 * This is just a cover funtion on top of InitNode()
 **********************************************************************/
int TABINDNode::GotoNodePtr(GInt32 nNewNodePtr)
{
    return InitNode(m_fp, nNewNodePtr, m_nKeyLength, m_nSubTreeDepth);
}

/**********************************************************************
 *                   TABINDNode::Search()
 *
 * Search the node and its children for a key value.  
 *
 * Return value:
 *  - the key's corresponding record number in the .DAT file (greater than 0)
 *  - 0 if the key was not found
 *  - or -1 if an error happened
 **********************************************************************/
GInt32 TABINDNode::Search(GByte *pKeyValue)
{
    if (m_poDataBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "TABINDNode::Search(): Node has not been initialized yet!");
        return -1;
    }

    // __TODO__

    return 0;  // Not found
}



/**********************************************************************
 *                   TABINDNode::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABINDNode::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABINDNode::Dump() -----\n");

    if (m_fp == NULL)
    {
        fprintf(fpOut, "Node is not initialized.\n");
    }
    else
    {
        fprintf(fpOut, "Node is initialized:\n");
        fprintf(fpOut, "   m_numEntriesInNode   = %d\n", m_numEntriesInNode);

    }

    fflush(fpOut);
}

#endif // DEBUG






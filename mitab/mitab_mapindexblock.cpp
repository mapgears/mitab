/**********************************************************************
 * $Id: mitab_mapindexblock.cpp,v 1.1 1999-07-12 04:18:25 daniel Exp $
 *
 * Name:     mitab_mapindexblock.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABMAPIndexBlock class used to handle
 *           reading/writing of the .MAP files' index blocks
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
 * $Log: mitab_mapindexblock.cpp,v $
 * Revision 1.1  1999-07-12 04:18:25  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABMAPIndexBlock
 *====================================================================*/


/**********************************************************************
 *                   TABMAPIndexBlock::TABMAPIndexBlock()
 *
 * Constructor.
 **********************************************************************/
TABMAPIndexBlock::TABMAPIndexBlock():
    TABRawBinBlock(TRUE)
{

}

/**********************************************************************
 *                   TABMAPIndexBlock::~TABMAPIndexBlock()
 *
 * Destructor.
 **********************************************************************/
TABMAPIndexBlock::~TABMAPIndexBlock()
{

}


/**********************************************************************
 *                   TABMAPIndexBlock::InitBlockData()
 *
 * Perform some initialization on the block after its binary data has
 * been set or changed (or loaded from a file).
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPIndexBlock::InitBlockData(GByte *pabyBuf, int nSize, 
                                         GBool bMakeCopy /* = TRUE */,
                                         FILE *fpSrc /* = NULL */, 
                                         int nOffset /* = 0 */)
{
    int nStatus;

    /*-----------------------------------------------------------------
     * First of all, we must call the base class' InitBlockData()
     *----------------------------------------------------------------*/
    nStatus = TABRawBinBlock::InitBlockData(pabyBuf, nSize, bMakeCopy,
                                            fpSrc, nOffset);
    if (nStatus != 0)   
        return nStatus;

    /*-----------------------------------------------------------------
     * Validate block type
     *----------------------------------------------------------------*/
    if (m_nBlockType != TABMAP_INDEX_BLOCK)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "InitBlockData(): Invalid Block Type: got %d expected %d",
                 m_nBlockType, TABMAP_INDEX_BLOCK);
        CPLFree(m_pabyBuf);
        m_pabyBuf = NULL;
        return -1;
    }

    /*-----------------------------------------------------------------
     * Init member variables
     *----------------------------------------------------------------*/
    GotoByteInBlock(0x002);
    m_numEntries = ReadInt16();

    return 0;
}


/**********************************************************************
 *                   TABMAPIndexBlock::ReadNextEntry()
 *
 * Read the next index entry from the block and fill the sEntry
 * structure. 
 *
 * Returns 0 if succesful or -1 if we reached the end of the block.
 **********************************************************************/
int     TABMAPIndexBlock::ReadNextEntry(TABMAPIndexEntry &sEntry)
{
    if (m_nCurPos < 4)
        GotoByteInBlock( 0x004 );

    if (m_nCurPos > 4+(16*m_numEntries) )
    {
        // End of BLock
        return -1;
    }

    sEntry.XMin = ReadInt32();
    sEntry.YMin = ReadInt32();
    sEntry.XMax = ReadInt32();
    sEntry.YMax = ReadInt32();
    sEntry.nBlockOffset = ReadInt32();

    return 0;
}

/**********************************************************************
 *                   TABMAPIndexBlock::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABMAPIndexBlock::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABMAPIndexBlock::Dump() -----\n");
    if (m_pabyBuf == NULL)
    {
        fprintf(fpOut, "Block has not been initialized yet.");
    }
    else
    {
        fprintf(fpOut,"Index Block (type %d) at offset %d.\n", 
                                                m_nBlockType, m_nFileOffset);
        fprintf(fpOut,"  m_numEntries          = %d\n", m_numEntries);
    }

    fflush(fpOut);
}

#endif // DEBUG

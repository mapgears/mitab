/**********************************************************************
 * $Id: mitab_mapobjectblock.cpp,v 1.1 1999-07-12 04:18:25 daniel Exp $
 *
 * Name:     mitab_mapobjectblock.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABMAPObjectBlock class used to handle
 *           reading/writing of the .MAP files' object data blocks
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
 * $Log: mitab_mapobjectblock.cpp,v $
 * Revision 1.1  1999-07-12 04:18:25  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABMAPObjectBlock
 *====================================================================*/


/**********************************************************************
 *                   TABMAPObjectBlock::TABMAPObjectBlock()
 *
 * Constructor.
 **********************************************************************/
TABMAPObjectBlock::TABMAPObjectBlock():
    TABRawBinBlock(TRUE)
{

}

/**********************************************************************
 *                   TABMAPObjectBlock::~TABMAPObjectBlock()
 *
 * Destructor.
 **********************************************************************/
TABMAPObjectBlock::~TABMAPObjectBlock()
{

}


/**********************************************************************
 *                   TABMAPObjectBlock::InitBlockData()
 *
 * Perform some initialization on the block after its binary data has
 * been set or changed (or loaded from a file).
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPObjectBlock::InitBlockData(GByte *pabyBuf, int nSize, 
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
    if (m_nBlockType != TABMAP_OBJECT_BLOCK)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "InitBlockData(): Invalid Block Type: got %d expected %d",
                 m_nBlockType, TABMAP_OBJECT_BLOCK);
        CPLFree(m_pabyBuf);
        m_pabyBuf = NULL;
        return -1;
    }

    /*-----------------------------------------------------------------
     * Init member variables
     *----------------------------------------------------------------*/
    GotoByteInBlock(0x002);
    m_numDataBytes = ReadInt16();       /* Excluding 4 bytes header */

    m_nCenterX = ReadInt32();
    m_nCenterY = ReadInt32();

    m_nFirstCoordBlock = ReadInt32();
    m_nLastCoordBlock = ReadInt32();

    return 0;
}


/**********************************************************************
 *                   TABMAPObjectBlock::ReadCoord()
 *
 * Read the next pair of integer coordinates value from the block, and
 * apply the translation relative to to the center of the data block
 * if bCompressed=TRUE.
 *
 * This means that the returned coordinates are always absolute integer
 * coordinates, even when the source coords are in compressed form.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPObjectBlock::ReadIntCoord(GBool bCompressed, 
                                        GInt32 &nX, GInt32 &nY)
{
    if (bCompressed)
    {   
        nX = m_nCenterX + ReadInt16();
        nY = m_nCenterY + ReadInt16();
    }
    else
    {
        nX = ReadInt32();
        nY = ReadInt32();
    }

    if (CPLGetLastErrorNo() != 0)
        return -1;

    return 0;
}

/**********************************************************************
 *                   TABMAPObjectBlock::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABMAPObjectBlock::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABMAPObjectBlock::Dump() -----\n");
    if (m_pabyBuf == NULL)
    {
        fprintf(fpOut, "Block has not been initialized yet.");
    }
    else
    {
        fprintf(fpOut,"Object Data Block (type %d) at offset %d.\n", 
                                                m_nBlockType, m_nFileOffset);
        fprintf(fpOut,"  m_numDataBytes        = %d\n", m_numDataBytes);
        fprintf(fpOut,"  m_nCenterX            = %d\n", m_nCenterX);
        fprintf(fpOut,"  m_nCenterY            = %d\n", m_nCenterY);
        fprintf(fpOut,"  m_nFirstCoordBlock    = %d\n", m_nFirstCoordBlock);
        fprintf(fpOut,"  m_nLastCoordBlock     = %d\n", m_nLastCoordBlock);
    }

    fflush(fpOut);
}

#endif // DEBUG

/**********************************************************************
 * $Id: mitab_maptoolblock.cpp,v 1.1 1999-09-16 02:39:17 daniel Exp $
 *
 * Name:     mitab_maptoollock.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABMAPToolBlock class used to handle
 *           reading/writing of the .MAP files' drawing tool blocks
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
 * $Log: mitab_maptoolblock.cpp,v $
 * Revision 1.1  1999-09-16 02:39:17  daniel
 * Completed read support for most feature types
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABMAPToolBlock
 *====================================================================*/

#define MAP_TOOL_HEADER_SIZE   8

/**********************************************************************
 *                   TABMAPToolBlock::TABMAPToolBlock()
 *
 * Constructor.
 **********************************************************************/
TABMAPToolBlock::TABMAPToolBlock():
    TABRawBinBlock(TRUE)
{
    m_nNextToolBlock = m_numDataBytes = 0;
 
    m_papsPen = NULL;
    m_papsBrush = NULL;
    m_papsFont = NULL;
    m_papsSymbol = NULL;
    m_numPen = 0;
    m_numBrushes = 0;
    m_numFonts = 0;
    m_numSymbols = 0;
    m_numAllocatedPen = 0;
    m_numAllocatedBrushes = 0;
    m_numAllocatedFonts = 0;
    m_numAllocatedSymbols = 0;

    m_bToolDefsInitialized = FALSE;

}

/**********************************************************************
 *                   TABMAPToolBlock::~TABMAPToolBlock()
 *
 * Destructor.
 **********************************************************************/
TABMAPToolBlock::~TABMAPToolBlock()
{
    int i;

    for(i=0; m_papsPen && i < m_numPen; i++)
        CPLFree(m_papsPen[i]);
    CPLFree(m_papsPen);

    for(i=0; m_papsBrush && i < m_numBrushes; i++)
        CPLFree(m_papsBrush[i]);
    CPLFree(m_papsBrush);

    for(i=0; m_papsFont && i < m_numFonts; i++)
        CPLFree(m_papsFont[i]);
    CPLFree(m_papsFont);

    for(i=0; m_papsSymbol && i < m_numSymbols; i++)
        CPLFree(m_papsSymbol[i]);
    CPLFree(m_papsSymbol);

   
}


/**********************************************************************
 *                   TABMAPToolBlock::InitBlockData()
 *
 * Perform some initialization on the block after its binary data has
 * been set or changed (or loaded from a file).
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPToolBlock::InitBlockData(GByte *pabyBuf, int nSize, 
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
    if (m_nBlockType != TABMAP_TOOL_BLOCK)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "InitBlockData(): Invalid Block Type: got %d expected %d",
                 m_nBlockType, TABMAP_TOOL_BLOCK);
        CPLFree(m_pabyBuf);
        m_pabyBuf = NULL;
        return -1;
    }

    /*-----------------------------------------------------------------
     * Init member variables
     *----------------------------------------------------------------*/
    GotoByteInBlock(0x002);
    m_numDataBytes = ReadInt16();       /* Excluding 8 bytes header */

    m_nNextToolBlock = ReadInt32();

    /*-----------------------------------------------------------------
     * The read ptr is now located at the beginning of the data part.
     *----------------------------------------------------------------*/
    GotoByteInBlock(MAP_TOOL_HEADER_SIZE);

    return 0;
}

/**********************************************************************
 *                   TABMAPToolBlock::ReadBytes()
 *
 * Cover function for TABRawBinBlock::ReadBytes() that will automagically
 * load the next coordinate block in the chain before reading the 
 * requested bytes if we are at the end of the current block and if
 * m_nNextToolBlock is a valid block.
 *
 * Then the control is passed to TABRawBinBlock::ReadBytes() to finish the
 * work:
 * Copy the number of bytes from the data block's internal buffer to
 * the user's buffer pointed by pabyDstBuf.
 *
 * Passing pabyDstBuf = NULL will only move the read pointer by the
 * specified number of bytes as if the copy had happened... but it 
 * won't crash.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPToolBlock::ReadBytes(int numBytes, GByte *pabyDstBuf)
{
    int nStatus;

    if (m_pabyBuf && 
        m_nCurPos >= (m_numDataBytes+MAP_TOOL_HEADER_SIZE) && 
        m_nNextToolBlock > 0)
    {
        if ( (nStatus=GotoByteInFile(m_nNextToolBlock)) != 0)
        {
            // Failed.... an error has already been reported.
            return nStatus;
        }

        GotoByteInBlock(8);      // Move pointer past header
    }

    return TABRawBinBlock::ReadBytes(numBytes, pabyDstBuf);
}


/**********************************************************************
 *                   TABMAPToolBlock::ReadAllToolDefs()
 *
 * Read all tool definition blocks until we reach the end of the chain.
 * This function will be called automatically the first time a drawing
 * tool definition is requested.
 *
 * This has to be done only once for the first initialization... after that
 * we keep all the tool definitions in memory, so calls to this function
 * will simply have no effect.
 **********************************************************************/
int     TABMAPToolBlock::ReadAllToolDefs()
{
    int nStatus = 0;
    int nDefType;

    if (m_bToolDefsInitialized)
        return 0;
    
    /*-----------------------------------------------------------------
     * Loop until we reach the end of the chain of blocks... we assume
     * that the first block of data is already pre-loaded. 
     *----------------------------------------------------------------*/
    while(nStatus == 0 && m_pabyBuf && 
          (m_nCurPos < (m_numDataBytes+MAP_TOOL_HEADER_SIZE) || 
           m_nNextToolBlock > 0 ) )
    {
        nDefType = ReadByte();
        switch(nDefType)
        {
          case 1:       // PEN
            if (m_numPen >= m_numAllocatedPen)
            {
                // Realloc array by blocks of 20 items
                m_numAllocatedPen += 20;
                m_papsPen = (TABPenDef**)CPLRealloc(m_papsPen, 
                                        m_numAllocatedPen*sizeof(TABPenDef*));
            }
            m_papsPen[m_numPen] = (TABPenDef*)CPLCalloc(1, sizeof(TABPenDef));

            m_papsPen[m_numPen]->nRefCount  = ReadInt32();
            m_papsPen[m_numPen]->nLineWidth = ReadByte();
            m_papsPen[m_numPen]->nLinePattern = ReadByte();
            m_papsPen[m_numPen]->nLineStyle = ReadByte();
            m_papsPen[m_numPen]->rgbColor   = ReadByte()*256*256+
                                              ReadByte()*256 + ReadByte();

            m_numPen++;

            break;
          case 2:       // BRUSH
            if (m_numBrushes >= m_numAllocatedBrushes)
            {
                // Realloc array by blocks of 20 items
                m_numAllocatedBrushes += 20;
                m_papsBrush = (TABBrushDef**)CPLRealloc(m_papsBrush, 
                                 m_numAllocatedBrushes*sizeof(TABBrushDef*));
            }
            m_papsBrush[m_numBrushes] = 
                               (TABBrushDef*)CPLCalloc(1,sizeof(TABBrushDef));

            m_papsBrush[m_numBrushes]->nRefCount    = ReadInt32();
            m_papsBrush[m_numBrushes]->nFillPattern = ReadByte();
            m_papsBrush[m_numBrushes]->bTransparentFill = ReadByte();
            m_papsBrush[m_numBrushes]->rgbFGColor   = ReadByte()*256*256+
                                                      ReadByte()*256 + 
                                                      ReadByte();
            m_papsBrush[m_numBrushes]->rgbBGColor   = ReadByte()*256*256+
                                                      ReadByte()*256 + 
                                                      ReadByte();

            m_numBrushes++;

            break;
          case 3:       // FONT NAME
            if (m_numFonts >= m_numAllocatedFonts)
            {
                // Realloc array by blocks of 20 items
                m_numAllocatedFonts += 20;
                m_papsFont = (TABFontDef**)CPLRealloc(m_papsFont, 
                                 m_numAllocatedFonts*sizeof(TABFontDef*));
            }
            m_papsFont[m_numFonts] = 
                               (TABFontDef*)CPLCalloc(1,sizeof(TABFontDef));

            m_papsFont[m_numFonts]->nRefCount    = ReadInt32();
            ReadBytes(32, (GByte*)m_papsFont[m_numFonts]->szFontName);
            m_papsFont[m_numFonts]->szFontName[32] = '\0';

            m_numFonts++;

            break;
          case 4:       // SYMBOL
            if (m_numSymbols >= m_numAllocatedSymbols)
            {
                // Realloc array by blocks of 20 items
                m_numAllocatedSymbols += 20;
                m_papsSymbol = (TABSymbolDef**)CPLRealloc(m_papsSymbol, 
                                 m_numAllocatedSymbols*sizeof(TABSymbolDef*));
            }
            m_papsSymbol[m_numSymbols] = 
                               (TABSymbolDef*)CPLCalloc(1,sizeof(TABSymbolDef));

            m_papsSymbol[m_numSymbols]->nRefCount    = ReadInt32();
            m_papsSymbol[m_numSymbols]->nSymbolNo    = ReadInt16();
            m_papsSymbol[m_numSymbols]->nPointSize   = ReadInt16();
            m_papsSymbol[m_numSymbols]->_nUnknownValue_ = ReadByte();
            m_papsSymbol[m_numSymbols]->rgbColor   = ReadByte()*256*256+
                                                     ReadByte()*256 + 
                                                     ReadByte();

            m_numSymbols++;

            break;
          default:
            /* Unsupported Tool type!!! */
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Unsupported drawing tool type: `%d'", nDefType);
            nStatus = -1;
        }

        if (CPLGetLastErrorNo() != 0)
        {
            // An error happened reading this tool definition... stop now.
            nStatus = -1;
        }
    }

    m_bToolDefsInitialized = TRUE;

    return nStatus;
}



/**********************************************************************
 *                   TABMAPToolBlock::GetNumPen()
 *
 * Return the number of valid pen indexes for this .MAP file
 **********************************************************************/
int     TABMAPToolBlock::GetNumPen()
{
    ReadAllToolDefs();
    
    return m_numPen;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetPenDefRef()
 *
 * Return a reference to the specified Pen tool definition, or NULL if
 * specified index is invalid.
 *
 * Note that nIndex is a 1-based index.  A value of 0 indicates "none" 
 * in MapInfo.
 **********************************************************************/
TABPenDef *TABMAPToolBlock::GetPenDefRef(int nIndex)
{
    ReadAllToolDefs();
    if (nIndex >0 && nIndex <= m_numPen)
        return m_papsPen[nIndex-1];

    return NULL;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetNumBrush()
 *
 * Return the number of valid Brush indexes for this .MAP file
 **********************************************************************/
int     TABMAPToolBlock::GetNumBrushes()
{
    ReadAllToolDefs();
    
    return m_numBrushes;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetBrushDefRef()
 *
 * Return a reference to the specified Brush tool definition, or NULL if
 * specified index is invalid.
 *
 * Note that nIndex is a 1-based index.  A value of 0 indicates "none" 
 * in MapInfo.
 **********************************************************************/
TABBrushDef *TABMAPToolBlock::GetBrushDefRef(int nIndex)
{
    ReadAllToolDefs();
    if (nIndex >0 && nIndex <= m_numBrushes)
        return m_papsBrush[nIndex-1];

    return NULL;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetNumFonts()
 *
 * Return the number of valid Font indexes for this .MAP file
 **********************************************************************/
int     TABMAPToolBlock::GetNumFonts()
{
    ReadAllToolDefs();
    
    return m_numFonts;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetFontDefRef()
 *
 * Return a reference to the specified Font tool definition, or NULL if
 * specified index is invalid.
 *
 * Note that nIndex is a 1-based index.  A value of 0 indicates "none" 
 * in MapInfo.
 **********************************************************************/
TABFontDef *TABMAPToolBlock::GetFontDefRef(int nIndex)
{
    ReadAllToolDefs();
    if (nIndex >0 && nIndex <= m_numFonts)
        return m_papsFont[nIndex-1];

    return NULL;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetNumSymbols()
 *
 * Return the number of valid Symbol indexes for this .MAP file
 **********************************************************************/
int     TABMAPToolBlock::GetNumSymbols()
{
    ReadAllToolDefs();
    
    return m_numSymbols;
}

/**********************************************************************
 *                   TABMAPToolBlock::GetSymbolDefRef()
 *
 * Return a reference to the specified Symbol tool definition, or NULL if
 * specified index is invalid.
 *
 * Note that nIndex is a 1-based index.  A value of 0 indicates "none" 
 * in MapInfo.
 **********************************************************************/
TABSymbolDef *TABMAPToolBlock::GetSymbolDefRef(int nIndex)
{
    ReadAllToolDefs();
    if (nIndex >0 && nIndex <= m_numSymbols)
        return m_papsSymbol[nIndex-1];

    return NULL;
}




/**********************************************************************
 *                   TABMAPToolBlock::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABMAPToolBlock::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABMAPToolBlock::Dump() -----\n");
    if (m_pabyBuf == NULL)
    {
        fprintf(fpOut, "Block has not been initialized yet.");
    }
    else
    {
        fprintf(fpOut,"Coordinate Block (type %d) at offset %d.\n", 
                                                 m_nBlockType, m_nFileOffset);
        fprintf(fpOut,"  m_numDataBytes        = %d\n", m_numDataBytes);
        fprintf(fpOut,"  m_nNextToolBlock     = %d\n", m_nNextToolBlock);
    }

    fflush(fpOut);
}

#endif // DEBUG




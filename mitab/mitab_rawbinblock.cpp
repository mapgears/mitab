/**********************************************************************
 * $Id: mitab_rawbinblock.cpp,v 1.2 1999-09-16 02:39:17 daniel Exp $
 *
 * Name:     mitab_rawbinblock.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABRawBinBlock class used to handle
 *           reading/writing blocks in the .MAP files
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
 * $Log: mitab_rawbinblock.cpp,v $
 * Revision 1.2  1999-09-16 02:39:17  daniel
 * Completed read support for most feature types
 *
 * Revision 1.1  1999/07/12 04:18:25  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABRawBinBlock
 *====================================================================*/


/**********************************************************************
 *                   TABRawBinBlock::TABRawBinBlock()
 *
 * Constructor.
 **********************************************************************/
TABRawBinBlock::TABRawBinBlock(GBool bHardBlockSize /*= TRUE*/)
{
    m_fp = NULL;
    m_pabyBuf = NULL;
    m_nFirstBlockPtr = 0;
    m_nBlockSize = m_nSizeUsed = m_nFileOffset = m_nCurPos = 0;
    m_bHardBlockSize = bHardBlockSize;

    m_eAccess = TABRead;        // For now: only read access supported

}

/**********************************************************************
 *                   TABRawBinBlock::~TABRawBinBlock()
 *
 * Destructor.
 **********************************************************************/
TABRawBinBlock::~TABRawBinBlock()
{
    if (m_pabyBuf)
        CPLFree(m_pabyBuf);
}


/**********************************************************************
 *                   TABRawBinBlock::ReadFromFile()
 *
 * Load data from the specified file location and initialize the block.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::ReadFromFile(FILE *fpSrc, int nOffset, 
                                     int nSize /*= 512*/)
{
    GByte *pabyBuf;

    if (fpSrc == NULL || nSize == 0)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed, 
                 "TABRawBinBlock::ReadFromFile(): Assertion Failed!");
        return -1;
    }

    m_fp = fpSrc;
    m_nFileOffset = nOffset;
    m_nCurPos = 0;
    
    /*----------------------------------------------------------------
     * Alloc a buffer to contain the data
     *---------------------------------------------------------------*/
    pabyBuf = (GByte*)CPLMalloc(nSize*sizeof(GByte));

    /*----------------------------------------------------------------
     * Read from the file
     *---------------------------------------------------------------*/
    if (VSIFSeek(fpSrc, nOffset, SEEK_SET) != 0 ||
        (m_nSizeUsed = VSIFRead(pabyBuf, sizeof(GByte), nSize, fpSrc) ) == 0 ||
        (m_bHardBlockSize && m_nSizeUsed != nSize ) )
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "ReadFromFile() failed reading %d bytes at offset %d.",
                 nSize, nOffset);
        return -1;
    }

    /*----------------------------------------------------------------
     * Init block with the data we just read
     *---------------------------------------------------------------*/

    return InitBlockData(pabyBuf, nSize, FALSE, fpSrc, nOffset);
}


/**********************************************************************
 *                   TABRawBinBlock::InitBlockData()
 *
 * Set the binary data buffer and initialize the block.
 *
 * Calling ReadFromFile() will automatically call InitBlockData() to
 * complete the initialization of the block after the data is read from the
 * file.  Derived classes should implement their own version of 
 * InitBlockData() if they need specific initialization... in this
 * case the derived InitBlockData() should call TABRawBinBlock::InitBlockData()
 * before doing anything else.
 *
 * By default, the buffer will be copied, but if bMakeCopy = FALSE then
 * it won't be copied, and the object will keep a reference to the
 * user's buffer... and this object will eventually free the user's buffer.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::InitBlockData(GByte *pabyBuf, int nSize, 
                                      GBool bMakeCopy /* = TRUE */,
                                      FILE *fpSrc /* = NULL */, 
                                      int nOffset /* = 0 */)
{
    m_fp = fpSrc;
    m_nFileOffset = nOffset;
    m_nCurPos = 0;
    
    /*----------------------------------------------------------------
     * Alloc or realloc the buffer to contain the data if necessary
     *---------------------------------------------------------------*/
    if (!bMakeCopy)
    {
        if (m_pabyBuf != NULL)
            CPLFree(m_pabyBuf);
        m_pabyBuf = pabyBuf;
        m_nSizeUsed = m_nBlockSize = nSize;
    }
    else if (m_pabyBuf == NULL || nSize != m_nBlockSize)
    {
        m_pabyBuf = (GByte*)CPLRealloc(m_pabyBuf, nSize*sizeof(GByte));
        m_nSizeUsed = m_nBlockSize = nSize;
        memcpy(m_pabyBuf, pabyBuf, m_nBlockSize);
    }

    /*----------------------------------------------------------------
     * Extract block type... header block (first block in a file) has
     * no block type, so we assign one by default.
     *---------------------------------------------------------------*/
    if (m_nFileOffset == 0)
        m_nBlockType = TABMAP_HEADER_BLOCK;
    else
    {
        // Block type will be validated only if GetBlockType() is called
        m_nBlockType = (int)m_pabyBuf[0];
    }

    return 0;
}

/**********************************************************************
 *                   TABRawBinBlock::InitBlock()
 *
 * Initialize the block so that it knows to which file is is attached,
 * its block size, etc.
 *
 * This is an alternative to calling ReadFromFile() or InitBlockData()
 * that puts the block in a stable state without loading any initial
 * data in it.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::InitBlock(FILE *fpSrc, int nBlockSize)
{
    m_fp = fpSrc;
    m_nBlockSize = nBlockSize;
    m_nSizeUsed = 0;
    m_nFileOffset = 0;
    m_nCurPos = 0;

    m_nBlockType = -1;

    m_pabyBuf = (GByte*)CPLRealloc(m_pabyBuf, m_nBlockSize*sizeof(GByte));

    return 0;
}


/**********************************************************************
 *                   TABRawBinBlock::GetBlockType()
 *
 * Return the block type for the current object.
 *
 * Returns a block type >= 0 if succesful or -1 if an error happened, in 
 * which case  CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::GetBlockType()
{
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "GetBlockType(): Block has not been initialized.");
        return -1;
    }

    if (m_nBlockType > TABMAP_LAST_VALID_BLOCK_TYPE)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "GetBlockType(): Unsupported block type %d.", 
                 m_nBlockType);
        return -1;
    }

    return m_nBlockType;
}

/**********************************************************************
 *                   TABRawBinBlock::GotoByteInBlock()
 *
 * Move the block pointer to the specified position relative to the 
 * beginning of the block.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::GotoByteInBlock(int nOffset)
{
    if (nOffset > m_nSizeUsed)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "GotoByteInBlock(): Attempt to go past end of data block.");
        return -1;
    }

    if (nOffset < 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
               "GotoByteInBlock(): Attempt to go before start of data block.");
        return -1;
    }

    m_nCurPos = nOffset;

    return 0;
}

/**********************************************************************
 *                   TABRawBinBlock::GotoByteRel()
 *
 * Move the block pointer by the specified number of bytes relative
 * to its current position.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::GotoByteRel(int nOffset)
{
    return GotoByteInBlock(m_nCurPos + nOffset);
}

/**********************************************************************
 *                   TABRawBinBlock::GotoByteInFile()
 *
 * Move the block pointer to the specified position relative to the 
 * beginning of the file.  This implies that the current block may be 
 * reloaded to contain a right block of binary data if necessary.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABRawBinBlock::GotoByteInFile(int nOffset)
{
    int nNewBlockPtr;

    if (nOffset < 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
               "GotoByteInFile(): Attempt to go before start of file.");
        return -1;
    }

    nNewBlockPtr = ( (nOffset-m_nFirstBlockPtr)/m_nBlockSize)*m_nBlockSize +
                     m_nFirstBlockPtr;

    if ((nOffset < m_nFileOffset || nOffset >= m_nFileOffset+m_nSizeUsed) &&
        ReadFromFile(m_fp, nNewBlockPtr, m_nBlockSize) != 0)
    {
        // Failed reading new block... error has already been reported.
        return -1;
    }

    m_nCurPos = nOffset-m_nFileOffset;

    return 0;
}


/**********************************************************************
 *                   TABRawBinBlock::SetFirstBlockPtr()
 *
 * Set the position in the file at which the first block starts.
 * This value will usually be the header size and needs to be specified 
 * only if the header size is different from the other blocks size. 
 *
 * This value will be used by GotoByteInFile() to properly align the data
 * blocks that it loads automatically when a requested position is outside
 * of the block currently in memory.
 **********************************************************************/
void  TABRawBinBlock::SetFirstBlockPtr(int nOffset)
{
    m_nFirstBlockPtr = nOffset;
}



/**********************************************************************
 *                   TABRawBinBlock::ReadBytes()
 *
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
int     TABRawBinBlock::ReadBytes(int numBytes, GByte *pabyDstBuf)
{
    /*----------------------------------------------------------------
     * Make sure block is initialized with Read access and that the
     * operation won't go beyond the buffer's size.
     *---------------------------------------------------------------*/
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "ReadBytes(): Block has not been initialized.");
        return -1;
    }

    if (m_eAccess != TABRead && m_eAccess != TABReadWrite )
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "ReadBytes(): Block does not support read operations.");
        return -1;
    }

    if (m_nCurPos + numBytes > m_nSizeUsed)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "ReadBytes(): Attempt to read past end of data block.");
        return -1;
    }

    if (pabyDstBuf)
    {
        memcpy(pabyDstBuf, m_pabyBuf + m_nCurPos, numBytes);
    }

    m_nCurPos += numBytes;

    return 0;
}

/**********************************************************************
 *                   TABRawBinBlock::Read<datatype>()
 *
 * MapInfo files are binary files with LSB first (Intel) byte 
 * ordering.  The following functions will read from the input file
 * and return a value with the bytes ordered properly for the current 
 * platform.
 **********************************************************************/
GByte  TABRawBinBlock::ReadByte()
{
    GByte byValue;

    ReadBytes(1, (GByte*)(&byValue));

    return byValue;
}

GInt16  TABRawBinBlock::ReadInt16()
{
    GInt16 n16Value;

    ReadBytes(2, (GByte*)(&n16Value));

#ifdef CPL_MSB
    return (GInt16)CPL_SWAP16(n16Value);
#else
    return n16Value;
#endif
}

GInt32  TABRawBinBlock::ReadInt32()
{
    GInt32 n32Value;

    ReadBytes(4, (GByte*)(&n32Value));

#ifdef CPL_MSB
    return (GInt32)CPL_SWAP32(n32Value);
#else
    return n32Value;
#endif
}

float   TABRawBinBlock::ReadFloat()
{
    float fValue;

    ReadBytes(4, (GByte*)(&fValue));

#ifdef CPL_MSB
    *(GUInt32*)(&fValue) = CPL_SWAP32(*(GUInt32*)(&fValue));
#endif
    return fValue;
}

double  TABRawBinBlock::ReadDouble()
{
    double dValue;

    ReadBytes(8, (GByte*)(&dValue));

#ifdef CPL_MSB
    CPL_SWAPDOUBLE(&dValue);
#endif

    return dValue;
}



/**********************************************************************
 *                   TABRawBinBlock::WriteBytes()
 *
 * Copy the number of bytes from the user's buffer pointed by pabySrcBuf
 * to the data block's internal buffer.
 * Note that this call only writes to the memory buffer... nothing is
 * written to the file until WriteToFile() is called.
 *
 * Passing pabySrcBuf = NULL will only move the write pointer by the
 * specified number of bytes as if the copy had happened... but it 
 * won't crash.
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int  TABRawBinBlock::WriteBytes(int nBytesToWrite, GByte *pabySrcBuf)
{
    /*----------------------------------------------------------------
     * Make sure block is initialized with Write access and that the
     * operation won't go beyond the buffer's size.
     *---------------------------------------------------------------*/
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "WriteBytes(): Block has not been initialized.");
        return -1;
    }

    if (m_eAccess != TABWrite && m_eAccess != TABReadWrite )
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "WriteBytes(): Block does not support write operations.");
        return -1;
    }

    if (m_nCurPos + nBytesToWrite > m_nSizeUsed)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "WriteBytes(): Attempt to write past end of data block.");
        return -1;
    }

    /*----------------------------------------------------------------
     * Everything is OK... copy the data
     *---------------------------------------------------------------*/
    if (pabySrcBuf)
    {
        memcpy(m_pabyBuf + m_nCurPos, pabySrcBuf, nBytesToWrite);
    }

    m_nCurPos += nBytesToWrite;

    return 0;
}


/**********************************************************************
 *                    TABRawBinBlock::Write<datatype>()
 *
 * Arc/Info files are binary files with MSB first (Motorola) byte 
 * ordering.  The following functions will reorder the byte for the
 * value properly and write that to the output file.
 *
 * If a problem happens, then CPLError() will be called and 
 * CPLGetLastErrNo() can be used to test if a write operation was 
 * succesful.
 **********************************************************************/
int  TABRawBinBlock::WriteByte(GByte byValue)
{
    return WriteBytes(1, (GByte*)&byValue);
}

int  TABRawBinBlock::WriteInt16(GInt16 n16Value)
{
#ifdef CPL_MSB
    n16Value = (GInt16)CPL_SWAP16(n16Value);
#endif

    return WriteBytes(2, (GByte*)&n16Value);
}

int  TABRawBinBlock::WriteInt32(GInt32 n32Value)
{
#ifdef CPL_MSB
    n32Value = (GInt32)CPL_SWAP32(n32Value);
#endif

    return WriteBytes(4, (GByte*)&n32Value);
}

int  TABRawBinBlock::WriteFloat(float fValue)
{
#ifdef CPL_MSB
    *(GUInt32*)(&fValue) = CPL_SWAP32(*(GUInt32*)(&fValue));
#endif

    return WriteBytes(4, (GByte*)&fValue);
}

int  TABRawBinBlock::WriteDouble(double dValue)
{
#ifdef CPL_MSB
    CPL_SWAPDOUBLE(&dValue);
#endif

    return WriteBytes(8, (GByte*)&dValue);
}


/**********************************************************************
 *                    TABRawBinBlock::WriteZeros()
 *
 * Write a number of zeros (sepcified in bytes) at the current position 
 * in the file.
 *
 * If a problem happens, then CPLError() will be called and 
 * CPLGetLastErrNo() can be used to test if a write operation was 
 * succesful.
 **********************************************************************/
int  TABRawBinBlock::WriteZeros(int nBytesToWrite)
{
    char acZeros[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int i;
    int nStatus = 0;

    /* Write by 8 bytes chunks.  The last chunk may be less than 8 bytes 
     */
    for(i=0; nStatus == 0 && i< nBytesToWrite; i+=8)
    {
        nStatus = WriteBytes(MIN(8,(nBytesToWrite-i)), (GByte*)acZeros);
    }

    return nStatus;
}

/**********************************************************************
 *                   TABRawBinBlock::WritePaddedString()
 *
 * Write a string and pad the end of the field (up to nFieldSize) with
 * spaces number of spaces at the current position in the file.
 *
 * If a problem happens, then CPLError() will be called and 
 * CPLGetLastErrNo() can be used to test if a write operation was 
 * succesful.
 **********************************************************************/
int  TABRawBinBlock::WritePaddedString(int nFieldSize, const char *pszString)
{
    char acSpaces[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    int i, nLen, numSpaces;
    int nStatus = 0;

    nLen = strlen(pszString);
    nLen = MIN(nLen, nFieldSize);
    numSpaces = nFieldSize - nLen;

    if (nLen > 0)
        nStatus = WriteBytes(nLen, (GByte*)pszString);

    /* Write spaces by 8 bytes chunks.  The last chunk may be less than 8 bytes
     */
    for(i=0; nStatus == 0 && i< numSpaces; i+=8)
    {
        nStatus = WriteBytes(MIN(8,(numSpaces-i)), (GByte*)acSpaces);
    }

    return nStatus;
}

/**********************************************************************
 *                   TABRawBinBlock::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABRawBinBlock::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABRawBinBlock::Dump() -----\n");
    if (m_pabyBuf == NULL)
    {
        fprintf(fpOut, "Block has not been initialized yet.");
    }
    else
    {
        fprintf(fpOut, "Block (type %d) size=%d bytes at offset %d in file.\n",
                m_nBlockType, m_nBlockSize, m_nFileOffset);
        fprintf(fpOut, "Current pointer at byte %d\n", m_nCurPos);
    }

    fflush(fpOut);
}

#endif // DEBUG



/**********************************************************************
 *                   TABCreateMAPBlockFromFile()
 *
 * Load data from the specified file location and create and initialize 
 * a TABMAP*Block of the right type to handle it.
 *
 * Returns the new object if succesful or NULL if an error happened, in 
 * which case CPLError() will have been called.
 **********************************************************************/
TABRawBinBlock *TABCreateMAPBlockFromFile(FILE *fpSrc, int nOffset, 
                                          int nSize /*= 512*/, 
                                          GBool bHardBlockSize /*= TRUE */)
{
    TABRawBinBlock *poBlock = NULL;
    GByte *pabyBuf;

    if (fpSrc == NULL || nSize == 0)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed, 
                 "TABCreateMAPBlockFromFile(): Assertion Failed!");
        return NULL;
    }

    /*----------------------------------------------------------------
     * Alloc a buffer to contain the data
     *---------------------------------------------------------------*/
    pabyBuf = (GByte*)CPLMalloc(nSize*sizeof(GByte));

    /*----------------------------------------------------------------
     * Read from the file
     *---------------------------------------------------------------*/
    if (VSIFSeek(fpSrc, nOffset, SEEK_SET) != 0 ||
        VSIFRead(pabyBuf, sizeof(GByte), nSize, fpSrc)!=(unsigned int)nSize )
    {
        CPLError(CE_Failure, CPLE_FileIO,
         "TABCreateMAPBlockFromFile() failed reading %d bytes at offset %d.",
                 nSize, nOffset);
        return NULL;
    }

    /*----------------------------------------------------------------
     * Create an object of the right type
     * Header block is different: it does not start with the object 
     * type byte but it is always the first block in a file
     *---------------------------------------------------------------*/
    if (nOffset == 0)
    {
        poBlock = new TABMAPHeaderBlock;
    }
    else
    {
        switch(pabyBuf[0])
        {
          case TABMAP_INDEX_BLOCK:
            poBlock = new TABMAPIndexBlock;
            break;
          case TABMAP_OBJECT_BLOCK:
            poBlock = new TABMAPObjectBlock;
            break;
          case TABMAP_COORD_BLOCK:
            poBlock = new TABMAPCoordBlock;
            break;
          case TABMAP_TOOL_BLOCK:
            poBlock = new TABMAPToolBlock;
            break;
          case TABMAP_GARB_BLOCK:
          default:
            poBlock = new TABRawBinBlock(bHardBlockSize);
            break;
        }
    }

    /*----------------------------------------------------------------
     * Init new object with the data we just read
     *---------------------------------------------------------------*/
    if (poBlock->InitBlockData(pabyBuf, nSize, FALSE, fpSrc, nOffset) != 0)
    {
        // Some error happened... and CPLError() has been called
        delete poBlock;
        poBlock = NULL;
    }

    return poBlock;
}



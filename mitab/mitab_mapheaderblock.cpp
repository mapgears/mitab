/**********************************************************************
 * $Id: mitab_mapheaderblock.cpp,v 1.1 1999-07-12 04:18:24 daniel Exp $
 *
 * Name:     mitab_mapheaderblock.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABHeaderBlock class used to handle
 *           reading/writing of the .MAP files' header block
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
 * $Log: mitab_mapheaderblock.cpp,v $
 * Revision 1.1  1999-07-12 04:18:24  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABMAPHeaderBlock
 *====================================================================*/


/**********************************************************************
 *                   TABMAPHeaderBlock::TABMAPHeaderBlock()
 *
 * Constructor.
 **********************************************************************/
TABMAPHeaderBlock::TABMAPHeaderBlock():
    TABRawBinBlock(TRUE)
{

}

/**********************************************************************
 *                   TABMAPHeaderBlock::~TABMAPHeaderBlock()
 *
 * Destructor.
 **********************************************************************/
TABMAPHeaderBlock::~TABMAPHeaderBlock()
{

}


/**********************************************************************
 *                   TABMAPHeaderBlock::InitBlockData()
 *
 * Perform some initialization on the block after its binary data has
 * been set or changed (or loaded from a file).
 *
 * Returns 0 if succesful or -1 if an error happened, in which case 
 * CPLError() will have been called.
 **********************************************************************/
int     TABMAPHeaderBlock::InitBlockData(GByte *pabyBuf, int nSize, 
                                         GBool bMakeCopy /* = TRUE */,
                                         FILE *fpSrc /* = NULL */, 
                                         int nOffset /* = 0 */)
{
    int i, nStatus;
    GInt32 nMagicCookie;

    /*-----------------------------------------------------------------
     * First of all, we must call the base class' InitBlockData()
     *----------------------------------------------------------------*/
    nStatus = TABRawBinBlock::InitBlockData(pabyBuf, nSize, bMakeCopy,
                                            fpSrc, nOffset);
    if (nStatus != 0)   
        return nStatus;

    /*-----------------------------------------------------------------
     * Validate block type
     * Header blocks have a magic cookie at byte 0x100
     *----------------------------------------------------------------*/
    GotoByteInBlock(0x100);
    nMagicCookie = ReadInt32();
    if (nMagicCookie != 42424242)
    {
        CPLError(CE_Failure, CPLE_FileIO,
              "ReadFromFile(): Invalid Magic Cookie: got %d expected 42424242",
                 nMagicCookie);
        CPLFree(m_pabyBuf);
        m_pabyBuf = NULL;
        return -1;
    }

    /*-----------------------------------------------------------------
     * Init member variables
     * Instead of having over 30 get/set methods, we'll make all data 
     * members public and we will initialize them here.  
     * For this reason, this class should be used with care.
     *----------------------------------------------------------------*/
    GotoByteInBlock(0x104);
    m_nVersionNumber = ReadInt16();
    m_nBlockSize = ReadInt16();

    GotoByteInBlock(0x110);
    m_nXMin = ReadInt32();
    m_nYMin = ReadInt32();
    m_nXMax = ReadInt32();
    m_nYMax = ReadInt32();

    GotoByteInBlock(0x130);
    m_nFirstIndexBlock = ReadInt32();
    m_nFirstGarbageBlock = ReadInt32();
    m_nFirstRenditionBlock = ReadInt32();

    m_numPointObjects = ReadInt32();
    m_numLineObjects = ReadInt32();
    m_numRegionObjects = ReadInt32();
    m_numTextObjects = ReadInt32();

    GotoByteInBlock(0x15f);
    m_nMaxSpIndexDepth = ReadByte();

    GotoByteInBlock(0x16d);
    m_sProj.nProjId  = ReadByte();
    m_sProj.nEllipsoidId = ReadByte();
    m_sProj.nUnitsId = ReadByte();
    m_XScale = ReadDouble();
    m_YScale = ReadDouble();
    m_XDispl = ReadDouble();
    m_YDispl = ReadDouble();

    for(i=0; i<6; i++)
        m_sProj.adProjParams[i] = ReadDouble();

    m_sProj.dDatumShiftX = ReadDouble();
    m_sProj.dDatumShiftY = ReadDouble();
    m_sProj.dDatumShiftZ = ReadDouble();
    for(i=0; i<5; i++)
        m_sProj.adDatumParams[i] = ReadDouble();

    return 0;
}


/**********************************************************************
 *                   TABMAPHeaderBlock::Int2Coordsys()
 *
 * Convert from long integer (internal) to coordinates system units
 * as defined in the file's coordsys clause.
 *
 * Note that the false easting/northing and the conversion factor from
 * datum to coordsys units are not included in the calculation.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPHeaderBlock::Int2Coordsys(GInt32 nX, GInt32 nY, 
                                    double &dX, double &dY)
{
    if (m_pabyBuf == NULL)
        return -1;

    // For some obscure reason, the X axis values are arbitrarily 
    // scaled by -1 !!!
    dX = -1.0 * (nX - m_XDispl) / m_XScale;
    dY = (nY - m_YDispl) / m_YScale;

    return 0;
}

/**********************************************************************
 *                   TABMAPHeaderBlock::Coordsys2Int()
 *
 * Convert from coordinates system units as defined in the file's 
 * coordsys clause to long integer (internal) coordinates.
 *
 * Note that the false easting/northing and the conversion factor from
 * datum to coordsys units are not included in the calculation.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPHeaderBlock::Coordsys2Int(double dX, double dY, 
                                    GInt32 &nX, GInt32 &nY)
{
    if (m_pabyBuf == NULL)
        return -1;

    // For some obscure reason, the X axis values are arbitrarily 
    // scaled by -1 !!!
    nX = (GInt32)(dX*m_XScale + m_XDispl) * -1;
    nY = (GInt32)(dY*m_YScale + m_YDispl);

    return 0;
}

/**********************************************************************
 *                   TABMAPHeaderBlock::ComprInt2Coordsys()
 *
 * Convert from compressed integer (internal) to coordinates system units
 * as defined in the file's coordsys clause.
 * The difference between long integer and compressed integer coords is
 * that compressed coordinates are scaled displacement relative to an 
 * object centroid.
 *
 * Note that the false easting/northing and the conversion factor from
 * datum to coordsys units are not included in the calculation.
 *
 * __TODO__ Is this function really useful???? We could simply add the
 *          2 integer values and call Int2Coordsys()...
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPHeaderBlock::ComprInt2Coordsys(GInt32 nCenterX, GInt32 nCenterY, 
                                         int nDeltaX, int nDeltaY, 
                                         double &dX, double &dY)
{
    if (m_pabyBuf == NULL)
        return -1;
    // Compute the centroid coords
    dX = (nCenterX - m_XDispl) / m_XScale;
    dY = (nCenterY - m_YDispl) / m_YScale;

    // Add scaled displacement
    dX = dX + (nDeltaX / m_XScale);
    dY = dY + (nDeltaY / m_YScale);

    // For some obscure reason, the X axis values are arbitrarily 
    // scaled by -1 !!!
    dX *= -1.0;

    return 0;
}


/**********************************************************************
 *                   TABMAPHeaderBlock::Int2CoordsysDist()
 *
 * Convert a pair of X and Y size (or distance) value from long integer
 * (internal) to coordinates system units as defined in the file's 
 * coordsys clause.
 *
 * The difference with Int2Coordsys() is that this function only applies
 * the scaling factor: it does not apply the displacement.
 *
 * Since the calculations on the X and Y values are independent, either
 * one can be omitted (i.e. passed as 0)
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPHeaderBlock::Int2CoordsysDist(GInt32 nX, GInt32 nY, 
                                    double &dX, double &dY)
{
    if (m_pabyBuf == NULL)
        return -1;

    dX = nX / m_XScale;
    dY = nY / m_YScale;

    return 0;
}

/**********************************************************************
 *                   TABMAPHeaderBlock::Coordsys2IntDist()
 *
 * Convert a pair of X and Y size (or distance) values from coordinates
 * system units as defined in the file's coordsys clause to long integer
 * (internal) coordinates.
 *
 * The difference with Coordsys2Int() is that this function only applies
 * the scaling factor: it does not apply the displacement.
 *
 * Since the calculations on the X and Y values are independent, either
 * one can be omitted (i.e. passed as 0)
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPHeaderBlock::Coordsys2IntDist(double dX, double dY, 
                                        GInt32 &nX, GInt32 &nY)
{
    if (m_pabyBuf == NULL)
        return -1;

    nX = (GInt32)(dX*m_XScale);
    nY = (GInt32)(dY*m_YScale);

    return 0;
}


/**********************************************************************
 *                   TABMAPHeaderBlock::GetMapObjectSize()
 *
 * Return the size of the object body for the specified object type.
 * The value is looked up in the first 256 bytes of the header.
 **********************************************************************/
int TABMAPHeaderBlock::GetMapObjectSize(int nObjType)
{
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Block has not been initialized yet!");
        return -1;
    }

    if (nObjType < 0 || nObjType > 255)
    {
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "Invalid object type %d", nObjType);
        return -1;
    }

    // Byte 0x80 is set for objects that have coordinates inside type 3 blocks
    return (m_pabyBuf[nObjType] & 0x7f);
}

/**********************************************************************
 *                   TABMAPHeaderBlock::MapObjectUsesCoordBlock()
 *
 * Return TRUE if the specified map object type has coordinates stored
 * inside type 3 coordinate blocks.
 * The info is looked up in the first 256 bytes of the header.
 **********************************************************************/
GBool TABMAPHeaderBlock::MapObjectUsesCoordBlock(int nObjType)
{
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Block has not been initialized yet!");
        return FALSE;
    }

    if (nObjType < 0 || nObjType > 255)
    {
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "Invalid object type %d", nObjType);
        return FALSE;
    }

    // Byte 0x80 is set for objects that have coordinates inside type 3 blocks

    return ((m_pabyBuf[nObjType] & 0x80) != 0) ? TRUE: FALSE;
}


/**********************************************************************
 *                   TABMAPHeaderBlock::GetProjInfo()
 *
 * Fill the psProjInfo structure with the projection parameters previously
 * read from this header block.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int  TABMAPHeaderBlock::GetProjInfo(TABProjInfo *psProjInfo)
{
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Block has not been initialized yet!");
        return -1;
    }

    if (psProjInfo)
        *psProjInfo = m_sProj;

    return 0;
}

/**********************************************************************
 *                   TABMAPHeaderBlock::SetProjInfo()
 *
 * Set the projection parameters for this dataset.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int  TABMAPHeaderBlock::SetProjInfo(TABProjInfo *psProjInfo)
{
    if (m_pabyBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Block has not been initialized yet!");
        return -1;
    }

    if (psProjInfo)
        m_sProj = *psProjInfo;

    return 0;
}


/**********************************************************************
 *                   TABMAPHeaderBlock::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABMAPHeaderBlock::Dump(FILE *fpOut /*=NULL*/)
{
    int i;

    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABMAPHeaderBlock::Dump() -----\n");

    if (m_pabyBuf == NULL)
    {
        fprintf(fpOut, "Block has not been initialized yet.");
    }
    else
    {
        fprintf(fpOut,"Version %d header block.\n", m_nVersionNumber);
        fprintf(fpOut,"  m_nBlockSize          = %d\n", m_nBlockSize);
        fprintf(fpOut,"  m_nFirstIndexBlock    = %d\n", m_nFirstIndexBlock);
        fprintf(fpOut,"  m_nFirstGarbageBlock  = %d\n", m_nFirstGarbageBlock);
        fprintf(fpOut,"  m_nFirstRenditionBlock= %d\n",m_nFirstRenditionBlock);
        fprintf(fpOut,"  m_numPointObjects     = %d\n", m_numPointObjects);
        fprintf(fpOut,"  m_numLineObjects      = %d\n", m_numLineObjects);
        fprintf(fpOut,"  m_numRegionObjects    = %d\n", m_numRegionObjects);
        fprintf(fpOut,"  m_numTextObjects      = %d\n", m_numTextObjects);

        fprintf(fpOut,"\n");
        fprintf(fpOut,"  m_nXMin               = %d\n", m_nXMin);
        fprintf(fpOut,"  m_nYMin               = %d\n", m_nYMin);
        fprintf(fpOut,"  m_nXMax               = %d\n", m_nXMax);
        fprintf(fpOut,"  m_nYMax               = %d\n", m_nYMax);
        fprintf(fpOut,"  m_XScale              = %g\n", m_XScale);
        fprintf(fpOut,"  m_YScale              = %g\n", m_YScale);
        fprintf(fpOut,"  m_XDispl              = %g\n", m_XDispl);
        fprintf(fpOut,"  m_YDispl              = %g\n", m_YDispl);

        fprintf(fpOut,"\n");
        fprintf(fpOut,"  m_sProj.nProjId       = %d\n", (int)m_sProj.nProjId);
        fprintf(fpOut,"  m_sProj.nEllipsoidId  = %d\n", 
                                                    (int)m_sProj.nEllipsoidId);
        fprintf(fpOut,"  m_sProj.nUnitsId      = %d\n", (int)m_sProj.nUnitsId);
        fprintf(fpOut,"  m_sProj.adProjParams  =");
        for(i=0; i<6; i++)
            fprintf(fpOut, " %g",  m_sProj.adProjParams[i]);
        fprintf(fpOut,"\n");

        fprintf(fpOut,"  m_sProj.dDatumShiftX  = %g\n", m_sProj.dDatumShiftX);
        fprintf(fpOut,"  m_sProj.dDatumShiftY  = %g\n", m_sProj.dDatumShiftY);
        fprintf(fpOut,"  m_sProj.dDatumShiftZ  = %g\n", m_sProj.dDatumShiftZ);
        fprintf(fpOut,"  m_sProj.adDatumParams =");
        for(i=0; i<5; i++)
            fprintf(fpOut, " %g",  m_sProj.adDatumParams[i]);
        fprintf(fpOut,"\n");

    }

    fflush(fpOut);
}

#endif // DEBUG

/**********************************************************************
 * $Id: mitab_mapfile.cpp,v 1.2 1999-09-16 02:39:16 daniel Exp $
 *
 * Name:     mitab_mapfile.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the TABMAPFile class used to handle
 *           reading/writing of the .MAP files at the MapInfo object level
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
 * $Log: mitab_mapfile.cpp,v $
 * Revision 1.2  1999-09-16 02:39:16  daniel
 * Completed read support for most feature types
 *
 * Revision 1.1  1999/07/12 04:18:24  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab.h"

/*=====================================================================
 *                      class TABMAPFile
 *====================================================================*/


/**********************************************************************
 *                   TABMAPFile::TABMAPFile()
 *
 * Constructor.
 **********************************************************************/
TABMAPFile::TABMAPFile()
{
    m_fp = NULL;
    m_pszFname = NULL;
    m_poHeader = NULL;
    m_nMaxSpIndexDepth = 0;
    m_iCurSpIndex = -1;
    m_papoSpIndex = NULL;

    m_poCurObjBlock = NULL;
    m_nCurObjPtr = -1;
    m_nCurObjType = -1;
    m_nCurObjId = -1;
    m_poCurCoordBlock = NULL;
    m_poDrawingToolBlock = NULL;

}

/**********************************************************************
 *                   TABMAPFile::~TABMAPFile()
 *
 * Destructor.
 **********************************************************************/
TABMAPFile::~TABMAPFile()
{
    Close();
}

/**********************************************************************
 *                   TABMAPFile::Open()
 *
 * Open a .MAP file, and initialize the structures to be ready to read
 * objects from it.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Open(const char *pszFname, const char *pszAccess)
{
    FILE        *fp=NULL;
    TABRawBinBlock *poBlock=NULL;

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
     * Open file for reading and try to read header block
     *----------------------------------------------------------------*/
    fp = VSIFOpen(pszFname, pszAccess);

    if (fp)
    {
        poBlock = TABCreateMAPBlockFromFile(fp, 0, 512);

        if (poBlock==NULL || poBlock->GetBlockClass() != TABMAP_HEADER_BLOCK)
        {
            if (poBlock)
                delete poBlock;
            poBlock = NULL;
            VSIFClose(fp);
            CPLError(CE_Failure, CPLE_FileIO,
                "Open() failed: %s does not appear to be a valid .MAP file",
                     pszFname);
            return -1;
        }
    }
    else
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Open() failed for %s", pszFname);
        return -1;
    }

    /*-----------------------------------------------------------------
     * File appears to be valid... set the various class members
     *----------------------------------------------------------------*/
    m_fp = fp;
    m_poHeader = (TABMAPHeaderBlock*)poBlock;
    m_pszFname = CPLStrdup(pszFname);

    m_poCurObjBlock = new TABMAPObjectBlock;
    m_poCurObjBlock->InitBlock(m_fp, 512);

    /*-----------------------------------------------------------------
     * Open associated .ID (object id index) file
     *----------------------------------------------------------------*/
    m_poIdIndex = new TABIDFile;
    if (m_poIdIndex->Open(pszFname, pszAccess) != 0)
    {
        // Failed... an error has already been reported
        Close();
        return -1;
    }

    /*-----------------------------------------------------------------
     * Default Coord filter is the MBR of the whole file
     *----------------------------------------------------------------*/
    m_XMinFilter = m_poHeader->m_nXMin;
    m_YMinFilter = m_poHeader->m_nYMin;
    m_XMaxFilter = m_poHeader->m_nXMax;
    m_YMaxFilter = m_poHeader->m_nYMax;
    Int2Coordsys(m_XMinFilter, m_YMinFilter, m_sMinFilter.x, m_sMinFilter.y);
    Int2Coordsys(m_XMaxFilter, m_YMaxFilter, m_sMaxFilter.x, m_sMaxFilter.y);

    /*-----------------------------------------------------------------
     * We can scan a file through its quad tree index... allocate an array
     * to hold the index objects from the root to the leaf we are currently
     * processing
     *----------------------------------------------------------------*/
    m_nMaxSpIndexDepth = m_poHeader->m_nMaxSpIndexDepth;
    m_papoSpIndex = (TABMAPIndexBlock**)CPLCalloc(m_nMaxSpIndexDepth,
                                                sizeof(TABMAPIndexBlock*));
    m_iCurSpIndex = -1;

    /*-----------------------------------------------------------------
     * Init the Drawing Tool block
     *----------------------------------------------------------------*/
    GetDrawingToolBlock();

    /*-----------------------------------------------------------------
     * Make sure all previous calls succeded.
     *----------------------------------------------------------------*/
    if (CPLGetLastErrorNo() != 0)
    {
        // Open Failed... an error has already been reported
        Close();
        return -1;
    }

    return 0;
}

/**********************************************************************
 *                   TABMAPFile::Close()
 *
 * Close current file, and release all memory used.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Close()
{
    if (m_fp == NULL)
        return 0;

    //__TODO__ Commit the latest changes to the file...
    
    // Delete all structures 
    delete m_poHeader;
    m_poHeader = NULL;

    if (m_nMaxSpIndexDepth > 0 && m_papoSpIndex)
    {
        int i;
        for(i=0; i< m_nMaxSpIndexDepth; i++)
        {
            if (m_papoSpIndex[i])
                delete m_papoSpIndex[i];
        }
        CPLFree(m_papoSpIndex);
        m_papoSpIndex = NULL;
        m_nMaxSpIndexDepth = 0;
        m_iCurSpIndex = -1;
    }

    if (m_poIdIndex)
    {
        m_poIdIndex->Close();
        delete m_poIdIndex;
        m_poIdIndex = NULL;
    }

    if (m_poCurObjBlock)
    {
        delete m_poCurObjBlock;
        m_poCurObjBlock = NULL;
        m_nCurObjPtr = -1;
        m_nCurObjType = -1;
        m_nCurObjId = -1;
    }

    if (m_poCurCoordBlock)
    {
        delete m_poCurCoordBlock;
        m_poCurCoordBlock = NULL;
    }

    if (m_poDrawingToolBlock)
    {
        delete m_poDrawingToolBlock;
        m_poDrawingToolBlock = NULL;
    }

    // Close file
    VSIFClose(m_fp);
    m_fp = NULL;

    CPLFree(m_pszFname);
    m_pszFname = NULL;

    return 0;
}


/**********************************************************************
 *                   TABMAPFile::Int2Coordsys()
 *
 * Convert from long integer (internal) to coordinates system units
 * as defined in the file's coordsys clause.
 *
 * Note that the false easting/northing and the conversion factor from
 * datum to coordsys units are not included in the calculation.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Int2Coordsys(GInt32 nX, GInt32 nY, double &dX, double &dY)
{
    if (m_poHeader == NULL)
        return -1;

    return m_poHeader->Int2Coordsys(nX, nY, dX, dY);
}

/**********************************************************************
 *                   TABMAPFile::Coordsys2Int()
 *
 * Convert from coordinates system units as defined in the file's 
 * coordsys clause to long integer (internal) coordinates.
 *
 * Note that the false easting/northing and the conversion factor from
 * datum to coordsys units are not included in the calculation.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Coordsys2Int(double dX, double dY, GInt32 &nX, GInt32 &nY)
{
    if (m_poHeader == NULL)
        return -1;

    return m_poHeader->Coordsys2Int(dX, dY, nX, nY);
}

/**********************************************************************
 *                   TABMAPFile::Int2CoordsysDist()
 *
 * Convert a pair of X,Y size (or distance) values from long integer
 * (internal) to coordinates system units as defined in the file's coordsys
 * clause.
 *
 * The difference with Int2Coordsys() is that this function only applies
 * the scaling factor: it does not apply the displacement.
 *
 * Since the calculations on the X and Y values are independent, either
 * one can be omitted (i.e. passed as 0)
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Int2CoordsysDist(GInt32 nX, GInt32 nY, double &dX, double &dY)
{
    if (m_poHeader == NULL)
        return -1;

    return m_poHeader->Int2CoordsysDist(nX, nY, dX, dY);
}

/**********************************************************************
 *                   TABMAPFile::Coordsys2IntDist()
 *
 * Convert a pair of X,Y size (or distance) values from coordinates 
 * system units as defined in the file's coordsys clause to long 
 * integer (internal) coordinate units.
 *
 * The difference with Int2Coordsys() is that this function only applies
 * the scaling factor: it does not apply the displacement.
 *
 * Since the calculations on the X and Y values are independent, either
 * one can be omitted (i.e. passed as 0)
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::Coordsys2IntDist(double dX, double dY, GInt32 &nX, GInt32 &nY)
{
    if (m_poHeader == NULL)
        return -1;

    return m_poHeader->Coordsys2IntDist(dX, dY, nX, nY);
}


/**********************************************************************
 *                   TABMAPFile::GetMaxObjId()
 *
 * Return the value of the biggest valid object id.
 *
 * Note that object ids are positive and start at 1.
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
GInt32 TABMAPFile::GetMaxObjId()
{
    if (m_poIdIndex)
        return m_poIdIndex->GetMaxObjId();

    return -1;
}

/**********************************************************************
 *                   TABMAPFile::MoveToObjId()
 *
 * Get ready to work with the object with the specified id.  The object
 * data pointer (inside m_poCurObjBlock) will be moved to the first byte
 * of data for this map object.  
 *
 * The object type and id (i.e. table row number) will be accessible 
 * using GetCurObjType() and GetCurObjId().
 * 
 * Note that object ids are positive and start at 1.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int   TABMAPFile::MoveToObjId(int nObjId)
{
    int nFileOffset;

    /*-----------------------------------------------------------------
     * Move map object pointer to the right location
     *----------------------------------------------------------------*/
    if (m_poIdIndex == NULL || m_poCurObjBlock == NULL ||
        (nFileOffset = m_poIdIndex->GetObjPtr(nObjId)) < 0 ||
        m_poCurObjBlock->GotoByteInFile(nFileOffset) != 0)
    {
        m_nCurObjPtr = m_nCurObjId = m_nCurObjType = -1;
        return -1;
    }

    m_nCurObjPtr = nFileOffset;
    m_nCurObjType = m_poCurObjBlock->ReadByte();
    m_nCurObjId   = m_poCurObjBlock->ReadInt32();

    // Do a consistency check...
    if (m_nCurObjId != nObjId)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "Object ID from the .ID file (%d) differs from the value "
                 "in the .MAP file (%d).  File may be corrupt.",
                 nObjId, m_nCurObjId);
        m_nCurObjPtr = m_nCurObjId = m_nCurObjType = -1;
        return -1;
    }

    return 0;
}

/**********************************************************************
 *                   TABMAPFile::GetCurObjType()
 *
 * Return the MapInfo object type of the object that the m_poCurObjBlock
 * is pointing to.  This value is set after a call to MoveToObjId().
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::GetCurObjType()
{
    return m_nCurObjType;
}

/**********************************************************************
 *                   TABMAPFile::GetCurObjId()
 *
 * Return the MapInfo object id of the object that the m_poCurObjBlock
 * is pointing to.  This value is set after a call to MoveToObjId().
 *
 * Returns a value >= 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::GetCurObjId()
{
    return m_nCurObjId;
}

/**********************************************************************
 *                   TABMAPFile::GetCurObjBlock()
 *
 * Return the m_poCurObjBlock.  If MoveToObjId() has previously been 
 * called then m_poCurObjBlock points to the beginning of the current 
 * object data.
 *
 * Returns a reference to an object owned by this TABMAPFile object, or
 * NULL on error.
 **********************************************************************/
TABMAPObjectBlock *TABMAPFile::GetCurObjBlock()
{
    return m_poCurObjBlock;
}

/**********************************************************************
 *                   TABMAPFile::GetCoordBlock()
 *
 * Return a TABMAPCoordBlock object ready to read coordinates from it.
 * The block that contains nFileOffset will automatically be
 * loaded, and if nFileOffset is the beginning of a new block then the
 * pointer will be moved to the beginning of the data.
 *
 * The contents of the returned object is only valid until the next call
 * to GetCoordBlock().
 *
 * Returns a reference to an object owned by this TABMAPFile object, or
 * NULL on error.
 **********************************************************************/
TABMAPCoordBlock *TABMAPFile::GetCoordBlock(int nFileOffset)
{
    if (m_poCurCoordBlock == NULL)
    {
        m_poCurCoordBlock = new TABMAPCoordBlock;
        m_poCurCoordBlock->InitBlock(m_fp, 512);
    }

    /*-----------------------------------------------------------------
     * Use GotoByteInFile() to go to the requested location.  This will
     * force loading the block if necessary and reading its header.
     * If nFileOffset is at the beginning of the requested block, then
     * we make sure to move the read pointer past the 8 bytes header
     * to be ready to read coordinates data
     *----------------------------------------------------------------*/
    if ( m_poCurCoordBlock->GotoByteInFile(nFileOffset) != 0)
    {
        // Failed... an error has already been reported.
        return NULL;
    }

    if (nFileOffset % 512 == 0)
        m_poCurCoordBlock->GotoByteInBlock(8);      // Skip Header

    return m_poCurCoordBlock;
}

/**********************************************************************
 *                   TABMAPFile::GetHeaderBlock()
 *
 * Return a reference to the MAP file's header block.
 *
 * The returned pointer is a reference to an object owned by this TABMAPFile
 * object and should not be deleted by the caller.
 *
 * Return NULL if file has not been opened yet.
 **********************************************************************/
TABMAPHeaderBlock *TABMAPFile::GetHeaderBlock()
{
    return m_poHeader;
}

/**********************************************************************
 *                   TABMAPFile::GetDrawingToolBlock()
 *
 * Return a TABMAPToolBlock object ready to read Drawing Tool definitions
 * from it.
 * The first drawing tool block will automatically be loaded, and the block
 * will take care of maintaining the list of tool definitions in memory.
 *
 * Returns a reference to an object owned by this TABMAPFile object, or
 * NULL on error.
 **********************************************************************/
TABMAPToolBlock *TABMAPFile::GetDrawingToolBlock()
{
    if (m_poHeader == NULL)
        return NULL;    // File not opened yet!

    if (m_poDrawingToolBlock == NULL)
    {
        m_poDrawingToolBlock = new TABMAPToolBlock;
        m_poDrawingToolBlock->InitBlock(m_fp, 512);
    
        /*-------------------------------------------------------------
         * Use GotoByteInFile() to go to the first block's location.  This will
         * force loading the block if necessary and reading its header.
         * Also make sure to move the read pointer past the 8 bytes header
         * to be ready to read drawing tools data
         *------------------------------------------------------------*/
        if ( m_poDrawingToolBlock->GotoByteInFile(m_poHeader->
                                                  m_nFirstToolBlock)!= 0)
        {
            // Failed... an error has already been reported.
            delete m_poDrawingToolBlock;
            m_poDrawingToolBlock = NULL;
            return NULL;
        }

        m_poDrawingToolBlock->GotoByteInBlock(8);      // Skip Header
    }

    return m_poDrawingToolBlock;
}

/**********************************************************************
 *                   TABMAPFile::ReadPenDef()
 *
 * Fill the TABPenDef structure with the definition of the specified pen
 * index... (1-based pen index)
 *
 * If nPenIndex==0 or is invalid, then the structure is cleared.
 *
 * Returns 0 on success, -1 on error (i.e. Pen not found).
 **********************************************************************/
int   TABMAPFile::ReadPenDef(int nPenIndex, TABPenDef *psDef)
{
    TABPenDef *psTmp;

    if (psDef && m_poDrawingToolBlock &&
        (psTmp = m_poDrawingToolBlock->GetPenDefRef(nPenIndex)) != NULL)
    {
        *psDef = *psTmp;
    }
    else if (psDef)
    {
        memset(psDef, 0, sizeof(TABPenDef));
        return -1;
    }
    return 0;
}

/**********************************************************************
 *                   TABMAPFile::ReadBrushDef()
 *
 * Fill the TABBrushDef structure with the definition of the specified Brush
 * index... (1-based Brush index)
 *
 * If nBrushIndex==0 or is invalid, then the structure is cleared.
 *
 * Returns 0 on success, -1 on error (i.e. Brush not found).
 **********************************************************************/
int   TABMAPFile::ReadBrushDef(int nBrushIndex, TABBrushDef *psDef)
{
    TABBrushDef *psTmp;

    if (psDef && m_poDrawingToolBlock &&
        (psTmp = m_poDrawingToolBlock->GetBrushDefRef(nBrushIndex)) != NULL)
    {
        *psDef = *psTmp;
    }
    else if (psDef)
    {
        memset(psDef, 0, sizeof(TABBrushDef));
        return -1;
    }
    return 0;
}

/**********************************************************************
 *                   TABMAPFile::ReadFontDef()
 *
 * Fill the TABFontDef structure with the definition of the specified Font
 * index... (1-based Font index)
 *
 * If nFontIndex==0 or is invalid, then the structure is cleared.
 *
 * Returns 0 on success, -1 on error (i.e. Font not found).
 **********************************************************************/
int   TABMAPFile::ReadFontDef(int nFontIndex, TABFontDef *psDef)
{
    TABFontDef *psTmp;

    if (psDef && m_poDrawingToolBlock &&
        (psTmp = m_poDrawingToolBlock->GetFontDefRef(nFontIndex)) != NULL)
    {
        *psDef = *psTmp;
    }
    else if (psDef)
    {
        memset(psDef, 0, sizeof(TABFontDef));
        return -1;
    }
    return 0;
}

/**********************************************************************
 *                   TABMAPFile::ReadSymbolDef()
 *
 * Fill the TABSymbolDef structure with the definition of the specified Symbol
 * index... (1-based Symbol index)
 *
 * If nSymbolIndex==0 or is invalid, then the structure is cleared.
 *
 * Returns 0 on success, -1 on error (i.e. Symbol not found).
 **********************************************************************/
int   TABMAPFile::ReadSymbolDef(int nSymbolIndex, TABSymbolDef *psDef)
{
    TABSymbolDef *psTmp;

    if (psDef && m_poDrawingToolBlock &&
        (psTmp = m_poDrawingToolBlock->GetSymbolDefRef(nSymbolIndex)) != NULL)
    {
        *psDef = *psTmp;
    }
    else if (psDef)
    {
        memset(psDef, 0, sizeof(TABSymbolDef));
        return -1;
    }
    return 0;
}

#ifdef __TODO__
/**********************************************************************
 *                   TABMAPFile::GetObjById()
 *
 * Return the geometry object with the specified id from the .MAP file.
 *
 * Note that object ids are positive and start at 1.
 *
 * Returns a pointer to a new TABGeometry object, or NULL if it failed.
 **********************************************************************/
TABGeometry *TABMAPFile::GetObjById(GInt32 nObjId)
{
    GInt32 nFileOffset;

    if (m_poIDBlock == NULL ||
        (nFileOffset = m_poIDBlock->GetObjPtr(nObjId)) < 0)
    {
        return NULL;
    }

    return GetObjAtPtr(nFileOffset);
}

/**********************************************************************
 *                   TABMAPFile::GetObjAtPtr()
 *
 * Return the geometry object that starts at the specified location in
 * the .MAP file.
 *
 * Returns a pointer to a new TABGeometry object, or NULL if it failed.
 **********************************************************************/
TABGeometry *TABMAPFile::GetObjAtPtr(GInt32 nFileOffset)
{
    TABGeometry *poGeometry = NULL;

    /*-----------------------------------------------------------------
     * Move map object pointer to the right location
     *----------------------------------------------------------------*/
    if (m_poCurObjBlock->GotoByteInFile(nFileOffset) != 0)
    {
        // Failed... and an error message has already been produced
        return NULL;
    }

    switch(m_poCurObjBlock->GetCurObjType())
    {
      case TAB_GEOM_SYMBOL_C:
      case TAB_GEOM_SYMBOL:
      case TAB_GEOM_FONTSYMBOL_C:
      case TAB_GEOM_FONTSYMBOL:
      case TAB_GEOM_CUSTOMSYMBOL_C:
      case TAB_GEOM_CUSTOMSYMBOL:
        poGeometry = new TABGSymbol;
        break;
      case TAB_GEOM_TEXT_C:
      case TAB_GEOM_TEXT:
        poGeometry = new TABGText;
        break;

      case TAB_GEOM_LINE_C:
      case TAB_GEOM_LINE:
      case TAB_GEOM_PLINE_C:
      case TAB_GEOM_PLINE:
      case TAB_GEOM_MULTIPLINE_C:
      case TAB_GEOM_MULTIPLINE:
       poGeometry = new TABGPolyline;
        break;
      case TAB_GEOM_ARC_C:
      case TAB_GEOM_ARC:
        poGeometry = new TABGArc;
        break;

      case TAB_GEOM_REGION_C:
      case TAB_GEOM_REGION:
        poGeometry = new TABGRegion;
        break;
      case TAB_GEOM_RECT_C:
      case TAB_GEOM_RECT:
        poGeometry = new TABGRectangle;
        break;
      case TAB_GEOM_ROUNDRECT_C:
      case TAB_GEOM_ROUNDRECT:
        poGeometry = new TABGRectangle;
        break;
      case TAB_GEOM_ELLIPSE_C:
      case TAB_GEOM_ELLIPSE:
        poGeometry = new TABGEllipse;
        break;
      default:
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Unsupported object type %d", 
                 m_poCurObjBlock->GetCurObjType());
        return NULL;
    }

    return poGeometry;
}

#endif /* __TODO__ */

/**********************************************************************
 *                   TABMAPFile::SetCoordFilter()
 *
 * Set the MBR of the area of interest... only objects that at least 
 * overlap with that area will be returned.
 *
 * sMin and sMax and the min/max expressed in the file's projection coord.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABMAPFile::SetCoordFilter(TABVertex &sMin, TABVertex &sMax)
{
    m_sMinFilter = sMin;
    m_sMaxFilter = sMax;

    Coordsys2Int(sMin.x, sMin.y, m_XMinFilter, m_YMinFilter);
    Coordsys2Int(sMax.x, sMax.y, m_XMaxFilter, m_YMaxFilter);

    return 0;
}


/**********************************************************************
 *                   TABMAPFile::Dump()
 *
 * Dump block contents... available only in DEBUG mode.
 **********************************************************************/
#ifdef DEBUG

void TABMAPFile::Dump(FILE *fpOut /*=NULL*/)
{
    if (fpOut == NULL)
        fpOut = stdout;

    fprintf(fpOut, "----- TABMAPFile::Dump() -----\n");

    if (m_fp == NULL)
    {
        fprintf(fpOut, "File is not opened.\n");
    }
    else
    {
        fprintf(fpOut, "File is opened: %s\n", m_pszFname);
        fprintf(fpOut, "Coordsys filter  = (%g,%g)-(%g,%g)\n", 
                m_sMinFilter.x, m_sMinFilter.y, m_sMaxFilter.x,m_sMaxFilter.y);
        fprintf(fpOut, "Int coord filter = (%d,%d)-(%d,%d)\n", 
                m_XMinFilter, m_YMinFilter, m_XMaxFilter,m_YMaxFilter);

        fprintf(fpOut, "\nFile Header follows ...\n\n");
        m_poHeader->Dump(fpOut);
        fprintf(fpOut, "... end of file header.\n\n");

        fprintf(fpOut, "Associated .ID file ...\n\n");
        m_poIdIndex->Dump(fpOut);
        fprintf(fpOut, "... end of ID file dump.\n\n");
    }

    fflush(fpOut);
}

#endif // DEBUG

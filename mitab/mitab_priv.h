/**********************************************************************
 * $Id: mitab_priv.h,v 1.1 1999-07-12 04:18:25 daniel Exp $
 *
 * Name:     mitab_priv.h
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Header file containing private definitions for the library.
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
 * $Log: mitab_priv.h,v $
 * Revision 1.1  1999-07-12 04:18:25  daniel
 * Initial checkin
 *
 **********************************************************************/

#ifndef _MITAB_PRIV_H_INCLUDED_
#define _MITAB_PRIV_H_INCLUDED_

#include "cpl_conv.h"
#include "cpl_string.h"

class TABFeature;

/*---------------------------------------------------------------------
 * Access mode: Read or Write
 *--------------------------------------------------------------------*/
typedef enum
{
    TABRead,
    TABWrite,
    TABReadWrite  /* ReadWrite not implemented yet */
} TABAccess;

/*---------------------------------------------------------------------
 * Supported .MAP block types (the first byte at the beginning of a block)
 *--------------------------------------------------------------------*/
#define TAB_RAWBIN_BLOCK        -1
#define TABMAP_HEADER_BLOCK     0
#define TABMAP_INDEX_BLOCK      1
#define TABMAP_OBJECT_BLOCK     2
#define TABMAP_COORD_BLOCK      3
#define TABMAP_GARB_BLOCK       4
#define TABMAP_REND_BLOCK       5
#define TABMAP_LAST_VALID_BLOCK_TYPE  5

/*---------------------------------------------------------------------
 * struct TABMAPIndexEntry - Entries found in type 1 blocks of .MAP files
 *
 * We will use this struct to rebuild the geographic index in memory
 *--------------------------------------------------------------------*/

typedef struct TABMAPIndexEntry_t
{
    // These members refer to the info we find in the file
    GInt32      XMin;
    GInt32      YMin;
    GInt32      XMax;
    GInt32      YMax;
    GInt32      nBlockOffset;

    // These members are used to build the geographic index in memory.
// __TODO__  ... this may not be used until writing is implemented
//    int         numChildren;
//    struct TABMAPIndexEntry_ *pasChildren;
}TABMAPIndexEntry;


/*---------------------------------------------------------------------
 * TABVertex 
 *--------------------------------------------------------------------*/
typedef struct TABVertex_t
{
    double x;
    double y;
} TABVertex;

/*---------------------------------------------------------------------
 * TABFieldType - MapInfo attribute field types
 *--------------------------------------------------------------------*/
typedef enum
{
    TABFUnknown = 0,
    TABFChar,
    TABFInteger,
    TABFSmallInt,
    TABFDecimal,
    TABFFloat,
    TABFDate,
    TABFLogical
} TABFieldType;

/*---------------------------------------------------------------------
 * TABDATFieldDef
 *--------------------------------------------------------------------*/
typedef struct TABDATFieldDef_t
{
    char        szName[11];
    char        cType;
    GByte       byLength;
    GByte       byDecimals;

    TABFieldType eTABType;
} TABDATFieldDef;

/*---------------------------------------------------------------------
 * TABMAPCoordSecHdr
 * struct used in the TABMAPCoordBlock to store info about the coordinates
 * for a section of a PLINE MULTIPLE or a REGION.
 *--------------------------------------------------------------------*/
typedef struct TABMAPCoordSecHdr_t
{
    GInt16      numVertices;
    GInt16      numHoles;
    GInt32      nXMin;
    GInt32      nYMin;
    GInt32      nXMax;
    GInt32      nYMax;

    GInt32      nDataOffset;
    int         nVertexOffset;
} TABMAPCoordSecHdr;

/*---------------------------------------------------------------------
 * TABProjInfo
 * struct used to store the projection parameters from the .MAP header
 *--------------------------------------------------------------------*/
typedef struct TABProjInfo_t
{
    GByte       nProjId;           // See MapInfo Ref. Manual, App. F and G
    GByte       nEllipsoidId;
    GByte       nUnitsId;
    double      adProjParams[6];   // params in same order as in .MIF COORDSYS

    double      dDatumShiftX;
    double      dDatumShiftY;
    double      dDatumShiftZ;
    double      adDatumParams[5];

} TABProjInfo;


/*=====================================================================
          Classes to handle .MAP files low-level blocks
 =====================================================================*/

/*---------------------------------------------------------------------
 *                      class TABRawBinBlock
 *
 * This is the base class used for all other data block types... it 
 * contains all the base functions to handle binary data.
 *--------------------------------------------------------------------*/

class TABRawBinBlock
{
  protected:
    FILE        *m_fp;          /* Associated file handle               */
    TABAccess   m_eAccess;      /* Read/Write access mode               */

    int         m_nBlockType;

    GByte       *m_pabyBuf;     /* Buffer to contain the block's data    */
    int         m_nBlockSize;   /* Size of current block (and buffer)    */
    int         m_nSizeUsed;    /* Number of bytes used in buffer        */
    GBool       m_bHardBlockSize;/* TRUE=Blocks MUST always be nSize bytes  */
                                 /* FALSE=last block may be less than nSize */
    int         m_nFileOffset;  /* Location of current block in the file */
    int         m_nCurPos;      /* Next byte to read from m_pabyBuf[]    */
    int         m_nFirstBlockPtr;/* Size of file header when different from */
                                 /* block size (used by GotoByteInFile())   */

  public:
    TABRawBinBlock(GBool bHardBlockSize = TRUE);
    virtual ~TABRawBinBlock();

    virtual int ReadFromFile(FILE *fpSrc, int nOffset, int nSize = 512);
//  virtual int WriteToFile(FILE *fpDst = NULL, int nOffset = 0);

    virtual int InitBlockData(GByte *pabyBuf, int nSize, 
                              GBool bMakeCopy = TRUE,
                              FILE *fpSrc = NULL, int nOffset = 0);
    virtual int InitBlock(FILE *fpSrc, int nBlockSize);

    int         GetBlockType();
    virtual int GetBlockClass() { return TAB_RAWBIN_BLOCK; };

#ifdef DEBUG
    virtual void Dump(FILE *fpOut = NULL);
#endif

    int         GotoByteRel(int nOffset);
    int         GotoByteInBlock(int nOffset);
    int         GotoByteInFile(int nOffset);
    void        SetFirstBlockPtr(int nOffset);

    virtual int ReadBytes(int numBytes, GByte *pabyDstBuf);
    GByte       ReadByte();
    GInt16      ReadInt16();
    GInt32      ReadInt32();
    float       ReadFloat();
    double      ReadDouble();

    virtual int WriteBytes(int nBytesToWrite, GByte *pBuf);
    int         WriteByte(GByte byValue);
    int         WriteInt16(GInt16 n16Value);
    int         WriteInt32(GInt32 n32Value);
    int         WriteFloat(float fValue);
    int         WriteDouble(double dValue);
    int         WriteZeros(int nBytesToWrite);
    int         WritePaddedString(int nFieldSize, const char *pszString);
};


/*---------------------------------------------------------------------
 *                      class TABMAPHeaderBlock
 *
 * Class to handle Read/Write operation on .MAP Header Blocks 
 *--------------------------------------------------------------------*/

class TABMAPHeaderBlock: public TABRawBinBlock
{
  protected:
    TABProjInfo m_sProj;

  public:
    TABMAPHeaderBlock();
    ~TABMAPHeaderBlock();

    virtual int InitBlockData(GByte *pabyBuf, int nSize, 
                              GBool bMakeCopy = TRUE,
                              FILE *fpSrc = NULL, int nOffset = 0);

    virtual int GetBlockClass() { return TABMAP_HEADER_BLOCK; };

    int         Int2Coordsys(GInt32 nX, GInt32 nY, double &dX, double &dY);
    int         Coordsys2Int(double dX, double dY, GInt32 &nX, GInt32 &nY);
    int         ComprInt2Coordsys(GInt32 nCenterX, GInt32 nCenterY, 
                                  int nDeltaX, int nDeltaY, 
                                  double &dX, double &dY);
    int         Int2CoordsysDist(GInt32 nX, GInt32 nY, double &dX, double &dY);
    int         Coordsys2IntDist(double dX, double dY, GInt32 &nX, GInt32 &nY);

    int         GetMapObjectSize(int nObjType);
    GBool       MapObjectUsesCoordBlock(int nObjType);

    int         GetProjInfo(TABProjInfo *psProjInfo);
    int         SetProjInfo(TABProjInfo *psProjInfo);

#ifdef DEBUG
    virtual void Dump(FILE *fpOut = NULL);
#endif

    // Instead of having over 30 get/set methods, we'll make all data 
    // members public and we will initialize them in the overloaded
    // LoadFromFile().  For this reason, this class should be used with care.

    GInt16      m_nVersionNumber;
    GInt16      m_nBlockSize;
    
    GInt32      m_nFirstIndexBlock;
    GInt32      m_nFirstGarbageBlock;
    GInt32      m_nFirstRenditionBlock;
    GInt32      m_numPointObjects;
    GInt32      m_numLineObjects;
    GInt32      m_numRegionObjects;
    GInt32      m_numTextObjects;
    GByte       m_nMaxSpIndexDepth;

    GInt32      m_nXMin;
    GInt32      m_nYMin;
    GInt32      m_nXMax;
    GInt32      m_nYMax;

    double      m_XScale;
    double      m_YScale;
    double      m_XDispl;
    double      m_YDispl;
    
};

/*---------------------------------------------------------------------
 *                      class TABMAPIndexBlock
 *
 * Class to handle Read/Write operation on .MAP Index Blocks (Type 01)
 *--------------------------------------------------------------------*/

class TABMAPIndexBlock: public TABRawBinBlock
{
  protected:
    int         m_numEntries;

  public:
    TABMAPIndexBlock();
    ~TABMAPIndexBlock();

    virtual int InitBlockData(GByte *pabyBuf, int nSize, 
                              GBool bMakeCopy = TRUE,
                              FILE *fpSrc = NULL, int nOffset = 0);

    virtual int GetBlockClass() { return TABMAP_INDEX_BLOCK; };

#ifdef DEBUG
    virtual void Dump(FILE *fpOut = NULL);
#endif

    int         ReadNextEntry(TABMAPIndexEntry &sEntry);
//    int         WriteNextEntry(TABMAPIndexEntry &sEntry);

};

/*---------------------------------------------------------------------
 *                      class TABMAPObjectBlock
 *
 * Class to handle Read/Write operation on .MAP Object data Blocks (Type 02)
 *--------------------------------------------------------------------*/

class TABMAPObjectBlock: public TABRawBinBlock
{
  protected:
    int         m_numDataBytes; /* Excluding first 4 bytes header */
    GInt32      m_nFirstCoordBlock;
    GInt32      m_nLastCoordBlock;
    GInt32      m_nCenterX;
    GInt32      m_nCenterY;

  public:
    TABMAPObjectBlock();
    ~TABMAPObjectBlock();

    virtual int InitBlockData(GByte *pabyBuf, int nSize, 
                              GBool bMakeCopy = TRUE,
                              FILE *fpSrc = NULL, int nOffset = 0);

    virtual int GetBlockClass() { return TABMAP_OBJECT_BLOCK; };

    virtual int ReadIntCoord(GBool bCompressed, GInt32 &nX, GInt32 &nY);

#ifdef DEBUG
    virtual void Dump(FILE *fpOut = NULL);
#endif

};

/*---------------------------------------------------------------------
 *                      class TABMAPCoordBlock
 *
 * Class to handle Read/Write operation on .MAP Coordinate Blocks (Type 03)
 *--------------------------------------------------------------------*/

class TABMAPCoordBlock: public TABRawBinBlock
{
  protected:
    int         m_numDataBytes; /* Excluding first 8 bytes header */
    GInt32      m_nNextCoordBlock;

    GInt32      m_nCenterX;
    GInt32      m_nCenterY;

  public:
    TABMAPCoordBlock();
    ~TABMAPCoordBlock();

    virtual int InitBlockData(GByte *pabyBuf, int nSize, 
                              GBool bMakeCopy = TRUE,
                              FILE *fpSrc = NULL, int nOffset = 0);

    virtual int GetBlockClass() { return TABMAP_COORD_BLOCK; };

    virtual int ReadBytes(int numBytes, GByte *pabyDstBuf);
    void        SetComprCoordOrigin(GInt32 nX, GInt32 nY);
    int         ReadIntCoord(GBool bCompressed, GInt32 &nX, GInt32 &nY);
    int         ReadIntCoords(GBool bCompressed, int numCoords, GInt32 *panXY);
    int         ReadCoordSecHdrs(GBool bCompressed, int numSections,
                                 TABMAPCoordSecHdr *pasHdrs,
                                 int    &numVerticesTotal);

#ifdef DEBUG
    virtual void Dump(FILE *fpOut = NULL);
#endif

};


/*=====================================================================
       Classes to deal with .MAP files at the MapInfo object level
 =====================================================================*/

/*---------------------------------------------------------------------
 *                      class TABIDFile
 *
 * Class to handle Read/Write operation on .ID files... the .ID file
 * contains an index to the objects in the .MAP file by object id.
 *--------------------------------------------------------------------*/

class TABIDFile
{
  private:
    char        *m_pszFname;
    FILE        *m_fp;
    TABAccess   m_eAccessMode;

    TABRawBinBlock *m_poIDBlock;
    int         m_nBlockSize;
    GInt32      m_nMaxId;

   public:
    TABIDFile();
    ~TABIDFile();

    int         Open(const char *pszFname, const char *pszAccess);
    int         Close();

    GInt32      GetObjPtr(GInt32 nObjId);
    GInt32      GetMaxObjId();

#ifdef DEBUG
    void Dump(FILE *fpOut = NULL);
#endif

};

/*---------------------------------------------------------------------
 *                      class TABMAPFile
 *
 * Class to handle Read/Write operation on .MAP files... this class hides
 * all the dealings with blocks, indexes, etc.
 * Use this class to deal with MapInfo objects directly.
 *--------------------------------------------------------------------*/

class TABMAPFile
{
  private:
    char        *m_pszFname;
    FILE        *m_fp;
    TABAccess   m_eAccessMode;

    TABMAPHeaderBlock   *m_poHeader;

    // Members used to access objects using the spatial index
    int         m_iCurSpIndex;
    int         m_nMaxSpIndexDepth;
    TABMAPIndexBlock  **m_papoSpIndex;

    // Member used to access objects using the object ids (.ID file)
    TABIDFile   *m_poIdIndex;

    // Current object data block.
    TABMAPObjectBlock *m_poCurObjBlock;
    int         m_nCurObjPtr;
    int         m_nCurObjType;
    int         m_nCurObjId;
    TABMAPCoordBlock *m_poCurCoordBlock;

    // Coordinates filter... default is MBR of the whole file
    TABVertex   m_sMinFilter;
    TABVertex   m_sMaxFilter;
    GInt32      m_XMinFilter;
    GInt32      m_YMinFilter;
    GInt32      m_XMaxFilter;
    GInt32      m_YMaxFilter;

  public:
    TABMAPFile();
    ~TABMAPFile();

    int         Open(const char *pszFname, const char *pszAccess);
    int         Close();

    int         Int2Coordsys(GInt32 nX, GInt32 nY, double &dX, double &dY);
    int         Coordsys2Int(double dX, double dY, GInt32 &nX, GInt32 &nY);
    int         Int2CoordsysDist(GInt32 nX, GInt32 nY, double &dX, double &dY);
    int         Coordsys2IntDist(double dX, double dY, GInt32 &nX, GInt32 &nY);
    int         SetCoordFilter(TABVertex &sMin, TABVertex &sMax);

    GInt32      GetMaxObjId();
    int         MoveToObjId(int nObjId);

    int         GetCurObjType();
    int         GetCurObjId();
    TABMAPObjectBlock *GetCurObjBlock();
    TABMAPCoordBlock  *GetCoordBlock(int nFileOffset);
    TABMAPHeaderBlock *GetHeaderBlock();

#ifdef DEBUG
    void Dump(FILE *fpOut = NULL);
#endif

};


/*---------------------------------------------------------------------
 *                      class TABDATFile
 *
 * Class to handle Read/Write operation on .DAT files... the .DAT file
 * contains the table of attribute field values.
 *--------------------------------------------------------------------*/

class TABDATFile
{
  private:
    char        *m_pszFname;
    FILE        *m_fp;
    TABAccess   m_eAccessMode;

    TABRawBinBlock *m_poHeaderBlock;
    int         m_numFields;
    TABDATFieldDef *m_pasFieldDef;

    TABRawBinBlock *m_poRecordBlock;
    int         m_nBlockSize;
    int         m_nRecordSize;
    int         m_nCurRecordId;

    GInt32      m_numRecords;
    GInt32      m_nFirstRecordPtr;

   public:
    TABDATFile();
    ~TABDATFile();

    int         Open(const char *pszFname, const char *pszAccess);
    int         Close();

    int         GetNumFields();
    TABFieldType GetFieldType(int nFieldId);
    int         GetFieldWidth(int nFieldId);
    int         ValidateFieldInfoFromTAB(int iField, const char *pszName,
                                         TABFieldType eType,
                                         int nWidth, int nPrecision);

    GInt32      GetNumRecords();
    TABRawBinBlock *GetRecordBlock(int nRecordId);

    const char  *ReadCharField(int nWidth);
    GInt32      ReadIntegerField();
    GInt16      ReadSmallIntField();
    double      ReadFloatField();
    double      ReadDecimalField(int nWidth);
    const char  *ReadLogicalField();
    const char  *ReadDateField();

#ifdef DEBUG
    void Dump(FILE *fpOut = NULL);
#endif

};

/*=====================================================================
                        Function prototypes
 =====================================================================*/

TABRawBinBlock *TABCreateMAPBlockFromFile(FILE *fpSrc, int nOffset, 
                                          int nSize = 512, 
                                          GBool bHardBlockSize = TRUE );


#endif /* _MITAB_PRIV_H_INCLUDED_ */


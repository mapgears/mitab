/**********************************************************************
 * $Id: mitab.h,v 1.9 1999-09-29 04:27:14 daniel Exp $
 *
 * Name:     mitab.h
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Header file containing public definitions for the library.
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
 * $Log: mitab.h,v $
 * Revision 1.9  1999-09-29 04:27:14  daniel
 * Changed some TABFeatureClass names
 *
 * Revision 1.8  1999/09/28 13:32:10  daniel
 * Added TABFile::AddFieldNative()
 *
 * Revision 1.7  1999/09/28 02:52:47  warmerda
 * Added SetProjInfo().
 *
 * Revision 1.6  1999/09/26 14:59:36  daniel
 * Implemented write support
 *
 * Revision 1.5  1999/09/24 20:23:09  warmerda
 * added GetProjInfo method
 *
 * Revision 1.4  1999/09/23 19:49:47  warmerda
 * Added setspatialref()
 *
 * Revision 1.3  1999/09/16 02:39:16  daniel
 * Completed read support for most feature types
 *
 * Revision 1.2  1999/09/01 17:46:49  daniel
 * Added GetNativeFieldType() and GetFeatureDefn() to TABFile
 *
 * Revision 1.1  1999/07/12 04:18:23  daniel
 * Initial checkin
 *
 **********************************************************************/

#ifndef _MITAB_H_INCLUDED_
#define _MITAB_H_INCLUDED_

#include "mitab_priv.h"
#include "ogr_feature.h"

#ifndef PI
#  define PI 3.14159265358979323846
#endif


class TABFeature;

/*---------------------------------------------------------------------
 *                      class TABFile
 *
 * Main class for the whole library... use this class to open
 * a TAB dataset and read/write features from/to it.
 *--------------------------------------------------------------------*/
class TABFile
{
  private:
    char        *m_pszFname;
    TABAccess   m_eAccessMode;
    char        **m_papszTABFile;
    char        *m_pszVersion;
    char        *m_pszCharset;

    TABDATFile  *m_poDATFile;   // Attributes file
    TABMAPFile  *m_poMAPFile;   // Object Geometry file

    OGRFeatureDefn *m_poDefn;
    OGRSpatialReference *m_poSpatialRef;

    TABFeature *m_poCurFeature;
    int         m_nCurFeatureId;
    int         m_nLastFeatureId;


    ///////////////
    // Read access specific stuff
    //
    int         ReadFeatureDefn();
    int         ParseTABFile();

    ///////////////
    // Write access specific stuff
    //
    GBool       m_bBoundsSet;
    int         WriteTABFile();

  public:
    TABFile();
    ~TABFile();

    int         Open(const char *pszFname, const char *pszAccess);
    int         Close();

    ///////////////
    // Read access specific stuff
    //
    int         GetNextFeatureId(int nPrevId);
    TABFeature *GetFeatureRef(int nFeatureId);
    OGRFeatureDefn *GetFeatureDefn();

    TABFieldType GetNativeFieldType(int nFieldId);

    OGRSpatialReference *GetSpatialRef();

    ///////////////
    // Write access specific stuff
    //
    int         SetBounds(double dXMin, double dYMin, 
                          double dXMax, double dYMax);
    int         SetFeatureDefn(OGRFeatureDefn *poFeatureDefn,
                            TABFieldType *paeMapInfoNativeFieldTypes = NULL);
    int         AddFieldNative(const char *pszName, TABFieldType eMapInfoType,
                               int nWidth, int nPrecision=0);
    int         SetSpatialRef(OGRSpatialReference *poSpatialRef);

    int         SetFeature(TABFeature *poFeature, int nFeatureId = -1);

    ///////////////
    // semi-private.
    int          GetProjInfo(TABProjInfo *poPI)
	    { return m_poMAPFile->GetHeaderBlock()->GetProjInfo( poPI ); }
    int          SetProjInfo(TABProjInfo *poPI)
	    { return m_poMAPFile->GetHeaderBlock()->SetProjInfo( poPI ); }

#ifdef DEBUG
    void Dump(FILE *fpOut = NULL);
#endif
};

/*---------------------------------------------------------------------
 * Codes for the known MapInfo Geometry types
 *--------------------------------------------------------------------*/
#define TAB_GEOM_NONE           0
#define TAB_GEOM_SYMBOL_C       0x01
#define TAB_GEOM_SYMBOL         0x02
#define TAB_GEOM_LINE_C         0x04
#define TAB_GEOM_LINE           0x05
#define TAB_GEOM_PLINE_C        0x07
#define TAB_GEOM_PLINE          0x08
#define TAB_GEOM_ARC_C          0x0a
#define TAB_GEOM_ARC            0x0b
#define TAB_GEOM_REGION_C       0x0d
#define TAB_GEOM_REGION         0x0e
#define TAB_GEOM_TEXT_C         0x10
#define TAB_GEOM_TEXT           0x11
#define TAB_GEOM_RECT_C         0x13
#define TAB_GEOM_RECT           0x14
#define TAB_GEOM_ROUNDRECT_C    0x16
#define TAB_GEOM_ROUNDRECT      0x17
#define TAB_GEOM_ELLIPSE_C      0x19
#define TAB_GEOM_ELLIPSE        0x1a
#define TAB_GEOM_MULTIPLINE_C   0x25
#define TAB_GEOM_MULTIPLINE     0x26
#define TAB_GEOM_FONTSYMBOL_C   0x28 
#define TAB_GEOM_FONTSYMBOL     0x29
#define TAB_GEOM_CUSTOMSYMBOL_C 0x2b
#define TAB_GEOM_CUSTOMSYMBOL   0x2c

/*---------------------------------------------------------------------
 * Codes for the feature classes
 *--------------------------------------------------------------------*/
typedef enum
{
    TABFCNoGeomFeature,
    TABFCPoint,
    TABFCFontPoint,
    TABFCCustomPoint,
    TABFCText,
    TABFCPolyline,
    TABFCArc,
    TABFCRegion,
    TABFCRectangle,
    TABFCEllipse,
    TABFCDebugFeature
} TABFeatureClass;

/*---------------------------------------------------------------------
 * Definitions for text attributes
 *--------------------------------------------------------------------*/
typedef enum TABTextJust_t
{
    TABTJLeft,          // Default: Left Justification
    TABTJCenter,
    TABTJRight
} TABTextJust;

typedef enum TABTextSpacing_t
{
    TABTSSingle,        // Default: Single spacing
    TABTS1_5,           // 1.5
    TABTSDouble
} TABTextSpacing;

typedef enum TABTextLineType_t
{
    TABTLNoLine,        // Default: No line
    TABTLSimple,
    TABTLArrow
} TABTextLineType;

typedef enum TABFontStyle_t     // Can be OR'ed
{                               // except box and halo are mutually exclusive
    TABFSNone       = 0,
    TABFSBold       = 0x0001,
    TABFSItalic     = 0x0002,
    TABFSUnderline  = 0x0004,
    TABFSStrikeout  = 0x0008,
    TABFSOutline    = 0x0010,
    TABFSShadow     = 0x0020,
    TABFSInverse    = 0x0040,
    TABFSBlink      = 0x0080,
    TABFSBox        = 0x0100,
    TABFSHalo       = 0x0200,   // ??? MIF uses 256, see MIF docs, App.A???
    TABFSAllCaps    = 0x0400,   // ??? MIF uses 512???
    TABFSExpanded   = 0x0800    // ??? MIF uses 1024???
} TABFontStyle;


typedef enum TABCustSymbStyle_t // Can be OR'ed
{ 
    TABCSNone       = 0,        // Transparent BG, use default colors
    TABCSBGOpaque   = 0x01,     // White pixels are opaque
    TABCSApplyColor = 0x02,     // non-white pixels drawn using symbol color
} TABCustSymbStyle;

/*=====================================================================
  Base classes to be used to add supported drawing tools to each feature type
 =====================================================================*/

class ITABFeaturePen
{
  protected:
    int         m_nPenDefIndex;
    TABPenDef   m_sPenDef;
  public:
    ITABFeaturePen() { m_nPenDefIndex=-1;
                      memset(&m_sPenDef, 0, sizeof(TABPenDef)); };
    ~ITABFeaturePen() {};
    int         GetPenDefIndex() {return m_nPenDefIndex;};
    TABPenDef  *GetPenDefRef() {return &m_sPenDef;};

    void        DumpPenDef(FILE *fpOut = NULL);
};

class ITABFeatureBrush
{
  protected:
    int         m_nBrushDefIndex;
    TABBrushDef m_sBrushDef;
  public:
    ITABFeatureBrush() { m_nBrushDefIndex=-1;
                        memset(&m_sBrushDef, 0, sizeof(TABBrushDef)); };
    ~ITABFeatureBrush() {};
    int         GetBrushDefIndex() {return m_nBrushDefIndex;};
    TABBrushDef *GetBrushDefRef() {return &m_sBrushDef;};

    void        DumpBrushDef(FILE *fpOut = NULL);
};

class ITABFeatureFont
{
  protected:
    int         m_nFontDefIndex;
    TABFontDef  m_sFontDef;
  public:
    ITABFeatureFont() { m_nFontDefIndex=-1;
                       memset(&m_sFontDef, 0, sizeof(TABFontDef)); };
    ~ITABFeatureFont() {};
    int         GetFontDefIndex() {return m_nFontDefIndex;};
    TABFontDef *GetFontDefRef() {return &m_sFontDef;};
    const char *GetFontNameRef() {return m_sFontDef.szFontName;};

    void        DumpFontDef(FILE *fpOut = NULL);
};

class ITABFeatureSymbol
{
  protected:
    int         m_nSymbolDefIndex;
    TABSymbolDef m_sSymbolDef;
  public:
    ITABFeatureSymbol() { m_nSymbolDefIndex=-1;
                         memset(&m_sSymbolDef, 0, sizeof(TABSymbolDef)); };
    ~ITABFeatureSymbol() {};
    int         GetSymbolDefIndex() {return m_nSymbolDefIndex;};
    TABSymbolDef *GetSymbolDefRef() {return &m_sSymbolDef;};

    GInt16      GetSymbolNo()    {return m_sSymbolDef.nSymbolNo;};
    GInt16      GetSymbolSize()  {return m_sSymbolDef.nPointSize;};
    GInt32      GetSymbolColor() {return m_sSymbolDef.rgbColor;};

    void        DumpSymbolDef(FILE *fpOut = NULL);
};


/*=====================================================================
                        Feature Classes
 =====================================================================*/

/*---------------------------------------------------------------------
 *                      class TABFeature
 *
 * Extend the OGRFeature to support MapInfo specific extensions related
 * to geometry types, representation strings, etc.
 *
 * TABFeature will be used as a base class for all the feature classes.
 *
 * This class will also be used to instanciate objects with no Geometry
 * (i.e. type TAB_GEOM_NONE) which is a valid case in MapInfo.
 *
 * The logic to read/write the object from/to the .DAT and .MAP files is also
 * implemented as part of this class and derived classes.
 *--------------------------------------------------------------------*/
class TABFeature: public OGRFeature
{
  protected:
    int         m_nMapInfoType;

    double      m_dXMin;
    double      m_dYMin;
    double      m_dXMax;
    double      m_dYMax;

  public:
             TABFeature(OGRFeatureDefn *poDefnIn );
    virtual ~TABFeature();

    virtual TABFeatureClass GetFeatureClass() { return TABFCNoGeomFeature; };
    virtual int             GetMapInfoType()  { return m_nMapInfoType; };
    virtual int            ValidateMapInfoType(){m_nMapInfoType=TAB_GEOM_NONE;
                                                 return m_nMapInfoType;};

    virtual int ReadRecordFromDATFile(TABDATFile *poDATFile);
    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);

    virtual int WriteRecordToDATFile(TABDATFile *poDATFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);

    void        SetMBR(double dXMin, double dYMin, 
                       double dXMax, double dYMax);
    void        GetMBR(double &dXMin, double &dYMin, 
                       double &dXMax, double &dYMax);

    virtual void DumpMID(FILE *fpOut = NULL);
    virtual void DumpMIF(FILE *fpOut = NULL);
};


/*---------------------------------------------------------------------
 *                      class TABPoint
 *
 * Feature class to handle old style MapInfo point symbols:
 *
 *     TAB_GEOM_SYMBOL_C        0x01
 *     TAB_GEOM_SYMBOL          0x02
 *
 * Feature geometry will be a OGRPoint
 *
 * NOTE: This class is also used as a base class for the other point
 * symbol types TABFontPoint and TABCustomPoint.
 *--------------------------------------------------------------------*/
class TABPoint: public TABFeature, 
                public ITABFeatureSymbol
{
  public:
             TABPoint(OGRFeatureDefn *poDefnIn);
    virtual ~TABPoint();

    virtual TABFeatureClass GetFeatureClass() { return TABFCPoint; };
    virtual int             ValidateMapInfoType();

    double	GetX();
    double	GetY();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
};


/*---------------------------------------------------------------------
 *                      class TABFontPoint
 *
 * Feature class to handle MapInfo Font Point Symbol types:
 *
 *     TAB_GEOM_FONTSYMBOL_C    0x28 
 *     TAB_GEOM_FONTSYMBOL      0x29
 *
 * Feature geometry will be a OGRPoint
 *--------------------------------------------------------------------*/
class TABFontPoint: public TABPoint, 
                    public ITABFeatureFont
{
  public:
    double      m_dAngle;
    GInt16      m_nFontStyle;           // Bold/shadow/halo/etc.

  public:
             TABFontPoint(OGRFeatureDefn *poDefnIn);
    virtual ~TABFontPoint();

    virtual TABFeatureClass GetFeatureClass() { return TABFCFontPoint; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);

    GBool       QueryFontStyle(TABFontStyle eStyleToQuery);

    // GetSymbolAngle(): Return angle in degrees counterclockwise
    double      GetSymbolAngle()        {return m_dAngle;};
};


/*---------------------------------------------------------------------
 *                      class TABCustomPoint
 *
 * Feature class to handle MapInfo Custom Point Symbol (Bitmap) types:
 *
 *     TAB_GEOM_CUSTOMSYMBOL_C  0x2b
 *     TAB_GEOM_CUSTOMSYMBOL    0x2c
 *
 * Feature geometry will be a OGRPoint
 *--------------------------------------------------------------------*/
class TABCustomPoint: public TABPoint, 
                      public ITABFeatureFont
{
  public:
    GByte       m_nUnknown_;
    GByte       m_nCustomStyle;         // Show BG/Apply Color

  public:
             TABCustomPoint(OGRFeatureDefn *poDefnIn);
    virtual ~TABCustomPoint();

    virtual TABFeatureClass GetFeatureClass() { return TABFCCustomPoint; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);

    const char *GetSymbolNameRef()      { return GetFontNameRef(); };
};


/*---------------------------------------------------------------------
 *                      class TABPolyline
 *
 * Feature class to handle the various MapInfo line types:
 *
 *     TAB_GEOM_LINE_C         0x04
 *     TAB_GEOM_LINE           0x05
 *     TAB_GEOM_PLINE_C        0x07
 *     TAB_GEOM_PLINE          0x08
 *     TAB_GEOM_MULTIPLINE_C   0x25
 *     TAB_GEOM_MULTIPLINE     0x26
 *
 * Feature geometry can be either a OGRLineString or a OGRMultiLineString
 *--------------------------------------------------------------------*/
class TABPolyline: public TABFeature, 
                   public ITABFeaturePen
{
  public:
             TABPolyline(OGRFeatureDefn *poDefnIn);
    virtual ~TABPolyline();

    virtual TABFeatureClass GetFeatureClass() { return TABFCPolyline; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);

    // MapInfo-specific attributes... made available through public vars
    // for now.
    GBool       m_bSmooth;

};

/*---------------------------------------------------------------------
 *                      class TABRegion
 *
 * Feature class to handle the MapInfo region types:
 *
 *     TAB_GEOM_REGION_C         0x0d
 *     TAB_GEOM_REGION           0x0e
 *
 * Feature geometry will be OGRPolygon
 *--------------------------------------------------------------------*/
class TABRegion: public TABFeature, 
                 public ITABFeaturePen, 
                 public ITABFeatureBrush
{
    GBool       m_bSmooth;
  public:
             TABRegion(OGRFeatureDefn *poDefnIn);
    virtual ~TABRegion();

    virtual TABFeatureClass GetFeatureClass() { return TABFCRegion; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
};


/*---------------------------------------------------------------------
 *                      class TABRectangle
 *
 * Feature class to handle the MapInfo rectangle types:
 *
 *     TAB_GEOM_RECT_C         0x13
 *     TAB_GEOM_RECT           0x14
 *     TAB_GEOM_ROUNDRECT_C    0x16
 *     TAB_GEOM_ROUNDRECT      0x17
 *
 * A rectangle is defined by the coords of its 2 opposite corners (the MBR)
 * Its corners can optionaly be rounded, in which case a X and Y rounding
 * radius will be defined.
 *
 * Feature geometry will be OGRPolygon
 *--------------------------------------------------------------------*/
class TABRectangle: public TABFeature, 
                    public ITABFeaturePen, 
                    public ITABFeatureBrush
{
  public:
             TABRectangle(OGRFeatureDefn *poDefnIn);
    virtual ~TABRectangle();

    virtual TABFeatureClass GetFeatureClass() { return TABFCRectangle; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);

    // MapInfo-specific attributes... made available through public vars
    // for now.
    GBool       m_bRoundCorners;
    double      m_dRoundXRadius;
    double      m_dRoundYRadius;

};


/*---------------------------------------------------------------------
 *                      class TABEllipse
 *
 * Feature class to handle the MapInfo ellipse types:
 *
 *     TAB_GEOM_ELLIPSE_C      0x19
 *     TAB_GEOM_ELLIPSE        0x1a
 *
 * An ellipse is defined by the coords of its 2 opposite corners (the MBR)
 *
 * Feature geometry will be OGRPolygon
 *--------------------------------------------------------------------*/
class TABEllipse: public TABFeature, 
                  public ITABFeaturePen, 
                  public ITABFeatureBrush
{

  public:
             TABEllipse(OGRFeatureDefn *poDefnIn);
    virtual ~TABEllipse();

    virtual TABFeatureClass GetFeatureClass() { return TABFCEllipse; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);

    // MapInfo-specific attributes... made available through public vars
    // for now.
    double      m_dCenterX;
    double      m_dCenterY;
    double      m_dXRadius;
    double      m_dYRadius;

};


/*---------------------------------------------------------------------
 *                      class TABArc
 *
 * Feature class to handle the MapInfo arc types:
 *
 *     TAB_GEOM_ARC_C      0x0a
 *     TAB_GEOM_ARC        0x0b
 *
 * An arc is defined by the coords of the 2 opposite corners of its 
 * defining ellipse, which in this case is different from the arc's MBR,
 * and a start and end angle in degrees.
 *
 * Feature geometry will be OGRLineString
 *--------------------------------------------------------------------*/
class TABArc: public TABFeature, 
              public ITABFeaturePen
{

  public:
             TABArc(OGRFeatureDefn *poDefnIn);
    virtual ~TABArc();

    virtual TABFeatureClass GetFeatureClass() { return TABFCArc; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);

    // MapInfo-specific attributes... made available through public vars
    // for now.
    double      m_dStartAngle;  // In degrees, counterclockwise, 
    double      m_dEndAngle;    // starting at 3 o'clock

    double      m_dCenterX;
    double      m_dCenterY;
    double      m_dXRadius;
    double      m_dYRadius;
};


/*---------------------------------------------------------------------
 *                      class TABText
 *
 * Feature class to handle the MapInfo text types:
 *
 *     TAB_GEOM_TEXT_C         0x10
 *     TAB_GEOM_TEXT           0x11
 *
 * Feature geometry will be a OGRPoint
 *--------------------------------------------------------------------*/
class TABText: public TABFeature, 
               public ITABFeatureFont,
               public ITABFeaturePen
{
  protected:
    char        *m_pszString;

    double      m_dAngle;
    double      m_dHeight;

    GInt32      m_rgbForeground;
    GInt32      m_rgbBackground;

    GInt16      m_nTextAlignment;       // Justification/Vert.Spacing/arrow
  public:
    GInt16      m_nFontStyle;           // Bold/italic/underlined/shadow/...

  public:
             TABText(OGRFeatureDefn *poDefnIn);
    virtual ~TABText();

    virtual TABFeatureClass GetFeatureClass() { return TABFCText; };
    virtual int             ValidateMapInfoType();

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);

    const char *GetTextString();
    void        SetTextString(const char *pszStr);
    double      GetTextAngle();
    double      GetTextHeight();
    GInt32      GetFontFGColor();
    GInt32      GetFontBGColor();

    TABTextJust GetTextJustification();
    TABTextSpacing  GetTextSpacing();
    TABTextLineType GetTextLineType();
    GBool       QueryFontStyle(TABFontStyle eStyleToQuery);
};



/*---------------------------------------------------------------------
 *                      class TABDebugFeature
 *
 * Feature class to use for testing purposes... this one does not 
 * correspond to any MapInfo type... it's just used to dump info about
 * feature types that are not implemented yet.
 *--------------------------------------------------------------------*/
class TABDebugFeature: public TABFeature
{
  private:
    GByte       m_abyBuf[512];
    int         m_nSize;
    int         m_nCoordDataPtr;  // -1 if none
    int         m_nCoordDataSize;

  public:
             TABDebugFeature(OGRFeatureDefn *poDefnIn);
    virtual ~TABDebugFeature();

    virtual TABFeatureClass GetFeatureClass() { return TABFCDebugFeature; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual int WriteGeometryToMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
};


#endif /* _MITAB_H_INCLUDED_ */


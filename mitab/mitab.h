/**********************************************************************
 * $Id: mitab.h,v 1.1 1999-07-12 04:18:23 daniel Exp $
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
 * Revision 1.1  1999-07-12 04:18:23  daniel
 * Initial checkin
 *
 **********************************************************************/

#ifndef _MITAB_H_INCLUDED_
#define _MITAB_H_INCLUDED_

#include "mitab_priv.h"
#include "ogr_feature.h"

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

    TABFeature  *m_poCurFeature;
    int         m_nCurFeatureId;
    int         m_nLastFeatureId;

    int         ReadFeatureDefn();

    int         ParseTABFile();

  public:
    TABFile();
    ~TABFile();

    int         Open(const char *pszFname, const char *pszAccess);
    int         Close();

    int         GetNextFeatureId(int nPrevId);
    TABFeature *GetFeatureRef(int nFeatureId);

    OGRSpatialReference *GetSpatialRef();

#ifdef DEBUG
    void Dump(FILE *fpOut = NULL);
#endif
};

/*---------------------------------------------------------------------
 * Codes for the known MapInfo Geometry types
 *--------------------------------------------------------------------*/

#define TAB_GEOM_OLDSYMBOL_C    0x01
#define TAB_GEOM_OLDSYMBOL      0x02
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
#define TAB_GEOM_FNTSYMBOL_C    0x28 
#define TAB_GEOM_FNTSYMBOL      0x29
#define TAB_GEOM_BMPSYMBOL_C    0x2b
#define TAB_GEOM_BMPSYMBOL      0x2c

/*---------------------------------------------------------------------
 * Codes for the feature classes
 *--------------------------------------------------------------------*/
typedef enum
{
    TABFCBaseFeature,
    TABFCPoint,
    TABFCText,
    TABFCPolyline,
    TABFCArc,
    TABFCRegion,
    TABFCRectangle,
    TABFCEllipse
} TABFeatureClass;

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

    virtual TABFeatureClass GetFeatureClass() { return TABFCBaseFeature; };
    virtual int             GetMapInfoType()  { return m_nMapInfoType; };

    virtual int ReadRecordFromDATFile(TABDATFile *poDATFile);
    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile) = 0;

    void        SetMBR(double dXMin, double dYMin, 
                       double dXMax, double dYMax);
    void        GetMBR(double &dXMin, double &dYMin, 
                       double &dXMax, double &dYMax);

    virtual void DumpMID(FILE *fpOut = NULL);
    virtual void DumpMIF(FILE *fpOut = NULL) = 0;
};


/*---------------------------------------------------------------------
 *                      class TABPoint
 *
 * Feature class to handle the various MapInfo symbol types:
 *
 *     TAB_GEOM_OLDSYMBOL_C    0x01
 *     TAB_GEOM_OLDSYMBOL      0x02
 *     TAB_GEOM_FNTSYMBOL_C    0x28 
 *     TAB_GEOM_FNTSYMBOL      0x29
 *     TAB_GEOM_BMPSYMBOL_C    0x2b
 *     TAB_GEOM_BMPSYMBOL      0x2c
 *
 * Feature geometry will be a OGRPoint
 *--------------------------------------------------------------------*/
class TABPoint: public TABFeature
{
  public:
             TABPoint(OGRFeatureDefn *poDefnIn);
    virtual ~TABPoint();

    virtual TABFeatureClass GetFeatureClass() { return TABFCPoint; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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
class TABPolyline: public TABFeature
{
    GBool       m_bSmooth;
  public:
             TABPolyline(OGRFeatureDefn *poDefnIn);
    virtual ~TABPolyline();

    virtual TABFeatureClass GetFeatureClass() { return TABFCPolyline; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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
class TABRegion: public TABFeature
{
    GBool       m_bSmooth;
  public:
             TABRegion(OGRFeatureDefn *poDefnIn);
    virtual ~TABRegion();

    virtual TABFeatureClass GetFeatureClass() { return TABFCRegion; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
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
 * An ellipse is defined by the coords of its 2 opposite corners (the MBR)
 * Its corners can optionaly be rounded, in which case a X and Y rounding
 * radius will be defined.
 *
 * Feature geometry will be OGRPolygon
 *--------------------------------------------------------------------*/
class TABRectangle: public TABFeature
{
  protected:
    GBool       m_bRoundCorners;
    double      m_dRoundXRadius;
    double      m_dRoundYRadius;

  public:
             TABRectangle(OGRFeatureDefn *poDefnIn);
    virtual ~TABRectangle();

    virtual TABFeatureClass GetFeatureClass() { return TABFCRectangle; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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
class TABEllipse: public TABFeature
{
  protected:

  public:
             TABEllipse(OGRFeatureDefn *poDefnIn);
    virtual ~TABEllipse();

    virtual TABFeatureClass GetFeatureClass() { return TABFCEllipse; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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
class TABArc: public TABFeature
{
  protected:
    double      m_dStartAngle;  // In degrees, counterclockwise, 
    double      m_dEndAngle;    // starting at 3 o'clock

    double      m_dCenterX;
    double      m_dCenterY;
    double      m_dXRadius;
    double      m_dYRadius;

  public:
             TABArc(OGRFeatureDefn *poDefnIn);
    virtual ~TABArc();

    virtual TABFeatureClass GetFeatureClass() { return TABFCEllipse; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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
class TABText: public TABFeature
{
  protected:
    char        *m_pszString;

    double      m_dAngle;
    double      m_dHeight;

    GInt32      m_rgbForeground;
    GInt32      m_rgbBackground;

  public:
             TABText(OGRFeatureDefn *poDefnIn);
    virtual ~TABText();

    virtual TABFeatureClass GetFeatureClass() { return TABFCPoint; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
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

    virtual TABFeatureClass GetFeatureClass() { return TABFCBaseFeature; };

    virtual int ReadGeometryFromMAPFile(TABMAPFile *poMapFile);
    virtual void DumpMIF(FILE *fpOut = NULL);
};


#endif /* _MITAB_H_INCLUDED_ */


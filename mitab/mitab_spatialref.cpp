/**********************************************************************
 * $Id: mitab_spatialref.cpp,v 1.5 1999-09-27 21:23:41 warmerda Exp $
 *
 * Name:     mitab_tabfile.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Implementation of the SpatialRef stuff in the TABFile class.
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
 * $Log: mitab_spatialref.cpp,v $
 * Revision 1.5  1999-09-27 21:23:41  warmerda
 * added more projections
 *
 * Revision 1.4  1999/09/24 04:01:28  warmerda
 * remember nMIDatumId changes
 *
 * Revision 1.3  1999/09/23 19:51:38  warmerda
 * added datum mapping table support
 *
 * Revision 1.2  1999/09/22 23:04:59  daniel
 * Handle reference count on OGRSpatialReference properly
 *
 * Revision 1.1  1999/09/21 19:39:22  daniel
 * Moved Get/SetSpatialRef() to a separate file
 *
 **********************************************************************/

#include "mitab.h"
#include "mitab_utils.h"

typedef struct {
    int		nMapInfoDatumID;
    const char  *pszOGCDatumName;
    double      dfShiftX;
    double	dfShiftY;
    double	dfShiftZ;
    double	dfDatumParm0;
    double	dfDatumParm1;
    double	dfDatumParm2;
    double	dfDatumParm3;
    double	dfDatumParm4;
} MapInfoDatumInfo;

static MapInfoDatumInfo asDatumInfoList[] =
{
{104, "WGS_1984", 0, 0, 0, 0, 0, 0, 0, 0},
{1, "Adindan", -162, -12, 206, 0, 0, 0, 0, 0},
{2, "Afgooye", -43, -163, 45, 0, 0, 0, 0, 0},
{3, "Ain_el_Abd_1970", -150, -251, -2, 0, 0, 0, 0, 0},
{4, "", -491, -22, 435, 0, 0, 0, 0, 0},
{5, "Arc_1950", -143, -90, -294, 0, 0, 0, 0, 0},
{6, "Arc_1960", -160, -8, -300, 0, 0, 0, 0, 0},
{7, "", -207, 107, 52, 0, 0, 0, 0, 0},
{8, "", 145, 75, -272, 0, 0, 0, 0, 0},
{9, "", 114, -116, -333, 0, 0, 0, 0, 0},
{10, "", -320, 550, -494, 0, 0, 0, 0, 0},
{11, "", 124, -234, -25, 0, 0, 0, 0, 0},
{12, "", -133, -48, 148, 0, 0, 0, 0, 0},
{13, "", -134, -48, 149, 0, 0, 0, 0, 0},
{14, "", -127, -769, 472, 0, 0, 0, 0, 0},
{15, "Bermuda_1957", -73, 213, 296, 0, 0, 0, 0, 0},
{16, "Bogota", 307, 304, -318, 0, 0, 0, 0, 0},
{17, "Campo_Inchanspe", -148, 136, 90, 0, 0, 0, 0, 0},
{18, "", 298, -304, -375, 0, 0, 0, 0, 0},
{19, "Cape", -136, -108, -292, 0, 0, 0, 0, 0},
{20, "", -2, 150, 181, 0, 0, 0, 0, 0},
{21, "Carthage", -263, 6, 431, 0, 0, 0, 0, 0},
{22, "", 175, -38, 113, 0, 0, 0, 0, 0},
{23, "Chua", -134, 229, -29, 0, 0, 0, 0, 0},
{24, "Corrego_Alegre", -206, 172, -6, 0, 0, 0, 0, 0},
{25, "Batavia", -377, 681, -50, 0, 0, 0, 0, 0},
{26, "", 230, -199, -752, 0, 0, 0, 0, 0},
{27, "", 211, 147, 111, 0, 0, 0, 0, 0},
{28, "European_Datum_1950", -87, -98, -121, 0, 0, 0, 0, 0},
{29, "European_Datum_1987", -86, -98, -119, 0, 0, 0, 0, 0},
{30, "Gandajika_1970", -133, -321, 50, 0, 0, 0, 0, 0},
{31, "", 84, -22, 209, 0, 0, 0, 0, 0},
{32, "", 0, 0, 0, 0, 0, 0, 0, 0},
{33, "", 0, 0, 0, 0, 0, 0, 0, 0},
{34, "", -100, -248, 259, 0, 0, 0, 0, 0},
{35, "", 252, -209, -751, 0, 0, 0, 0, 0},
{36, "Hito_XVIII_1963", 16, 196, 93, 0, 0, 0, 0, 0},
{37, "", -73, 46, -86, 0, 0, 0, 0, 0},
{38, "", -156, -271, -189, 0, 0, 0, 0, 0},
{39, "Hu_Tzu_Shan", -634, -549, -201, 0, 0, 0, 0, 0},
{40, "", 214, 836, 303, 0, 0, 0, 0, 0},
{41, "", 289, 734, 257, 0, 0, 0, 0, 0},
{42, "", 506, -122, 611, 0, 0, 0, 0, 0},
{43, "", 208, -435, -229, 0, 0, 0, 0, 0},
{44, "", 191, -77, -204, 0, 0, 0, 0, 0},
{45, "Kandawala", -97, 787, 86, 0, 0, 0, 0, 0},
{46, "", 145, -187, 103, 0, 0, 0, 0, 0},
{47, "Kertau", -11, 851, 5, 0, 0, 0, 0, 0},
{48, "", 42, 124, 147, 0, 0, 0, 0, 0},
{49, "Liberia_1964", -90, 40, 88, 0, 0, 0, 0, 0},
{50, "", -133, -77, -51, 0, 0, 0, 0, 0},
{51, "", -133, -79, -72, 0, 0, 0, 0, 0},
{52, "Mahe_1971", 41, -220, -134, 0, 0, 0, 0, 0},
{53, "", -289, -124, 60, 0, 0, 0, 0, 0},
{54, "Massawa", 639, 405, 60, 0, 0, 0, 0, 0},
{55, "", 31, 146, 47, 0, 0, 0, 0, 0},
{56, "", 912, -58, 1227, 0, 0, 0, 0, 0},
{57, "Minna", -92, -93, 122, 0, 0, 0, 0, 0},
{58, "", -247, -148, 369, 0, 0, 0, 0, 0},
{59, "", -249, -156, 381, 0, 0, 0, 0, 0},
{60, "", -231, -196, 482, 0, 0, 0, 0, 0},
{61, "Naparima_1972", -2, 374, 172, 0, 0, 0, 0, 0},
{62, "North_American_Datum_1927", -8, 160, 176, 0, 0, 0, 0, 0},
{63, "North_American_Datum_1927", -5, 135, 172, 0, 0, 0, 0, 0},
{64, "North_American_Datum_1927", -4, 154, 178, 0, 0, 0, 0, 0},
{65, "North_American_Datum_1927", 1, 140, 165, 0, 0, 0, 0, 0},
{66, "North_American_Datum_1927", -10, 158, 187, 0, 0, 0, 0, 0},
{67, "North_American_Datum_1927", 0, 125, 201, 0, 0, 0, 0, 0},
{68, "North_American_Datum_1927", -7, 152, 178, 0, 0, 0, 0, 0},
{69, "North_American_Datum_1927", 0, 125, 194, 0, 0, 0, 0, 0},
{70, "North_American_Datum_1927", -9, 152, 178, 0, 0, 0, 0, 0},
{71, "North_American_Datum_1927", 11, 114, 195, 0, 0, 0, 0, 0},
{72, "North_American_Datum_1927", -12, 130, 190, 0, 0, 0, 0, 0},
{73, "NAD_Michigan", -8, 160, 176, 0, 0, 0, 0, 0},
{74, "North_American_Datum_1983", 0, 0, 0, 0, 0, 0, 0, 0},
{75, "", -425, -169, 81, 0, 0, 0, 0, 0},
{76, "", -130, 110, -13, 0, 0, 0, 0, 0},
{77, "", 61, -285, -181, 0, 0, 0, 0, 0},
{78, "", -346, -1, 224, 0, 0, 0, 0, 0},
{79, "OSGB_1936", 375, -111, 431, 0, 0, 0, 0, 0},
{80, "", -307, -92, 127, 0, 0, 0, 0, 0},
{81, "", 185, 165, 42, 0, 0, 0, 0, 0},
{82, "", -288, 175, -376, 0, 0, 0, 0, 0},
{83, "Provisional_South_Americian_Datum_1956", 11, 72, -101, 0, 0, 0, 0, 0},
{84, "", -128, -283, 22, 0, 0, 0, 0, 0},
{85, "Qornoq", 164, 138, -189, 0, 0, 0, 0, 0},
{86, "", 94, -948, -1262, 0, 0, 0, 0, 0},
{87, "", -225, -65, 9, 0, 0, 0, 0, 0},
{88, "", 170, 42, 84, 0, 0, 0, 0, 0},
{89, "", -203, 141, 53, 0, 0, 0, 0, 0},
{90, "Sapper_Hill_1943", -355, 16, 74, 0, 0, 0, 0, 0},
{91, "Schwarzeck", 616, 97, -251, 0, 0, 0, 0, 0},
{92, "South_American_Datum_1969", -57, 1, -41, 0, 0, 0, 0, 0},
{93, "", 7, -10, -26, 0, 0, 0, 0, 0},
{94, "", -499, -249, 314, 0, 0, 0, 0, 0},
{95, "", -104, 167, -38, 0, 0, 0, 0, 0},
{96, "Timbalai_1948", -689, 691, -46, 0, 0, 0, 0, 0},
{97, "Tokyo", -128, 481, 664, 0, 0, 0, 0, 0},
{98, "", -632, 438, -609, 0, 0, 0, 0, 0},
{99, "", 51, 391, -36, 0, 0, 0, 0, 0},
{100, "", 101, 52, -39, 0, 0, 0, 0, 0},
{101, "", 0, 0, 0, 0, 0, 0, 0, 0},
{102, "", 0, 0, 0, 0, 0, 0, 0, 0},
{103, "WGS_1972", 0, 8, 10, 0, 0, 0, 0, 0},
{105, "Yacare", -155, 171, 37, 0, 0, 0, 0, 0},
{106, "Zanderij", -265, 120, -358, 0, 0, 0, 0, 0},
{107, "Nouvelle_Triangulation_Francaise", -168, -60, 320, 0, 0, 0, 0, 0},
{108, "", -83, -96, -113, 0, 0, 0, 0, 0},
{109, "", 593, 26, 478, 0, 0, 0, 0, 0},
{110, "", 81, 120, 129, 0, 0, 0, 0, 0},
{111, "", -1, 15, 1, 0, 0, 0, 0, 0},
{112, "", 498, -36, 568, 0, 0, 0, 0, 0},
{113, "", -303, -62, 105, 0, 0, 0, 0, 0},
{114, "", -223, 110, 37, 0, 0, 0, 0, 0},
{-1, NULL, 0, 0, 0, 0, 0, 0, 0, 0}
};

 
/**********************************************************************
 *                   TABFile::GetSpatialRef()
 *
 * Returns a reference to an OGRSpatialReference for this dataset.
 * If the projection parameters have not been parsed yet, then we will
 * parse them before returning.
 *
 * The returned object is owned and maintained by this TABFile and
 * should not be modified or freed by the caller.
 *
 * Returns NULL if the SpatialRef cannot be accessed.
 **********************************************************************/
OGRSpatialReference *TABFile::GetSpatialRef()
{
    if (m_eAccessMode != TABRead)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "GetSpatialRef() can be used only with Read access.");
        return NULL;
    }
 
    if (m_poMAPFile == NULL )
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "GetSpatialRef() failed: file has not been opened yet.");
        return NULL;
    }

    /*-----------------------------------------------------------------
     * If projection params have already been processed, just use them.
     *----------------------------------------------------------------*/
    if (m_poSpatialRef != NULL)
        return m_poSpatialRef;
    

    /*-----------------------------------------------------------------
     * Fetch the parameters from the header.
     *----------------------------------------------------------------*/
    TABMAPHeaderBlock *poHeader;
    TABProjInfo     sTABProj;

    if ((poHeader = m_poMAPFile->GetHeaderBlock()) == NULL ||
        poHeader->GetProjInfo( &sTABProj ) != 0)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "GetSpatialRef() failed reading projection parameters.");
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Transform them into an OGRSpatialReference.
     *----------------------------------------------------------------*/
    m_poSpatialRef = new OGRSpatialReference;

    /*-----------------------------------------------------------------
     * Handle the PROJCS style projections, but add the datum later.
     *----------------------------------------------------------------*/
    switch( sTABProj.nProjId )
    {
        /*--------------------------------------------------------------
         * lat/long .. just add the GEOGCS later.
         *-------------------------------------------------------------*/
      case 1:
        break;

        /*--------------------------------------------------------------
         * Cylindrical Equal Area
         *-------------------------------------------------------------*/
      case 2:
        m_poSpatialRef->SetCEA( sTABProj.adProjParams[2],
                                sTABProj.adProjParams[0],
                                sTABProj.adProjParams[4],
                                sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Lambert Conic Conformal
         *-------------------------------------------------------------*/
      case 3:
        m_poSpatialRef->SetLCC( sTABProj.adProjParams[2],
                                sTABProj.adProjParams[3],
                                sTABProj.adProjParams[1],
                                sTABProj.adProjParams[0],
                                sTABProj.adProjParams[4],
                                sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Lambert Azimuthal Equal Area
         *-------------------------------------------------------------*/
      case 4:
        m_poSpatialRef->SetLAEA( sTABProj.adProjParams[1],
                                 sTABProj.adProjParams[0],
                                 0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Azimuthal Equidistant (Polar aspect only)
         *-------------------------------------------------------------*/
      case 5:
        m_poSpatialRef->SetAE( sTABProj.adProjParams[1],
                               sTABProj.adProjParams[0],
                               0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Equidistant Conic
         *-------------------------------------------------------------*/
      case 6:
        m_poSpatialRef->SetEC( sTABProj.adProjParams[2],
                               sTABProj.adProjParams[3],
                               sTABProj.adProjParams[1],
                               sTABProj.adProjParams[0],
                               sTABProj.adProjParams[4],
                               sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Eckert IV
         *-------------------------------------------------------------*/
      case 14:
        m_poSpatialRef->SetEckertIV( sTABProj.adProjParams[0], 0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Eckert VI
         *-------------------------------------------------------------*/
      case 15:
        m_poSpatialRef->SetEckertVI( sTABProj.adProjParams[0], 0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Hotine Oblique Mercator
         *-------------------------------------------------------------*/
      case 7:
        m_poSpatialRef->SetHOM( sTABProj.adProjParams[1],
                                sTABProj.adProjParams[0], 
                                sTABProj.adProjParams[2],
                                90.0, 
                                sTABProj.adProjParams[3],
                                sTABProj.adProjParams[4],
                                sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Albers Conic Equal Area
         *-------------------------------------------------------------*/
      case 9:
        m_poSpatialRef->SetACEA( sTABProj.adProjParams[2],
                                 sTABProj.adProjParams[3],
                                 sTABProj.adProjParams[1],
                                 sTABProj.adProjParams[0],
                                 sTABProj.adProjParams[4],
                                 sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Mercator
         *-------------------------------------------------------------*/
      case 10:
        m_poSpatialRef->SetMercator( 0.0, sTABProj.adProjParams[0],
                                     1.0, 0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Miller Cylindrical
         *-------------------------------------------------------------*/
      case 11:
        m_poSpatialRef->SetMC( 0.0, sTABProj.adProjParams[0],
                               0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Robinson
         *-------------------------------------------------------------*/
      case 12:
        m_poSpatialRef->SetRobinson( sTABProj.adProjParams[0],
                                     0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Mollweide
         *-------------------------------------------------------------*/
      case 13:
        m_poSpatialRef->SetMollweide( sTABProj.adProjParams[0],
                                      0.0, 0.0 );

        /*--------------------------------------------------------------
         * Sinusoidal
         *-------------------------------------------------------------*/
      case 16:
        m_poSpatialRef->SetSinusoidal( sTABProj.adProjParams[0],
                                       0.0, 0.0 );
        break;

        /*--------------------------------------------------------------
         * Transverse Mercator
         *-------------------------------------------------------------*/
      case 8:
      case 21:
      case 22:
      case 23:
      case 24:
        m_poSpatialRef->SetTM( sTABProj.adProjParams[1],
                               sTABProj.adProjParams[0],
                               sTABProj.adProjParams[2],
                               sTABProj.adProjParams[3],
                               sTABProj.adProjParams[4] );
        break;

        /*--------------------------------------------------------------
         * Gall
         *-------------------------------------------------------------*/
      case 17:
        m_poSpatialRef->SetGS( sTABProj.adProjParams[0], 0.0, 0.0 );
        break;
        
        /*--------------------------------------------------------------
         * New Zealand Map Grid
         *-------------------------------------------------------------*/
      case 18:
        m_poSpatialRef->SetNZMG( sTABProj.adProjParams[1],
                                 sTABProj.adProjParams[0],
                                 sTABProj.adProjParams[2],
                                 sTABProj.adProjParams[3] );
        break;

        /*--------------------------------------------------------------
         * Lambert Conic Conformal (Belgium)
         *-------------------------------------------------------------*/
      case 19:
        m_poSpatialRef->SetLCCB( sTABProj.adProjParams[2],
                                 sTABProj.adProjParams[3],
                                 sTABProj.adProjParams[1],
                                 sTABProj.adProjParams[0],
                                 sTABProj.adProjParams[4],
                                 sTABProj.adProjParams[5] );
        break;

        /*--------------------------------------------------------------
         * Stereographic
         *-------------------------------------------------------------*/
      case 20:
        m_poSpatialRef->SetStereographic( 0.0, sTABProj.adProjParams[0], 
                                          1.0,
                                          sTABProj.adProjParams[1],
                                          sTABProj.adProjParams[2] );
        break;

      default:
        break;
    }

    /*-----------------------------------------------------------------
     * Collect units definition.
     *----------------------------------------------------------------*/
    if( sTABProj.nProjId != 1 && m_poSpatialRef->GetRoot() != NULL )
    {
        OGR_SRSNode	*poUnits = new OGR_SRSNode("UNIT");
        
        m_poSpatialRef->GetRoot()->AddChild(poUnits);

        poUnits->AddChild( new OGR_SRSNode( SRS_UL_METER ) );
        poUnits->AddChild( new OGR_SRSNode( "1.0" ) );
       
        switch( sTABProj.nUnitsId )
        {
          case 1:
            poUnits->GetChild(0)->SetValue("Kilometer");
            poUnits->GetChild(1)->SetValue("1000.0");
            break;
            
          case 2:
            poUnits->GetChild(0)->SetValue("Inch");
            poUnits->GetChild(1)->SetValue("0.0254");
            break;
            
          case 3:
            poUnits->GetChild(0)->SetValue(SRS_UL_FOOT);
            poUnits->GetChild(1)->SetValue(SRS_UL_FOOT_CONV);
            break;
            
          case 4:
            poUnits->GetChild(0)->SetValue("Yard");
            poUnits->GetChild(1)->SetValue("0.9144");
            break;
            
          case 5:
            poUnits->GetChild(0)->SetValue("Millimeter");
            poUnits->GetChild(1)->SetValue("0.001");
            break;
            
          case 6:
            poUnits->GetChild(0)->SetValue("Centimeter");
            poUnits->GetChild(1)->SetValue("0.01");
            break;
            
          case 7:
            poUnits->GetChild(0)->SetValue(SRS_UL_METER);
            poUnits->GetChild(1)->SetValue("1.0");
            break;
            
          case 8:
            poUnits->GetChild(0)->SetValue(SRS_UL_US_FOOT);
            poUnits->GetChild(1)->SetValue(SRS_UL_US_FOOT_CONV);
            break;
            
          case 9:
            poUnits->GetChild(0)->SetValue(SRS_UL_NAUTICAL_MILE);
            poUnits->GetChild(1)->SetValue(SRS_UL_NAUTICAL_MILE_CONV);
            break;
            
          case 30:
            poUnits->GetChild(0)->SetValue(SRS_UL_LINK);
            poUnits->GetChild(1)->SetValue(SRS_UL_LINK_CONV);
            break;
            
          case 31:
            poUnits->GetChild(0)->SetValue(SRS_UL_CHAIN);
            poUnits->GetChild(1)->SetValue(SRS_UL_CHAIN_CONV);
            break;
            
          case 32:
            poUnits->GetChild(0)->SetValue(SRS_UL_ROD);
            poUnits->GetChild(1)->SetValue(SRS_UL_ROD_CONV);
            break;
            
          default:
            break;
        }
    }

    /*-----------------------------------------------------------------
     * Create a GEOGCS definition.
     *----------------------------------------------------------------*/
    OGR_SRSNode	*poGCS, *poDatum, *poSpheroid, *poPM;
    char	szDatumName[128];

    poGCS = new OGR_SRSNode("GEOGCS");

    if( m_poSpatialRef->GetRoot() == NULL )
        m_poSpatialRef->SetRoot( poGCS );
    else
        m_poSpatialRef->GetRoot()->AddChild( poGCS );

    poGCS->AddChild( new OGR_SRSNode("unnamed") );

    /*-----------------------------------------------------------------
     * Set the datum.  We are only given the X, Y and Z shift for
     * the datum, so for now we just synthesize a name from this.
     * It would be better if we could lookup a name based on the shift.
     *----------------------------------------------------------------*/
    int		iDatumInfo;
    MapInfoDatumInfo *psDatumInfo;

    for( iDatumInfo = 0;
         asDatumInfoList[iDatumInfo].nMapInfoDatumID != -1;
         iDatumInfo++ )
    {
        psDatumInfo = asDatumInfoList + iDatumInfo;
        
        if( psDatumInfo->dfShiftX == sTABProj.dDatumShiftX
            && psDatumInfo->dfShiftY == sTABProj.dDatumShiftY
            && psDatumInfo->dfShiftZ == sTABProj.dDatumShiftZ
            && psDatumInfo->dfDatumParm0 == sTABProj.adDatumParams[0]
            && psDatumInfo->dfDatumParm1 == sTABProj.adDatumParams[1]
            && psDatumInfo->dfDatumParm2 == sTABProj.adDatumParams[2]
            && psDatumInfo->dfDatumParm3 == sTABProj.adDatumParams[3]
            && psDatumInfo->dfDatumParm4 == sTABProj.adDatumParams[4] )
            break;

        psDatumInfo = NULL;
    }

    poGCS->AddChild( (poDatum = new OGR_SRSNode("DATUM")) );

    if( psDatumInfo == NULL )
    {
        sprintf( szDatumName, "(%.4f,%.4f,%.4f)",
                 sTABProj.dDatumShiftX, 
                 sTABProj.dDatumShiftY, 
                 sTABProj.dDatumShiftZ );
        
        poDatum->AddChild( new OGR_SRSNode(szDatumName) );

        sTABProj.nMIDatumId = 999;
        poHeader->SetProjInfo( &sTABProj );
    }
    else if( strlen(psDatumInfo->pszOGCDatumName) > 0 )
    {
        poDatum->AddChild( new OGR_SRSNode(psDatumInfo->pszOGCDatumName) );
        sTABProj.nMIDatumId = psDatumInfo->nMapInfoDatumID;
        poHeader->SetProjInfo( &sTABProj );
    }
    else
    {
        sprintf( szDatumName, "MapInfo %d", psDatumInfo->nMapInfoDatumID );
        
        poDatum->AddChild( new OGR_SRSNode(szDatumName) );
        sTABProj.nMIDatumId = psDatumInfo->nMapInfoDatumID;
        poHeader->SetProjInfo( &sTABProj );
    }

    /*-----------------------------------------------------------------
     * Set the spheroid.
     *----------------------------------------------------------------*/
    poDatum->AddChild( (poSpheroid = new OGR_SRSNode("SPHEROID")) );

    poSpheroid->AddChild( new OGR_SRSNode( "GRS_1980" ) );
    poSpheroid->AddChild( new OGR_SRSNode( "6378137" ) );
    poSpheroid->AddChild( new OGR_SRSNode( "298.257222101" ) );

    /* 
    switch( sTABProj.nEllipsoidId )
    {
    }
    */

    /*-----------------------------------------------------------------
     * It seems that the prime meridian is always Greenwich for Mapinfo
     *----------------------------------------------------------------*/
    
    poDatum->AddChild( (poPM = new OGR_SRSNode("PRIMEM")) );

    poPM->AddChild( new OGR_SRSNode("Greenwich") );
    poPM->AddChild( new OGR_SRSNode("0") );
                    
    /*-----------------------------------------------------------------
     * GeogCS is always in degrees.
     *----------------------------------------------------------------*/
    OGR_SRSNode	*poUnit;

    poDatum->AddChild( (poUnit = new OGR_SRSNode("UNIT")) );

    poUnit->AddChild( new OGR_SRSNode(SRS_UA_DEGREE) );
    poUnit->AddChild( new OGR_SRSNode(SRS_UA_DEGREE_CONV) );

    return m_poSpatialRef;
}

/**********************************************************************
 *                   TABFile::SetSpatialRef()
 *
 * Set the OGRSpatialReference for this dataset.
 * A reference to the OGRSpatialReference will be kept, and it will also
 * be converted into a TABProjInfo to be stored in the .MAP header.
 *
 * Returns 0 on success, and -1 on error.
 **********************************************************************/
int TABFile::SetSpatialRef(OGRSpatialReference *poSpatialRef)
{
    if (m_eAccessMode != TABWrite)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "SetSpatialRef() can be used only with Write access.");
        return -1;
    }

    if (m_poMAPFile == NULL )
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "SetSpatialRef() failed: file has not been opened yet.");
        return -1;
    }

    /*-----------------------------------------------------------------
     * Keep a copy of the OGRSpatialReference...
     * Note: we have to take the reference count into account...
     *----------------------------------------------------------------*/
    if (m_poSpatialRef && m_poSpatialRef->Dereference() == 0)
        delete m_poSpatialRef;
    
    m_poSpatialRef = poSpatialRef;
    m_poSpatialRef->Reference();

    /*-----------------------------------------------------------------
     * ... and transform it into a TABProjInfo
     *----------------------------------------------------------------*/
    TABProjInfo     sTABProj;

    // __TODO__ 


    /*-----------------------------------------------------------------
     * Set the new parameters in the .MAP header.
     *----------------------------------------------------------------*/
    TABMAPHeaderBlock *poHeader;

    if ((poHeader = m_poMAPFile->GetHeaderBlock()) == NULL ||
        poHeader->SetProjInfo( &sTABProj ) != 0)
    {
        CPLError(CE_Failure, CPLE_FileIO,
                 "SetSpatialRef() failed setting projection parameters.");
        return -1;
    }

    return 0;
}


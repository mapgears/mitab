/**********************************************************************
 * $Id: mitab_spatialref.cpp,v 1.1 1999-09-21 19:39:22 daniel Exp $
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
 * Revision 1.1  1999-09-21 19:39:22  daniel
 * Moved Get/SetSpatialRef() to a separate file
 *
 **********************************************************************/

#include "mitab.h"
#include "mitab_utils.h"


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

      /*--------------------------------------------------------------
       * Cylindrical Equal Area
       *-------------------------------------------------------------*/
      case 2:

      /*--------------------------------------------------------------
       * Mollweide
       *-------------------------------------------------------------*/
      case 13:

      /*--------------------------------------------------------------
       * Eckert IV
       *-------------------------------------------------------------*/
      case 14:

      /*--------------------------------------------------------------
       * Eckert VI
       *-------------------------------------------------------------*/
      case 15:

      /*--------------------------------------------------------------
       * Gall
       *-------------------------------------------------------------*/
      case 17:

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
    poGCS->AddChild( (poDatum = new OGR_SRSNode("DATUM")) );

    sprintf( szDatumName, "MapInfo (%.4f,%.4f,%.4f)",
             sTABProj.dDatumShiftX, 
             sTABProj.dDatumShiftY, 
             sTABProj.dDatumShiftZ );
             
    poDatum->AddChild( new OGR_SRSNode(szDatumName) );

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
 *                   TABFile::GetSpatialRef()
 *
 * Set the OGRSpatialReference for this dataset.
 * A copy of the OGRSpatialReference will be kept, and it will also
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
     *----------------------------------------------------------------*/
    if (m_poSpatialRef == NULL)
        m_poSpatialRef = new OGRSpatialReference;
    *m_poSpatialRef = *poSpatialRef;

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

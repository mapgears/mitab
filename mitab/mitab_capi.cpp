/**********************************************************************
 * $Id: mitab_capi.cpp,v 1.1 2000-01-14 14:53:59 warmerda Exp $
 *
 * Name:     mitab_capi.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Simple C access API.  Suitable for limited access
 *           to MITAB from an application able to call C style DLL
 *           entry points.
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 **********************************************************************
 * Copyright (c) 2000, Daniel Morissette
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
 * $Log: mitab_capi.cpp,v $
 * Revision 1.1  2000-01-14 14:53:59  warmerda
 * New
 *
 */

#include "mitab.h"
#include "mitab_capi.h"

/************************************************************************/
/*                            mitab_c_open()                            */
/*                                                                      */
/*      Open an existing tab file for read access.                      */
/************************************************************************/

mitab_handle mitab_c_open( const char * pszFilename )

{
    return (mitab_handle) IMapInfoFile::SmartOpen( pszFilename );
}

/************************************************************************/
/*                           mitab_c_close()                            */
/************************************************************************/

void mitab_c_close( mitab_handle handle )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

    poFile->Close();

    delete poFile;	
}

/************************************************************************/
/*                           mitab_c_create()                           */
/*                                                                      */
/*      Create a new TAB file.                                          */
/************************************************************************/

mitab_handle mitab_c_create( const char * filename,
                             const char * mif_or_tab,
                             const char * mif_projection,
                             double north, double south,
                             double east, double west )

{
    IMapInfoFile	*poFile;
    
    if( mif_or_tab == NULL || !EQUAL(mif_or_tab,"mif") )
    {
        poFile = new TABFile;
    }
    else
    {
        poFile = new MIFFile;
    }

    if( poFile->Open( filename, "wb" ) != 0 )
    {
        delete poFile;
        return NULL;
    }

    poFile->SetBounds( west, south, east, north );

    if( mif_projection != NULL && strlen(mif_projection) > 0 )
    {
        OGRSpatialReference	*poSRS;

        poSRS = MITABCoordSys2SpatialRef( mif_projection );
        poFile->SetSpatialRef( poSRS );
        delete poSRS;
    }

    return (mitab_handle) poFile;
}

/************************************************************************/
/*                      mitab_c_destroy_feature()                       */
/************************************************************************/

void mitab_c_destroy_feature( mitab_feature feature )

{
    TABFeature	*poFeature = (TABFeature *) feature;

    delete poFeature;
}

/************************************************************************/
/*                      mitab_c_next_feature_id()                       */
/*                                                                      */
/*      Iterator to get the next feature id.  Use -1 as previous to     */
/*      get first feature id.  Returns -1 when there are no more        */
/*      feature ids.                                                    */
/************************************************************************/

int mitab_c_next_feature_id( mitab_handle handle, int last_feature_id )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

    return poFile->GetNextFeatureId( last_feature_id );
}

/************************************************************************/
/*                        mitab_c_read_feature()                        */
/************************************************************************/

mitab_feature mitab_c_read_feature( mitab_handle handle, int feature_id )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
    TABFeature		*poFeature;

    poFeature = poFile->GetFeatureRef( feature_id );
    if( poFeature != NULL )
        poFeature = poFeature->CloneTABFeature();

    return (mitab_feature) poFeature;
}

/************************************************************************/
/*                       mitab_c_write_feature()                        */
/************************************************************************/

int mitab_c_write_feature( mitab_handle handle, mitab_feature feature )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
    TABFeature		*poFeature = (TABFeature *) feature;

    return poFile->SetFeature( poFeature ) != -1;
}

/************************************************************************/
/*                       mitab_c_create_feature()                       */
/************************************************************************/

mitab_feature mitab_c_create_feature( mitab_handle handle,
                                      int feature_type )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
    TABFeature	*poFeature = NULL;

    if( poFile->GetLayerDefn() == NULL )
    {
        poFile->AddFieldNative( "NDX", TABFInteger, 10, 0 );
    }

    if( feature_type == TABFC_Point )
        poFeature = new TABPoint(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Text )
        poFeature = new TABText(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Polyline )
        poFeature = new TABPolyline(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Region )
        poFeature = new TABRegion(poFile->GetLayerDefn());

    return poFeature;
}

/************************************************************************/
/*                         mitab_c_set_points()                         */
/************************************************************************/

void mitab_c_set_points( mitab_feature feature, int part,
                         int vertex_count, double * x, double * y )

{
    TABFeature	*poFeature = (TABFeature *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Point
        || poFeature->GetFeatureClass() == TABFC_Text )
    {
        CPLAssert( vertex_count == 1 );
        poFeature->SetGeometryDirectly( new OGRPoint( x[0], y[0] ) );
    }

    else if( poFeature->GetFeatureClass() == TABFC_Polyline )
    {
        OGRLineString	*poLine = new OGRLineString();

        CPLAssert( part == 0 );
        poLine->setPoints( vertex_count, x, y );

        poFeature->SetGeometryDirectly( poLine );
    }

    else if( poFeature->GetFeatureClass() == TABFC_Region )
    {
        OGRLinearRing	*poRing = new OGRLinearRing();
        OGRPolygon	*poPolygon;

        poRing->setPoints( vertex_count, x, y );
        if( poFeature->GetGeometryRef() != NULL && part > 0 )
        {

            poPolygon = (OGRPolygon *) poFeature->GetGeometryRef();
            CPLAssert( part == poPolygon->getNumInteriorRings() + 1 );
            poPolygon->addRingDirectly( poRing );
        }
        else
        {
            CPLAssert( part == 0 );
            
            poPolygon = new OGRPolygon;
            poPolygon->addRingDirectly( poRing );
            poFeature->SetGeometryDirectly( poPolygon );
        }
    }
}

/**********************************************************************
 * $Id: mitab_capi.cpp,v 1.22 2002-05-08 21:37:40 daniel Exp $
 *
 * Name:     mitab_capi.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Simple C access API.  Suitable for limited access
 *           to MITAB from an application able to call C style DLL
 *           entry points.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 **********************************************************************
 * Copyright (c) 2000-2002, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************
 *
 * $Log: mitab_capi.cpp,v $
 * Revision 1.22  2002-05-08 21:37:40  daniel
 * Added FontPoint and CustomPoint in mitab_c_create_feature() and
 * mitab_c_set_points().  And (hopefully) fixed the STDCALL stuff.
 *
 * Revision 1.21  2002/05/08 20:02:03  daniel
 * Made mitab_c_set_font() and mitab_c_get_font() work for TABFC_FontPoint
 * and TABFC_CustomPoint.
 *
 * Revision 1.20  2002/05/03 15:09:14  daniel
 * Added mitab_c_get_field_width() and mitab_c_get_field_precision()
 *
 * Revision 1.19  2002/04/26 14:16:49  julien
 * Finishing the implementation of Multipoint (support for MIF)
 *
 * Revision 1.18  2002/03/26 19:27:43  daniel
 * Got rid of tabs in source
 *
 * Revision 1.17  2002/02/22 14:10:33  daniel
 * Restored mitab_c_set_arc() fix from v1.15 which had been overwritten.
 * Added a note about VB, Pascal, MapBasic in Doxygen docs introduction.
 *
 * Revision 1.16  2002/02/22 13:50:28  daniel
 * (From Bo Thomsen) New VB interface functions
 *
 * Revision 1.16  2002/02/20 12:35:00  bvt
 * Added alternative functions to fetch various strings for VB compatibility.
 * Functions has suffix _vb.
 *
 * Revision 1.15  2001/12/17 16:05:19  warmerda
 * set point geometry in mitab_c_set_arc() so validate will work
 *
 * Revision 1.14  2001/11/02 17:30:02  daniel
 * Added mitab_c_get/set_projinfo() and mitab_c_get_mif_coordsys().
 * Changed mitab_c_create() to make bounds optional and allow using default
 * projection bounds if available.
 *
 * Revision 1.13  2001/08/10 19:32:38  warmerda
 * check reference count before deleting spatial ref
 *
 * Revision 1.12  2001/07/02 20:03:28  daniel
 * Added mitab_c_get_text().
 *
 * Revision 1.11  2001/06/25 01:49:47  daniel
 * Added get methods for all pen, brush, font and symbol properties.
 *
 * Revision 1.10  2001/01/22 16:03:58  warmerda
 * expanded tabs
 *
 * Revision 1.9  2000/10/16 21:44:50  warmerda
 * added nonearth support
 *
 * Revision 1.8  2000/10/03 20:43:36  daniel
 * Added support for writing arcs,ellipses and rectangles in C API
 *
 * Revision 1.7  2000/04/21 13:37:59  daniel
 * Added doxygen file definition block
 *
 * Revision 1.6  2000/04/21 12:53:07  daniel
 * Added funcs to fetch feature coordinates and attributes + written docs
 *
 * Revision 1.5  2000/02/28 16:46:37  daniel
 * Removed style param to mitab_c_set_pen() since this param is actually
 * used inside the format to define pen width in points (version 450)
 *
 * Revision 1.4  2000/01/15 22:30:43  daniel
 * Switch to MIT/X-Consortium OpenSource license
 *
 * Revision 1.3  2000/01/14 21:58:40  warmerda
 * added error handling
 *
 * Revision 1.2  2000/01/14 16:33:24  warmerda
 * initial implementation complete
 *
 * Revision 1.1  2000/01/14 14:53:59  warmerda
 * New
 *
 */


/** \file mitab_capi.cpp
 *
 *  MITAB C API.
 *
 * The C wrapper functions contained in mitab_capi.cpp provide a much simpler
 * interface than the OGR-based C++ classes to the MITAB library to read and
 * write .TAB and .MIF files.
 *
 * To use the C API, simply #include "mitab_capi.h" and use the functions 
 * defined in mitab_capi.cpp.  
 *
 * See also mitabc_test.cpp which for examples of use of the C API.
 *
 * See contrib/README_VB.TXT in the MITAB source code distribution for
 * informations on using the MITAB C API from VB, Pascal, or MapBasic.
 *
 */

#include "mitab.h"
#include "mitab_capi.h"

static int _mitab_c_get_feature_info( mitab_feature feature, int what_info, 
                                      int *part, int *point, double *vertex );
#define INFO_NUMPARTS  0
#define INFO_NUMPOINTS 1
#define INFO_XVERTEX   2
#define INFO_YVERTEX   3


/* ==================================================================== */
/*                   Error handling functions                           */
/* ==================================================================== */

/************************************************************************/
/*                       mitab_c_getlasterrorno()                       */
/************************************************************************/

/**
 * Fetch the last error number.
 *
 * This is the error number, not the error class.
 *
 * @return the error number of the last error to occur, or CPLE_None (0)
 * if there are no posted errors.
 */

int MITAB_STDCALL
mitab_c_getlasterrorno()

{
    return CPLGetLastErrorNo();
}

/************************************************************************/
/*                      mitab_c_getlasterrormsg()                       */
/************************************************************************/

/**
 * Get the last error message.
 *
 * Fetches the last error message posted with CPLError(), that hasn't
 * been cleared by CPLErrorReset().  The returned pointer is to an internal
 * string that should not be altered or freed.
 *
 * @return the last error message, or an empty string if there is no posted
 *         error message.
 */

const char * MITAB_STDCALL
mitab_c_getlasterrormsg()

{
    const char      *pszLastMessage = CPLGetLastErrorMsg();

    if( pszLastMessage == NULL )
        return "";
    else
        return pszLastMessage;
}

/************************************************************************/
/*                      mitab_c_getlasterrormsg_vb()                    */
/************************************************************************/

/**
 * Get the last error message (VB Version).
 *
 * Fetches the last error message posted with CPLError(), that hasn't
 * been cleared by CPLErrorReset().  The returned pointer is to an internal
 * string that should not be altered or freed.
 *
 * @param errormsg string buffer to return the last error message (will
 *         return an empty string if there is no posted error message).
 * @param l the maximum length of the errormessage string including 
 *         terminating null.
 * @return the length of the last error message, or zero if there is no posted
 *         error message.
 */

int MITAB_STDCALL 
mitab_c_getlasterrormsg_vb (char * errormsg, int l)

{
    strncpy (errormsg,CPLGetLastErrorMsg(),l); 
    return strlen(errormsg);
}


/* ==================================================================== */
/*                       C API Functions                                */
/* ==================================================================== */

/************************************************************************/
/*                            mitab_c_open()                            */
/************************************************************************/

/**
 * Open an existing .TAB or .MIF dataset for read access.
 *
 * The function automatically detects the format (.MIF or .TAB) of the 
 * specified file.
 *
 * Note that it is not possible to open a file for update (i.e. read+write)
 * with the current version of the library.
 *
 * @param pszFilename the complete filename (including extension .TAB or .MIF)
 *        of the file to open.
 * @return a valid mitab_handle, or NULL if the file could not be opened.
 */

mitab_handle MITAB_STDCALL
mitab_c_open( const char * pszFilename )

{
    CPLSetErrorHandler( CPLQuietErrorHandler );
    return (mitab_handle) IMapInfoFile::SmartOpen( pszFilename );
}

/************************************************************************/
/*                           mitab_c_close()                            */
/************************************************************************/

/**
 * Close a dataset previously opened using mitab_c_open() or created using
 * mitab_c_create().
 *
 * @param handle the mitab_handle of the dataset to close.
 */

void MITAB_STDCALL
mitab_c_close( mitab_handle handle )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;

    poFile->Close();

    delete poFile;      
}

/************************************************************************/
/*                           mitab_c_create()                           */
/************************************************************************/

/**
 * Create a new .TAB or .MIF dataset.
 *
 * Note that it is not possible to open a file for update (i.e. read+write)
 * with the current version of the library.
 *
 * @param filename the complete filename (including extension .TAB or .MIF)
 *        of the file to create.
 * @param mif_or_tab one of "mif" to create a .MIF dataset or "tab" to create
 *        a .TAB dataset.  The default is to create a TAB dataset if this
 *        parameter's value is NULL or an empty string.
 * @param mif_projection the projection to use for the dataset, in the same
 *        format that is used in the "CoordSys" line of a MIF file header.
 *        If this parameter's value is NULL or empty then a LAT/LON coordsys
 *        is assumed.  See also mitab_c_get_mif_coordsys().
 * @param north the upper dataset bound.
 *        Note that valid bounds are required for a .TAB dataset otherwise
 *        data may not be stored properly in the file.  
 *        MITAB knows the default bounds only for the most common MapInfo 
 *        coordinate systems, passing north,south,east,west as 0,0,0,0 will
 *        instruct MITAB to attempt to use the default bounds for that 
 *        projection.  If no default bounds are found for this projection then
 *        your data may not be stored properly in the file unless you provide
 *        valid bounds via the north,south,east,west parameters.
 * @param south the lower dataset bound.
 * @param east the right dataset bound.
 * @param west the left dataset bound.
 * @return a valid mitab_handle, or NULL if the file could not be created.
 */

mitab_handle MITAB_STDCALL
mitab_c_create( const char * filename,
                const char * mif_or_tab,
                const char * mif_projection,
                double north, double south,
                double east, double west )

{
    IMapInfoFile        *poFile;
    
    CPLSetErrorHandler( CPLQuietErrorHandler );

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

    if( mif_projection != NULL && strlen(mif_projection) > 0 )
    {
        poFile->SetMIFCoordSys( mif_projection );
    }

    if (north != 0 || south != 0 || east != 0 || west != 0)
        poFile->SetBounds( west, south, east, north );

    return (mitab_handle) poFile;
}

/************************************************************************/
/*                         mitab_c_add_field()                          */
/*                                                                      */
/*      Add a new field to the schema.  Return the field id.            */
/************************************************************************/

/**
 * Add a new field to the dataset's schema.
 *
 * Adding fields MUST be done immediately after creating a new dataset and
 * BEFORE creating the first feature.  
 *
 * @param dataset the mitab_handle of the newly created dataset.
 * @param field_name the name of the new field to create.
 * @param field_type the new field type, one of TABFT_Char (1), 
 *        TABFT_Integer (2), TABFT_SmallInt (3), TABFT_Decimal (4), 
 *        TABFT_Float (5), TABFT_Date (6), or TABFT_Logical (7)
 * @param width the width of the new field, applies only to char and decimal
 *        types.
 * @param precision the number of digits after the decimal point, applies only
 *        to the decimal field type.
 * @return the 0-based index of the new field, or -1 if the field could not
 *         be created.
 */

int MITAB_STDCALL
mitab_c_add_field( mitab_handle dataset, const char *field_name,
                   int field_type, int width, int precision )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) dataset;

    if( poFile->AddFieldNative( field_name, (TABFieldType) field_type,
                                width, precision ) != -1 )
    {
        return poFile->GetLayerDefn()->GetFieldCount() - 1;
    }
    else
        return -1;
}

/************************************************************************/
/*                      mitab_c_destroy_feature()                       */
/************************************************************************/

/**
 * Destroy a mitab_feature object and release all memory associated with it.
 *
 * @param feature the mitab_feature to destroy.
 */

void MITAB_STDCALL
mitab_c_destroy_feature( mitab_feature feature )

{
    TABFeature  *poFeature = (TABFeature *) feature;

    if (poFeature)
        delete poFeature;
}

/************************************************************************/
/*                      mitab_c_next_feature_id()                       */
/*                                                                      */
/*      Iterator to get the next feature id.  Use -1 as previous to     */
/*      get first feature id.  Returns -1 when there are no more        */
/*      feature ids.                                                    */
/************************************************************************/

/**
 * Iterator to get the next valid feature id when reading a dataset opened
 * with mitab_c_open().
 * 
 * @param handle the mitab_handle of the file opened for read access.
 * @param last_feature_id the id of the last feature that was read.
 *        Use -1 to get first feature id in the dataset.
 * @return the next valid feature id in the dataset, or -1 when there are
 *         no more feature ids.
 */

int MITAB_STDCALL
mitab_c_next_feature_id( mitab_handle handle, int last_feature_id )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;

    return poFile->GetNextFeatureId( last_feature_id );
}

/************************************************************************/
/*                        mitab_c_read_feature()                        */
/************************************************************************/

/**
 * Read a mitab_feature object from the file.  
 *
 * Works only with datasets opened with mitab_c_open().
 *
 * @param handle the mitab_handle of the file opened for read access.
 * @param feature_id the id of the feature to read, obtained by calling
 *        mitab_c_next_feature_id().
 * @return the mitab_feature object that was read.  The object will have to
 *         be destroyed using mitab_c_destroy_feature() once you are done 
 *         with it.
 */

mitab_feature MITAB_STDCALL
mitab_c_read_feature( mitab_handle handle, int feature_id )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    TABFeature          *poFeature;

    poFeature = poFile->GetFeatureRef( feature_id );
    if( poFeature != NULL )
        poFeature = poFeature->CloneTABFeature();

    return (mitab_feature) poFeature;
}

/************************************************************************/
/*                       mitab_c_write_feature()                        */
/************************************************************************/

/**
 * Write a new feature to a dataset.
 *
 * Works only with datasets created using mitab_c_create().
 *
 * The feature to write should have been created using 
 * mitab_c_create_feature() and its attributes and coordinate information 
 * set prior to calling this function.  
 *
 * Also note that the mitab_feature object will NOT be owned by the dataset
 * after this call and it is still the responsibility of the caller
 * to destroy it.
 *
 * @param handle the mitab_handle of the dataset opened for write access.
 * @param feature the mitab_feature to write to the dataset. 
 * @return TRUE (1) on success, FALSE (0) on error.
 */

int MITAB_STDCALL
mitab_c_write_feature( mitab_handle handle, mitab_feature feature )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    TABFeature          *poFeature = (TABFeature *) feature;

    return poFile->SetFeature( poFeature ) != -1;
}

/************************************************************************/
/*                       mitab_c_create_feature()                       */
/************************************************************************/

/**
 * Create a new mitab_feature object to be written to a dataset created
 * using mitab_c_create().
 *
 * @param handle the handle of the dataset opened for write access.
 * @param feature_type the type of feature object to create.  At this point,
 *        only the following types can be created by this C API function:
 *        TABFC_Point (1), TABFC_FontPoint (2), TABFC_CustomPoint (3),
 *        TABFC_Text (4), TABFC_Polyline (5), TABFC_Arc (6), 
 *        TABFC_Region (7), TABFC_Rectangle (8), TABFC_Ellipse (9) and
 *        TABFC_MultiPoint (10)
 * @return the new mitab_feature object, or NULL if creation failed.  Note that
 *         the new object will have to be released using 
 *         mitab_c_destroy_feature().
 */

mitab_feature MITAB_STDCALL
mitab_c_create_feature( mitab_handle handle,
                        int feature_type )

{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    TABFeature  *poFeature = NULL;

    if( poFile->GetLayerDefn() == NULL )
    {
        poFile->AddFieldNative( "NDX", TABFInteger, 10, 0 );
    }

    if( feature_type == TABFC_Point )
        poFeature = new TABPoint(poFile->GetLayerDefn());
    else if( feature_type == TABFC_FontPoint )
        poFeature = new TABFontPoint(poFile->GetLayerDefn());
    else if( feature_type == TABFC_CustomPoint )
        poFeature = new TABCustomPoint(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Text )
    {
        TABText         *poText = new TABText(poFile->GetLayerDefn());

        poText->SetTextString( "Default Text" );
        poFeature = poText;
    }
    else if( feature_type == TABFC_Polyline )
        poFeature = new TABPolyline(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Arc )
        poFeature = new TABArc(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Region )
        poFeature = new TABRegion(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Rectangle )
        poFeature = new TABRectangle(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Ellipse )
        poFeature = new TABEllipse(poFile->GetLayerDefn());
    else if( feature_type == TABFC_MultiPoint )
        poFeature = new TABMultiPoint(poFile->GetLayerDefn());

    return poFeature;
}

/************************************************************************/
/*                         mitab_c_set_field()                          */
/************************************************************************/

/**
 * Set the value of specified field in a feature object.
 *
 * @param feature the mitab_feature object.
 * @param field_index the 0-based index of the field to set.
 * @param field_value a string containing the value to set the field to.
 *        For any field type, the value should always be passed using its
 *        character string representation.
 */
void MITAB_STDCALL
mitab_c_set_field( mitab_feature feature, int field_index,
                   const char *field_value )

{
    TABFeature  *poFeature = (TABFeature *) feature;

    poFeature->SetField( field_index, field_value );
}

/************************************************************************/
/*                         mitab_c_set_points()                         */
/************************************************************************/

/** 
 * Set the feature's coordinates.
 *
 * @param feature the mitab_feature object.
 * @param part the part number.  Applies only to a region or a polyline, it
 *        is ignored for other object types.  For regions and polyline, with
 *        multiple parts, call mitab_c_set_points() once for each part of
 *        the object (ring or polyline segment), starting with 0 for the
 *        first part.  
 *        Note that it is only possible to add parts in a sequential order,
 *        and it is not possible to overwrite or modify existing parts using
 *        this function.
 * @param vertex_count the number of points (pairs of x,y values).
 * @param x the array of 'vertex_count' X values.
 *        Note: for rectangle objects, the MBR of the array of points
 *        defines rectangle corners.
 * @param y the array of 'vertex_count' Y values.
 */

void MITAB_STDCALL
mitab_c_set_points( mitab_feature feature, int part,
                    int vertex_count, double * x, double * y )

{
    TABFeature  *poFeature = (TABFeature *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Point
        || poFeature->GetFeatureClass() == TABFC_FontPoint
        || poFeature->GetFeatureClass() == TABFC_CustomPoint
        || poFeature->GetFeatureClass() == TABFC_Text )
    {
        CPLAssert( vertex_count == 1 );
        poFeature->SetGeometryDirectly( new OGRPoint( x[0], y[0] ) );
    }

    else if( poFeature->GetFeatureClass() == TABFC_Polyline )
    {
        OGRLineString   *poLine = new OGRLineString();

        poLine->setPoints( vertex_count, x, y );

        if ( poFeature->GetGeometryRef() != NULL && part > 0 )
        {
            OGRGeometry *poGeom = poFeature->GetGeometryRef();
            if (poGeom->getGeometryType() == wkbLineString && part == 1)
            {
                OGRMultiLineString *poMulti = new OGRMultiLineString();
                /* Note: we use addGeometry() to add poGeom to poMulti since
                 * original geometry object will be freed by call to 
                 * SetGeometryDirectly() below.
                 */
                poMulti->addGeometry(poGeom);
                poMulti->addGeometryDirectly(poLine);

                poFeature->SetGeometryDirectly( poMulti );
            }
            else if (poGeom->getGeometryType() == wkbMultiLineString)
            {
                CPLAssert( part > 1 );
                OGRMultiLineString *poMulti = (OGRMultiLineString *)poGeom;
                poMulti->addGeometryDirectly(poLine);
            }
        }
        else
        {
            CPLAssert( part == 0 );
            poFeature->SetGeometryDirectly( poLine );
        }
    }

    else if( poFeature->GetFeatureClass() == TABFC_Region )
    {
        OGRLinearRing   *poRing = new OGRLinearRing();
        OGRPolygon      *poPolygon;

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

    else if( poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        if ( poFeature->GetGeometryRef() == NULL && part == 0 )
        {
            // Rectangle: The MBR of the first part defines the rectangle 
            // corners
            OGRPolygon      *poPoly = new OGRPolygon;
            OGRLinearRing   *poRing = new OGRLinearRing;

            poRing->setPoints( vertex_count, x, y );

            poPoly->addRingDirectly( poRing );
            poFeature->SetGeometryDirectly( poPoly );
        }
    }

    else if( poFeature->GetFeatureClass() == TABFC_MultiPoint )
    {
        OGRPoint        *poPoint;
        OGRMultiPoint   *poMultiPoint;
        int i;

        if( poFeature->GetGeometryRef() != NULL )
            poMultiPoint = (OGRMultiPoint *) poFeature->GetGeometryRef();
        else
            poMultiPoint = new OGRMultiPoint;

        for(i=0; i<vertex_count; i++)
        {
            poPoint =  new OGRPoint( x[0], y[0] );
            poMultiPoint->addGeometryDirectly( poPoint );
        }
        poFeature->SetGeometryDirectly( poMultiPoint );
    }

}

/************************************************************************/
/*                         mitab_c_set_points()                         */
/************************************************************************/

/** 
 * Set an arc or ellipse feature parameters.
 *
 * @param feature the mitab_feature object.
 * @param center_x the arc/ellipse center X coordinate.
 * @param center_y the arc/ellipse center Y coordinate.
 * @param x_radius the arc/ellipse X radius.
 * @param y_radius the arc/ellipse Y radius.
 * @param start_angle for an arc: the start angle in degrees, counterclockwise.
 *                    for an ellipse, this parameter is ignored.
 * @param end_angle for an arc: the end angle in degrees, counterclockwise.
 *                  for an ellipse, this parameter is ignored.
 */

void MITAB_STDCALL
mitab_c_set_arc( mitab_feature feature, 
                 double center_x, double center_y,
                 double x_radius, double y_radius,
                 double start_angle, double end_angle)

{
    TABFeature  *poFeature = (TABFeature *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Arc )
    {
        TABArc *poArc = (TABArc *)poFeature;

        poArc->m_dCenterX = center_x;
        poArc->m_dCenterY = center_y;
        poArc->m_dXRadius = x_radius;
        poArc->m_dYRadius = y_radius;
        poArc->SetStartAngle(start_angle);
        poArc->SetEndAngle(end_angle);

        // We also need a point geometry to make things legal.
        OGRPoint      oPoint( center_x, center_y );
        poArc->SetGeometry( &oPoint );
    }
    else if (poFeature->GetFeatureClass() == TABFC_Ellipse)
    {
        TABEllipse *poEllipse = (TABEllipse *)poFeature;

        poEllipse->m_dCenterX = center_x;
        poEllipse->m_dCenterY = center_y;
        poEllipse->m_dXRadius = x_radius;
        poEllipse->m_dYRadius = y_radius;

        // TABEllipse expects a polygon geometry... just use the MBR
        OGRPolygon      *poPoly = new OGRPolygon;
        OGRLinearRing   *poRing = new OGRLinearRing;

        poRing->setNumPoints(5);
        poRing->setPoint(0, center_x-x_radius, center_y-y_radius );
        poRing->setPoint(1, center_x-x_radius, center_y+y_radius );
        poRing->setPoint(2, center_x+x_radius, center_y+y_radius );
        poRing->setPoint(3, center_x+x_radius, center_y-y_radius );
        poRing->setPoint(4, center_x-x_radius, center_y-y_radius );

        poPoly->addRingDirectly( poRing );
        poEllipse->SetGeometryDirectly( poPoly );
    }

}

/************************************************************************/
/*                          mitab_c_set_text()                          */
/************************************************************************/

/**
 * Set the text string on a TABFC_Text object.
 *
 * @param feature the mitab_feature object.
 * @param text the text string to set in the object.
 */

void MITAB_STDCALL
mitab_c_set_text( mitab_feature feature, const char * text )

{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poFeature->SetTextString( text );
}

/************************************************************************/
/*                          mitab_c_get_text()                          */
/************************************************************************/

/**
 * Get the text string on a TABFC_Text object.
 *
 * @param feature the mitab_feature object.
 * @return the text string in the object.
 */

const char * MITAB_STDCALL
mitab_c_get_text( mitab_feature feature )

{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
        return poFeature->GetTextString( );

    return "";
}

/************************************************************************/
/*                          mitab_c_get_text_vb()                       */
/************************************************************************/

/**
 * Get the text string on a TABFC_Text object (VB Version).
 *
 * @param feature the mitab_feature object.
 * @param text string buffer to return the text string in the object.
 * @param l the maximum length of the text string including terminating null.
 * @return the length of the text string in the object.
 */

int MITAB_STDCALL 
mitab_c_get_text_vb( mitab_feature feature, char * text, int l )

{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {  
      strncpy (text,poFeature->GetTextString( ),l); 
      return strlen(text);
    }

    return 0;
}


/************************************************************************/
/*                          mitab_c_set_text_display()                  */
/************************************************************************/

/**
 * Set a TABFC_Text object's display properties.
 *
 * See the MIF specs for more details on the meaning and valid values of
 * each parameter.
 *
 * @param feature the mitab_feature object.
 * @param angle the text angle in degrees.
 * @param height the height of the text's MBR in ground units.
 * @param width the width of the text's MBR in ground units.
 * @param fg_color foreground color (24 bits RGB value).
 * @param bg_color background color.
 * @param justification one of TABTJ_Left (0), TABTJ_Center (1), or 
 *        TABTJ_Right (2).
 * @param spacing one of TABTS_1 (0), TABTS_1_5 (1), or TABTS_2 (2)
 * @param linetype one of TABTL_NoLine (0), TABTL_Simple (1), or 
 *        TABTL_Arrow (2).
 */

void MITAB_STDCALL
mitab_c_set_text_display( mitab_feature feature,
                          double angle, double height, double width,
                          int fg_color, int bg_color,
                          int justification, int spacing, int linetype )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        poFeature->SetTextAngle( angle );
        if( height > 0 )
            poFeature->SetTextBoxHeight( height );
        if( width > 0 )
            poFeature->SetTextBoxWidth( width );
        if( fg_color != -1 )
            poFeature->SetFontFGColor( fg_color );
        if( bg_color != -1 )
            poFeature->SetFontBGColor( bg_color );

        if( justification != -1 )
            poFeature->SetTextJustification( (TABTextJust) justification );
        if( spacing != -1 )
            poFeature->SetTextSpacing( (TABTextSpacing) justification );
        if( linetype != -1 )
            poFeature->SetTextLineType( (TABTextLineType) linetype );
    }
}

/************************************************************************/
/*                       mitab_c_get_text_angle()                       */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's angle property.
 *
 * @param feature the mitab_feature object.
 * @return the text angle in degrees.
 */

double MITAB_STDCALL
mitab_c_get_text_angle( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextAngle();
    }
    return 0.0;
}

/************************************************************************/
/*                       mitab_c_get_text_height()                       */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's MBR height property.
 *
 * @param feature the mitab_feature object.
 * @return  the height of the text's MBR in ground units.
 */

double MITAB_STDCALL
mitab_c_get_text_height( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextBoxHeight();
    }
    return 0.0;
}

/************************************************************************/
/*                       mitab_c_get_text_width()                       */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's MBR width property.
 *
 * @param feature the mitab_feature object.
 * @return  the width of the text's MBR in ground units.
 */

double MITAB_STDCALL
mitab_c_get_text_width( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextBoxWidth();
    }
    return 0.0;
}

/************************************************************************/
/*                      mitab_c_get_text_fgcolor()                      */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's foreground color property.
 *
 * @param feature the mitab_feature object.
 * @return the text foreground color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_text_fgcolor( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetFontFGColor();
    }
    return 0x000000;
}

/************************************************************************/
/*                      mitab_c_get_text_bgcolor()                      */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's background color property.
 *
 * @param feature the mitab_feature object.
 * @return the text background color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_text_bgcolor( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetFontBGColor();
    }
    return 0x000000;
}

/************************************************************************/
/*                   mitab_c_get_text_justification()                   */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's justification property.
 *
 * @param feature the mitab_feature object.
 * @return the text justification, one of TABTJ_Left (0), TABTJ_Center (1), or 
 *        TABTJ_Right (2).
 */

int MITAB_STDCALL
mitab_c_get_text_justification( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextJustification();
    }
    return 0;
}

/************************************************************************/
/*                      mitab_c_get_text_spacing()                      */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's spacing property.
 *
 * @param feature the mitab_feature object.
 * @return the text spacing, one of TABTS_1 (0), TABTS_1_5 (1), or TABTS_2 (2).
 */

int MITAB_STDCALL
mitab_c_get_text_spacing( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextSpacing();
    }
    return 0;
}

/************************************************************************/
/*                     mitab_c_get_text_linetype()                      */
/************************************************************************/

/**
 * Fetch a TABFC_Text object's linetype property.
 *
 * @param feature the mitab_feature object.
 * @return the text linetype, one of TABTL_NoLine (0), TABTL_Simple (1), or 
 *        TABTL_Arrow (2).
 */

int MITAB_STDCALL
mitab_c_get_text_linetype( mitab_feature feature )
    
{
    TABText     *poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
    {
        return poFeature->GetTextLineType();
    }
    return 0;
}

/************************************************************************/
/*                          mitab_c_set_font()                          */
/************************************************************************/

/** 
 * Set the font name in a a TABFC_Text or TABFC_FontPoint object, or set the
 * symbol name in a TABFC_CustomPoint.
 *
 * @param feature the mitab_feature object.
 * @param fontname the new font name.
 */

void MITAB_STDCALL
mitab_c_set_font( mitab_feature feature, const char * fontname )

{
    TABFeature *poFeature = (TABFeature *)feature;

    if( poFeature->GetFeatureClass() == TABFC_Text ||
        poFeature->GetFeatureClass() == TABFC_FontPoint ||
        poFeature->GetFeatureClass() == TABFC_CustomPoint )
    {
        ITABFeatureFont *poFontFeature = (ITABFeatureFont *) poFeature;

        poFontFeature->SetFontName( fontname );
    }
}

/************************************************************************/
/*                          mitab_c_get_font()                          */
/************************************************************************/

/** 
 * Get the font name from a TABFC_Text or TABFC_FontPoint object, or the
 * symbol name from a TABFC_CustomPoint.
 *
 * @param feature the mitab_feature object.
 * @return the text font name.
 */

const char * MITAB_STDCALL
mitab_c_get_font( mitab_feature feature )

{
    TABFeature *poFeature = (TABFeature *)feature;

    if( poFeature->GetFeatureClass() == TABFC_Text ||
        poFeature->GetFeatureClass() == TABFC_FontPoint ||
        poFeature->GetFeatureClass() == TABFC_CustomPoint )
    {
        ITABFeatureFont *poFontFeature = (ITABFeatureFont *) poFeature;

        return poFontFeature->GetFontNameRef();
    }

    return "";
}

/************************************************************************/
/*                          mitab_c_get_font_vb()                       */
/************************************************************************/

/** 
 * Get the font name from a TABFC_Text or TABFC_FontPoint object, or the
 * symbol name from a TABFC_CustomPoint. (VB Version)
 *
 * @param feature the mitab_feature object.
 * @param font string buffer to return the text font name.
 * @param l the maximum lentgh of the text string including terminating null.
 * @return the length of the text font name.
 */

int MITAB_STDCALL 
mitab_c_get_font_vb( mitab_feature feature, char * font, int l )
{
    TABFeature *poFeature = (TABFeature *)feature;

    if( poFeature->GetFeatureClass() == TABFC_Text ||
        poFeature->GetFeatureClass() == TABFC_FontPoint ||
        poFeature->GetFeatureClass() == TABFC_CustomPoint )
    {
        ITABFeatureFont *poFontFeature = (ITABFeatureFont *) poFeature;

        strncpy(font, poFontFeature->GetFontNameRef(), l);
        return strlen(font);
    }

    return 0;
}


/************************************************************************/
/*                         mitab_c_set_brush()                          */
/************************************************************************/

/**
 * Set an object's brush properties.  Applies to region, ellipse and 
 * rectangle objects.
 *
 * See the MIF specs for more details on the meaning and valid values of
 * each parameter.
 *
 * @param feature the mitab_feature object.
 * @param fg_color the foreground color (24 bits RGB value).
 * @param bg_color the background color.
 * @param pattern the brush number (1 is none, 2 is solid fill, etc.).
 * @param transparent either 0 for an opaque brush (using bg color) or 1 for 
 *        transparent (ignore bg color).
 */

void MITAB_STDCALL
mitab_c_set_brush( mitab_feature feature,
                   int fg_color, int bg_color, int pattern, int transparent )

{
    TABRegion   *poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region ||
        poFeature->GetFeatureClass() == TABFC_Ellipse ||
        poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        poFeature->SetBrushFGColor( fg_color );
        poFeature->SetBrushBGColor( bg_color );
        poFeature->SetBrushPattern( pattern );
        poFeature->SetBrushTransparent( transparent );
    }
}

/************************************************************************/
/*                     mitab_c_get_brush_fgcolor()                      */
/************************************************************************/

/**
 * Get an object's brush foreground color property.  Applies to region, 
 * ellipse and rectangle objects.
 *
 * @param feature the mitab_feature object.
 * @return the brush foreground color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_brush_fgcolor( mitab_feature feature )
    
{
    TABRegion   *poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region ||
        poFeature->GetFeatureClass() == TABFC_Ellipse ||
        poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        return poFeature->GetBrushFGColor();
    }
    return 0x000000;
}

/************************************************************************/
/*                     mitab_c_get_brush_bgcolor()                      */
/************************************************************************/

/**
 * Get an object's brush background color property.  Applies to region, 
 * ellipse and rectangle objects.
 *
 * @param feature the mitab_feature object.
 * @return the brush background color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_brush_bgcolor( mitab_feature feature )
    
{
    TABRegion   *poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region ||
        poFeature->GetFeatureClass() == TABFC_Ellipse ||
        poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        return poFeature->GetBrushBGColor();
    }
    return 0x000000;
}

/************************************************************************/
/*                     mitab_c_get_brush_pattern()                      */
/************************************************************************/

/**
 * Get an object's brush pattern property.  Applies to region, 
 * ellipse and rectangle objects.
 *
 * @param feature the mitab_feature object.
 * @return the brush pattern number (1 is none, 2 is solid fill, etc.).
 */

int MITAB_STDCALL
mitab_c_get_brush_pattern( mitab_feature feature )
    
{
    TABRegion   *poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region ||
        poFeature->GetFeatureClass() == TABFC_Ellipse ||
        poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        return poFeature->GetBrushPattern();
    }
    return 1;
}

/************************************************************************/
/*                   mitab_c_get_brush_transparent()                    */
/************************************************************************/

/**
 * Get an object's brush transparency property.  Applies to region, 
 * ellipse and rectangle objects.
 *
 * @param feature the mitab_feature object.
 * @return the brush transparency value, either 0 for an opaque brush 
 *        (using bg color) or 1 for transparent (ignore bg color).
 */

int MITAB_STDCALL
mitab_c_get_brush_transparent( mitab_feature feature )
    
{
    TABRegion   *poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region ||
        poFeature->GetFeatureClass() == TABFC_Ellipse ||
        poFeature->GetFeatureClass() == TABFC_Rectangle )
    {
        return poFeature->GetBrushTransparent();
    }
    return 0;
}



/************************************************************************/
/*                          mitab_c_set_pen()                           */
/************************************************************************/

/**
 * Set an object's pen properties.  Applies only to polyline, region, 
 * rectangle, arc and ellipse objects.
 *
 * See the MIF specs for more details on the meaning and valid values of
 * each parameter.
 *
 * @param feature the mitab_feature object.
 * @param width the pen width as defined in the MIF specs:  1-7 for pixel 
 *        width, or 11-2047 for width in points (10 + (point_width*10))
 * @param pattern the pen number (2 is default solid pen).
 * @param color the pen color (24 bits RGB value).
 */

void MITAB_STDCALL
mitab_c_set_pen( mitab_feature feature,
                 int width, int pattern, int color )

{
    TABFeature          *poFeature = (TABFeature *) feature;
    ITABFeaturePen      *poPen = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
        poPen = ((TABPolyline *) poFeature);
    
    if( poFeature->GetFeatureClass() == TABFC_Region )
        poPen = ((TABRegion *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Rectangle )
        poPen = ((TABRectangle *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Arc )
        poPen = ((TABArc *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Ellipse )
        poPen = ((TABEllipse *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poPen = ((TABText *) poFeature);

    if( poPen != NULL )
    {
        poPen->SetPenWidthMIF( width );
        poPen->SetPenPattern( pattern );
        poPen->SetPenColor( color );
    }
}

/************************************************************************/
/*                       mitab_c_get_pen_color()                        */
/************************************************************************/

/**
 * Get an object's pen color property.  Applies only to polyline, region, 
 * rectangle, arc and ellipse objects.
 *
 * @param feature the mitab_feature object.
 * @return the pen color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_pen_color( mitab_feature feature )
    
{
    TABFeature          *poFeature = (TABFeature *) feature;
    ITABFeaturePen      *poPen = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
        poPen = ((TABPolyline *) poFeature);
    
    if( poFeature->GetFeatureClass() == TABFC_Region )
        poPen = ((TABRegion *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Rectangle )
        poPen = ((TABRectangle *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Arc )
        poPen = ((TABArc *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Ellipse )
        poPen = ((TABEllipse *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poPen = ((TABText *) poFeature);

    if( poPen != NULL )
    {
        return poPen->GetPenColor();
    }

    return 0x000000;
}

/************************************************************************/
/*                       mitab_c_get_pen_width()                        */
/************************************************************************/

/**
 * Get an object's pen width property.  Applies only to polyline, region, 
 * rectangle, arc and ellipse objects.
 *
 * @param feature the mitab_feature object.
 * @return the pen as defined in the MIF specs:  1-7 for pixel 
 *        width, or 11-2047 for width in points (10 + (point_width*10)).
 */

int MITAB_STDCALL
mitab_c_get_pen_width( mitab_feature feature )
    
{
    TABFeature          *poFeature = (TABFeature *) feature;
    ITABFeaturePen      *poPen = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
        poPen = ((TABPolyline *) poFeature);
    
    if( poFeature->GetFeatureClass() == TABFC_Region )
        poPen = ((TABRegion *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Rectangle )
        poPen = ((TABRectangle *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Arc )
        poPen = ((TABArc *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Ellipse )
        poPen = ((TABEllipse *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poPen = ((TABText *) poFeature);

    if( poPen != NULL )
    {
        return poPen->GetPenWidthMIF();
    }

    return 0;
}

/************************************************************************/
/*                      mitab_c_get_pen_pattern()                       */
/************************************************************************/

/**
 * Get an object's pen pattern property.  Applies only to polyline, region, 
 * rectangle, arc and ellipse objects.
 *
 * @param feature the mitab_feature object.
 * @return the pen pattern number (2 is default solid pen).
 */

int MITAB_STDCALL
mitab_c_get_pen_pattern( mitab_feature feature )
    
{
    TABFeature          *poFeature = (TABFeature *) feature;
    ITABFeaturePen      *poPen = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
        poPen = ((TABPolyline *) poFeature);
    
    if( poFeature->GetFeatureClass() == TABFC_Region )
        poPen = ((TABRegion *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Rectangle )
        poPen = ((TABRectangle *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Arc )
        poPen = ((TABArc *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Ellipse )
        poPen = ((TABEllipse *) poFeature);

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poPen = ((TABText *) poFeature);

    if( poPen != NULL )
    {
        return poPen->GetPenPattern();
    }

    return 0x000000;
}


/************************************************************************/
/*                         mitab_c_set_symbol()                         */
/************************************************************************/

/**
 * Set an object's symbol properties.  Applies only to point objects and
 * multipoint.
 *
 * See the MIF specs for more details on the meaning and valid values of
 * each parameter.
 *
 * @param feature the mitab_feature object.
 * @param symbol_no the symbol number (valid range: 32 to 67)
 * @param symbol_size the symbol size in pixels (valid range 1 to 48)
 * @param symbol_color the symbol color (24 bits RGB value)
 */

void MITAB_STDCALL
mitab_c_set_symbol( mitab_feature feature, int symbol_no,
                    int symbol_size, int symbol_color )

{
    TABPoint    *poFeature = (TABPoint *) feature;

    if(( poFeature->GetFeatureClass() == TABFC_Point ) ||
       ( poFeature->GetFeatureClass() == TABFC_FontPoint ) ||
       ( poFeature->GetFeatureClass() == TABFC_CustomPoint ) ||
       ( poFeature->GetFeatureClass() == TABFC_MultiPoint ))
    {
        poFeature->SetSymbolNo( symbol_no );
        poFeature->SetSymbolSize( symbol_size );
        poFeature->SetSymbolColor( symbol_color );
    }
}                                                               

/************************************************************************/
/*                     mitab_c_get_symbol_color()                       */
/************************************************************************/

/**
 * Get an object's symbol color property.  Applies only to point and 
 * multipoint objects.
 *
 * @param feature the mitab_feature object.
 * @return the symbol color (24 bits RGB value).
 */

int MITAB_STDCALL
mitab_c_get_symbol_color( mitab_feature feature )
    
{
    TABPoint    *poFeature = (TABPoint *) feature;

    if(( poFeature->GetFeatureClass() == TABFC_Point ) ||
       ( poFeature->GetFeatureClass() == TABFC_MultiPoint ))
    {
        return poFeature->GetSymbolColor();
    }
    return 0x000000;
}

/************************************************************************/
/*                       mitab_c_get_symbol_no()                        */
/************************************************************************/

/**
 * Get an object's symbol number property.  Applies only to point and 
 * multipoint objects.
 *
 * @param feature the mitab_feature object.
 * @return the symbol number (valid range: 32 to 67)
 */

int MITAB_STDCALL
mitab_c_get_symbol_no( mitab_feature feature )
    
{
    TABPoint    *poFeature = (TABPoint *) feature;

    if(( poFeature->GetFeatureClass() == TABFC_Point ) ||
       ( poFeature->GetFeatureClass() == TABFC_MultiPoint ))
    {
        return poFeature->GetSymbolNo();
    }
    return 0;
}

/************************************************************************/
/*                     mitab_c_get_symbol_size()                        */
/************************************************************************/

/**
 * Get an object's symbol size property.  Applies only to point and 
 * multipoint objects.
 *
 * @param feature the mitab_feature object.
 * @return the symbol size in pixels (valid range 1 to 48)
 */

int MITAB_STDCALL
mitab_c_get_symbol_size( mitab_feature feature )
    
{
    TABPoint    *poFeature = (TABPoint *) feature;

    if(( poFeature->GetFeatureClass() == TABFC_Point ) ||
       ( poFeature->GetFeatureClass() == TABFC_MultiPoint ))
    {
        return poFeature->GetSymbolSize();
    }
    return 1;
}


/************************************************************************/
/*                          mitab_c_get_type()                          */
/************************************************************************/

/**
 * Return a mitab_feature's object type
 *
 * @param feature the mitab_feature object.
 * @return the feature type, one of TABFC_NoGeom (0), TABFC_Point (1), 
 *         TABFC_FontPoint (2), TABFC_CustomPoint (3), TABFC_Text (4),
 *         TABFC_Polyline (5), TABFC_Arc (6), TABFC_Region (7),
 *         TABFC_Rectangle (8), TABFC_Ellipse (9) or TABFC_MultiPoint (10).
 */

int MITAB_STDCALL 
mitab_c_get_type( mitab_feature feature )
{
    TABFeature          *poFeature = (TABFeature *) feature;

    if (poFeature)
        return poFeature->GetFeatureClass();

    return 0;
}



/************************************************************************/
/*                         mitab_c_get_parts()                          */
/************************************************************************/

/**
 * Return the number of parts (rings or polyline segments) in an object.
 *
 * @param feature the mitab_feature object.
 * @return the number of parts (in a region or polyline) or 0 if the object's
 *         geometry was not set.  For all object types other than polyline 
 *         region, returns 1 if object geometry is set.
 */

int MITAB_STDCALL 
mitab_c_get_parts( mitab_feature feature )
{
    int numParts = 0;

    if (_mitab_c_get_feature_info( feature, INFO_NUMPARTS, &numParts, 
                                   NULL, NULL ) == 0)
    {
        return numParts;
    }

    return 0;
}

/************************************************************************/
/*                      mitab_c_get_vertex_count()                      */
/************************************************************************/

/**
 * Return the number of points in a part of a mitab_feature object.
 *
 * @param feature the mitab_feature object.
 * @param part the part number we are interested in.  Use 0 for objects which
 *         cannot have multiple parts.
 * @return the number of points in that feature's part or 0 if the object
 *         has no geometry or the part number is invalid.
 */

int MITAB_STDCALL
mitab_c_get_vertex_count( mitab_feature feature, int part )
{
    int numPoints = 0;

    if (_mitab_c_get_feature_info( feature, INFO_NUMPOINTS, &part, 
                                   &numPoints, NULL ) == 0)
    {
        return numPoints;
    }

    return 0;
}

/************************************************************************/
/*                        mitab_c_get_vertex_x()                        */
/************************************************************************/

/** 
 * Return the X coordinate of a point in a part of a mitab_feature object.
 *
 * @param feature the mitab_feature object.
 * @param part the part number we are interested in.  Use 0 for objects which
 *         cannot have multiple parts.
 * @param point the point number, with 0 being the first point.
 * @return the X coordinate of the specified point or if the object has no
 *         geometry or the part or the point number is invalid.
 */

double MITAB_STDCALL
mitab_c_get_vertex_x( mitab_feature feature, int part, int vertex )
{
    double dX = 0.0;

    if (_mitab_c_get_feature_info( feature, INFO_XVERTEX, &part, 
                                   &vertex, &dX ) == 0)
    {
        return dX;
    }

    return 0.0;
}

/************************************************************************/
/*                        mitab_c_get_vertex_x()                        */
/************************************************************************/

/** 
 * Return the Y coordinate of a point in a part of a mitab_feature object.
 *
 * @param feature the mitab_feature object.
 * @param part the part number we are interested in.  Use 0 for objects which
 *         cannot have multiple parts.
 * @param point the point number, with 0 being the first point.
 * @return the Y coordinate of the specified point or if the object has no
 *         geometry or the part or the point number is invalid.
 */

double MITAB_STDCALL
mitab_c_get_vertex_y( mitab_feature feature, int part, int vertex )
{
    double dY = 0.0;

    if (_mitab_c_get_feature_info( feature, INFO_YVERTEX, &part, 
                                   &vertex, &dY ) == 0)
    {
        return dY;
    }

    return 0.0;
}

/************************************************************************/
/*                      mitab_c_get_field_count()                       */
/************************************************************************/

/**
 * Return the number of attribute fields in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @return the number of attribute fields defined in the dataset.
 */

int MITAB_STDCALL 
mitab_c_get_field_count( mitab_handle handle )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;

    if (poFile && (poDefn = poFile->GetLayerDefn()) != NULL)
        return poDefn->GetFieldCount();

    return 0;
}

/************************************************************************/
/*                       mitab_c_get_field_type()                       */
/************************************************************************/

/**
 * Return the type of an attribute field in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field type, one of TABFT_Char (1), 
 *        TABFT_Integer (2), TABFT_SmallInt (3), TABFT_Decimal (4), 
 *        TABFT_Float (5), TABFT_Date (6), or TABFT_Logical (7)
 */

int MITAB_STDCALL 
mitab_c_get_field_type( mitab_handle handle, int field )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;

    if (poFile)
        return poFile->GetNativeFieldType(field);

    return 0;
}

/************************************************************************/
/*                       mitab_c_get_field_name()                       */
/************************************************************************/

/**
 * Return the name of an attribute field in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field name.  The returned string pointer is a reference to an
 *        internal buffer and should not be modified or freed by the caller.
 */

const char * MITAB_STDCALL
mitab_c_get_field_name( mitab_handle handle, int field )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;
    OGRFieldDefn        *poFDefn;

    if (poFile && 
        (poDefn = poFile->GetLayerDefn()) != NULL &&
        (poFDefn = poDefn->GetFieldDefn(field)) != NULL)
    {
        return poFDefn->GetNameRef();
    }

    return "";
}

/************************************************************************/
/*                       mitab_c_get_field_name_vb()                    */
/************************************************************************/

/**
 * Return the name of an attribute field in a dataset's schema (VB version).
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @param name string buffer to return the field name. 
 * @param l the maximum lenght of the name string including terminating null. 
 * @return the length of the field name.
 */
int MITAB_STDCALL
mitab_c_get_field_name_vb( mitab_handle handle, int field, char * name, int l )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;
    OGRFieldDefn        *poFDefn;

    if (poFile && 
        (poDefn = poFile->GetLayerDefn()) != NULL &&
        (poFDefn = poDefn->GetFieldDefn(field)) != NULL)
    {
        strncpy (name,poFDefn->GetNameRef(),l);
        return (strlen(name));
    }

    return 0;
}


/************************************************************************/
/*                       mitab_c_get_field_width()                      */
/************************************************************************/

/**
 * Return the width of an attribute field in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field width.
 */

int MITAB_STDCALL 
mitab_c_get_field_width( mitab_handle handle, int field )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;
    OGRFieldDefn        *poFDefn;

    if (poFile && 
        (poDefn = poFile->GetLayerDefn()) != NULL &&
        (poFDefn = poDefn->GetFieldDefn(field)) != NULL)
    {
        return poFDefn->GetWidth();
    }

    return 0;
}

/************************************************************************/
/*                       mitab_c_get_field_precision()                  */
/************************************************************************/

/**
 * Return the precision of an attribute field in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field precision.
 */

int MITAB_STDCALL 
mitab_c_get_field_precision( mitab_handle handle, int field )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;
    OGRFieldDefn        *poFDefn;

    if (poFile && 
        (poDefn = poFile->GetLayerDefn()) != NULL &&
        (poFDefn = poDefn->GetFieldDefn(field)) != NULL)
    {
        return poFDefn->GetPrecision();
    }

    return 0;
}


/************************************************************************/
/*                    mitab_c_get_field_as_string()                     */
/************************************************************************/

/**
 * Fetch an attribute field value in a mitab_feature as a string.
 *
 * The function returns a reference to an internal string buffer that contains
 * the string representation of the attribute field's value (integer
 * and floating point values are converted to string using sprintf()).
 *
 * @param feature the mitab_feature object.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return a string containing the value of the field.  The returned string
 *         pointer is a reference to an internal buffer and should not be
 *         modified or freed by the caller.  Its value will be valid only
 *         until the next call to mitab_c_get_field().
 */

const char * MITAB_STDCALL
mitab_c_get_field_as_string( mitab_feature feature, int field )
{
    TABFeature          *poFeature = (TABFeature *) feature;

    if (poFeature)
        return poFeature->GetFieldAsString(field);

    return "";
}

/************************************************************************/
/*                    mitab_c_get_field_as_string_vb()                  */
/************************************************************************/

/**
 * Fetch an attribute field value in a mitab_feature as a string (VB Version).
 *
 * The function returns a reference to the string representation of the 
 * attribute field's value (integer and floating point values are converted 
 * to string using sprintf()).
 *
 * @param feature the mitab_feature object.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @param value string buffer to return the value of the field.
 * @param l the maximum lenght of the value string including terminating null. 
 * @return the length of the string containing the value of the field.
 */

int MITAB_STDCALL
mitab_c_get_field_as_string_vb( mitab_feature feature, int field, char * value, int l )
{
    TABFeature          *poFeature = (TABFeature *) feature;

    if (poFeature)
    {
        strncpy(value,poFeature->GetFieldAsString(field),l);
        return strlen(value);
    };
    return 0;
}


/********************************************************************/
/*                    mitab_c_get_projinfo()                        */
/********************************************************************/
 
/**
 * Get the projinfo handle from an opened dataset
 *
 * @param dataset the mitab_handle of the source dataset.
 * @return a mitab_projinfo handle usable with mitab_c_set_projinfo() or NULL
 *    if the information is not available.
 *    The handle is valid only until the next call to mitab_c_get_projinfo().
 */
 
mitab_projinfo MITAB_STDCALL
mitab_c_get_projinfo( mitab_handle dataset )
{
    static TABProjInfo  sProjInfo;
    IMapInfoFile        *poFile = (IMapInfoFile *) dataset;

    if( poFile->GetProjInfo(&sProjInfo) == 0 )
    {
        return &sProjInfo;
    }

    return NULL;
}
 

/********************************************************************/
/*                    mitab_c_set_projinfo()                        */
/********************************************************************/
 
/**
 * Set the projinfo on a newly created dataset.  Should be called immediately
 * after creating the dataset and before adding features to it.
 *
 * @param dataset the mitab_handle of the target dataset.
 * @param projinfo the mitab_projinfo to set on the target dataset, obtained
 *        from mitab_c_get_projinfo().
 * @return 0 on success, -1 on failure.
 */
 
int MITAB_STDCALL
mitab_c_set_projinfo( mitab_handle dataset, mitab_projinfo projinfo )
{
    IMapInfoFile        *poFile = (IMapInfoFile *) dataset;
    TABProjInfo         *psProjInfo = (TABProjInfo *) projinfo;

    if (poFile && psProjInfo)
        return poFile->SetProjInfo(psProjInfo);

    return -1;
}
 


/************************************************************************/
/*                        mitab_c_get_mif_coordsys()                    */
/************************************************************************/

/**
 * Get the MIF CoordSys string from an opened dataset.
 *
 * @param dataset the mitab_handle of the source dataset.
 * @return a string with the dataset coordinate system definition in MIF
 *    CoordSys format.  This value can be passed to mitab_c_create() to 
 *    create new datasets with the same coordinate system.
 *    Returns NULL if the information could not be read.
 *    The returned string is valid only until the next call to 
 *    mitab_c_get_mif_coordsys().
 */

const char * MITAB_STDCALL
mitab_c_get_mif_coordsys( mitab_handle dataset)
{
    static char *spszCoordSys = NULL;
    IMapInfoFile        *poFile = (IMapInfoFile *) dataset;
    OGRSpatialReference *poSRS;

    if (poFile && (poSRS = poFile->GetSpatialRef()) != NULL)
    {
        CPLFree( spszCoordSys );
        spszCoordSys = MITABSpatialRef2CoordSys( poSRS );
        return spszCoordSys;
    }

    return NULL;
}

/************************************************************************/
/*                        mitab_c_get_mif_coordsys_vb()                 */
/************************************************************************/

/**
 * Get the MIF CoordSys string from an opened dataset (VB Version).
 *
 * @param dataset the mitab_handle of the source dataset.
 * @param coordsys a string buffer to return the dataset coordinate system 
 *    definition in MIF CoordSys format.  This value can then be passed 
 *    to mitab_c_create() to create new datasets with the same coordinate
 *    system.  Returns empty string if the information could not be read.
 * @param l the maximum length of the coordsys string including terminating 
 *    null.
 * @return the length of the string in coordsys or zero if the information 
 *    could not be read
 */

int MITAB_STDCALL
mitab_c_get_mif_coordsys_vb( mitab_handle dataset, char * coordsys, int l)
{
    IMapInfoFile        *poFile = (IMapInfoFile *) dataset;
    OGRSpatialReference *poSRS;

    if (poFile && (poSRS = poFile->GetSpatialRef()) != NULL)
    {
        strncpy( coordsys, MITABSpatialRef2CoordSys( poSRS ),l);
        return strlen(coordsys);
    }

    return 0;
}

/* ==================================================================== */
/*                           Helper functions                           */
/* ==================================================================== */

/************************************************************************/
/*                     _mitab_c_get_feature_info()                      */
/*                                                                      */
/*      This is only a helper function... not to be used by outside     */
/*      programs.                                                       */
/************************************************************************/
static int _mitab_c_get_feature_info( mitab_feature feature, int what_info, 
                                      int *part, int *point, double *vertex )
{
    TABFeature  *poFeature = (TABFeature *) feature;
    OGRGeometry *poGeom = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
    {
/* -------------------------------------------------------------------- */
/*      Polyline                                                        */
/* -------------------------------------------------------------------- */
        TABPolyline *poPolyline = (TABPolyline *)poFeature;
        if (what_info == INFO_NUMPARTS)
        {
            *part = poPolyline->GetNumParts();
            return 0;
        }
        OGRLineString *poLine = poPolyline->GetPartRef(*part);
        if (poLine && what_info == INFO_NUMPOINTS)
        {
            *point = poLine->getNumPoints();
            return 0;
        }
        if (poLine && what_info == INFO_XVERTEX)
        {
            *vertex = poLine->getX(*point);
            return 0;
        }
        if (poLine && what_info == INFO_YVERTEX)
        {
            *vertex = poLine->getY(*point);
            return 0;
        }
    }
    else if( poFeature->GetFeatureClass() == TABFC_Region )
    {
/* -------------------------------------------------------------------- */
/*      Region                                                          */
/* -------------------------------------------------------------------- */
        TABRegion *poRegion = (TABRegion *)poFeature;
        if (what_info == INFO_NUMPARTS)
        {
            *part = poRegion->GetNumRings();
            return 0;
        }
        OGRLinearRing *poRing = poRegion->GetRingRef(*part);
        if (poRing && what_info == INFO_NUMPOINTS)
        {
            *point = poRing->getNumPoints();
            return 0;
        }
        if (poRing && what_info == INFO_XVERTEX)
        {
            *vertex = poRing->getX(*point);
            return 0;
        }
        if (poRing && what_info == INFO_YVERTEX)
        {
            *vertex = poRing->getY(*point);
            return 0;
        }
    }
    else if ( (poGeom = poFeature->GetGeometryRef()) != NULL &&
              poGeom->getGeometryType() == wkbPoint)
    {
/* -------------------------------------------------------------------- */
/*      Single point: text and point objects                            */
/* -------------------------------------------------------------------- */
        if (what_info == INFO_NUMPARTS)
        {
            *part = 1;
            return 0;
        }
        if (*part != 0)
            return -1;  /* Invalid part number */

        if (what_info == INFO_NUMPOINTS)
        {
            *point = 1;
            return 0;
        }
        OGRPoint *poPoint = (OGRPoint *)poGeom;
        if (poPoint && what_info == INFO_XVERTEX && *point == 0)
        {
            *vertex = poPoint->getX();
            return 0;
        }
        if (poPoint && what_info == INFO_YVERTEX && *point == 0)
        {
            *vertex = poPoint->getY();
            return 0;
        }
    }
    else if ( poGeom && poGeom->getGeometryType() == wkbLineString)
    {
/* -------------------------------------------------------------------- */
/*      LineString: Arc object                                          */
/* -------------------------------------------------------------------- */
        if (what_info == INFO_NUMPARTS)
        {
            *part = 1;
            return 0;
        }
        if (*part != 0)
            return -1;  /* Invalid part number */

        OGRLineString *poLine = (OGRLineString*)poGeom;
        if (poLine && what_info == INFO_NUMPOINTS)
        {
            *point = poLine->getNumPoints();
            return 0;
        }
        if (poLine && what_info == INFO_XVERTEX)
        {
            *vertex = poLine->getX(*point);
            return 0;
        }
        if (poLine && what_info == INFO_YVERTEX)
        {
            *vertex = poLine->getY(*point);
            return 0;
        }
    }
    else if ( poGeom && poGeom->getGeometryType() == wkbPolygon)
    {
/* -------------------------------------------------------------------- */
/*      Polygon: Rectangle or Ellipse object                            */
/* -------------------------------------------------------------------- */
        if (what_info == INFO_NUMPARTS)
        {
            *part = 1;
            return 0;
        }
        if (*part != 0)
            return -1;  /* Invalid part number */

        OGRPolygon *poPoly = (OGRPolygon*)poGeom;
        OGRLinearRing *poRing = poPoly->getExteriorRing();

        if (poRing && what_info == INFO_NUMPOINTS)
        {
            *point = poRing->getNumPoints();
            return 0;
        }
        if (poRing && what_info == INFO_XVERTEX)
        {
            *vertex = poRing->getX(*point);
            return 0;
        }
        if (poRing && what_info == INFO_YVERTEX)
        {
            *vertex = poRing->getY(*point);
            return 0;
        }
    }
    else if( poFeature->GetFeatureClass() == TABFC_MultiPoint )
    {
/* -------------------------------------------------------------------- */
/*      MultiPoint                                                      */
/* -------------------------------------------------------------------- */
        double dX, dY;
        TABMultiPoint *poMultiPoint = (TABMultiPoint *)poFeature;
        if (what_info == INFO_NUMPARTS)
        {
            *part = 1;
            return 0;
        }
        if (*part != 0)
            return -1;  /* Invalid part number */

        if (poMultiPoint && what_info == INFO_NUMPOINTS)
        {
            *point = poMultiPoint->GetNumPoints();
            return 0;
        }
        if (poMultiPoint && what_info == INFO_XVERTEX)
        {
            return poMultiPoint->GetXY(*point, *vertex, dY);
        }
        if (poMultiPoint && what_info == INFO_YVERTEX)
        {
            return poMultiPoint->GetXY(*point, dX, *vertex);
        }
    }

    return -1;  /* Requested info could not be returned */
}

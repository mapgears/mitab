/**********************************************************************
 * $Id: mitab_capi.cpp,v 1.7 2000-04-21 13:37:59 daniel Exp $
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
 * Copyright (c) 2000, Frank Warmerdam
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
 * Revision 1.7  2000-04-21 13:37:59  daniel
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

const char MITAB_STDCALL *
mitab_c_getlasterrormsg()

{
    const char      *pszLastMessage = CPLGetLastErrorMsg();

    if( pszLastMessage == NULL )
        return "";
    else
        return pszLastMessage;
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
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

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
 *        is assumed.
 * @param north the upper dataset bound.
 *        Note that valid bounds must be provided for a .TAB dataset otherwise
 *        data may not be stored properly in the file.
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
    IMapInfoFile	*poFile;
    
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
    IMapInfoFile	*poFile = (IMapInfoFile *) dataset;

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
    TABFeature	*poFeature = (TABFeature *) feature;

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
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

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
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
    TABFeature		*poFeature = (TABFeature *) feature;

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
 *        TABFC_Point (1), TABFC_Text (4), TABFC_Polyline (5), and 
 *        TABFC_Region (7)
 * @return the new mitab_feature object, or NULL if creation failed.  Note that
 *         the new object will have to be released using 
 *         mitab_c_destroy_feature().
 */

mitab_feature MITAB_STDCALL
mitab_c_create_feature( mitab_handle handle,
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
    {
        TABText		*poText = new TABText(poFile->GetLayerDefn());

        poText->SetTextString( "Default Text" );
        poFeature = poText;
    }
    else if( feature_type == TABFC_Polyline )
        poFeature = new TABPolyline(poFile->GetLayerDefn());
    else if( feature_type == TABFC_Region )
        poFeature = new TABRegion(poFile->GetLayerDefn());

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
    TABFeature	*poFeature = (TABFeature *) feature;

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
 * @param y the array of 'vertex_count' Y values.
 */

void MITAB_STDCALL
mitab_c_set_points( mitab_feature feature, int part,
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
    TABText	*poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poFeature->SetTextString( text );
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
    TABText	*poFeature = (TABText *) feature;

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
/*                          mitab_c_set_font()                          */
/************************************************************************/

/** 
 * Set a TABFC_Text object's font name.
 *
 * @param feature the mitab_feature object.
 * @param fontname the new font name.
 */

void MITAB_STDCALL
mitab_c_set_font( mitab_feature feature, const char * fontname )

{
    TABText	*poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poFeature->SetFontName( fontname );
}

/************************************************************************/
/*                         mitab_c_set_brush()                          */
/************************************************************************/

/**
 * Set an object's brush properties.  Applies only to polygon objects.
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
    TABRegion	*poFeature = (TABRegion *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Region )
    {
        poFeature->SetBrushFGColor( fg_color );
        poFeature->SetBrushBGColor( bg_color );
        poFeature->SetBrushPattern( pattern );
        poFeature->SetBrushTransparent( transparent );
    }
}

/************************************************************************/
/*                          mitab_c_set_pen()                           */
/************************************************************************/

/**
 * Set an object's pen properties.  Applies only to polyline and region
 * objects.
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
    TABFeature		*poFeature = (TABFeature *) feature;
    ITABFeaturePen	*poPen = NULL;

    if( poFeature->GetFeatureClass() == TABFC_Polyline )
        poPen = ((TABPolyline *) poFeature);
    
    if( poFeature->GetFeatureClass() == TABFC_Region )
        poPen = ((TABRegion *) poFeature);

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
/*                         mitab_c_set_symbol()                         */
/************************************************************************/

/**
 * Set an object's symbol properties.  Applies only to point objects.
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
    TABPoint	*poFeature = (TABPoint *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Point )
    {
        poFeature->SetSymbolNo( symbol_no );
        poFeature->SetSymbolSize( symbol_size );
        poFeature->SetSymbolColor( symbol_color );
    }
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
 *         TABFC_Rectangle (8) or TABFC_Ellipse (9).
 */

int MITAB_DLL 
mitab_c_get_type( mitab_feature feature )
{
    TABFeature		*poFeature = (TABFeature *) feature;

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
 * @return the number parts (in a region or polyline) or 0 if the object's
 *         geometry was not set.  For all object types other than polyline 
 *         region, returns 1 if object geometry is set.
 */

int MITAB_DLL 
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

int MITAB_DLL
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

double MITAB_DLL
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

double MITAB_DLL
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

int MITAB_DLL 
mitab_c_get_field_count( mitab_handle handle )
{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
    OGRFeatureDefn      *poDefn;

    if (poFile && (poDefn = poFile->GetLayerDefn()) != NULL)
        return poDefn->GetFieldCount();

    return 0;
}

/************************************************************************/
/*                       mitab_c_get_field_type()                       */
/************************************************************************/

/**
 * Return the type of an attribute fields in a datase's schemat.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field type, one of TABFT_Char (1), 
 *        TABFT_Integer (2), TABFT_SmallInt (3), TABFT_Decimal (4), 
 *        TABFT_Float (5), TABFT_Date (6), or TABFT_Logical (7)
 */

int MITAB_DLL 
mitab_c_get_field_type( mitab_handle handle, int field )
{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

    if (poFile)
        return poFile->GetNativeFieldType(field);

    return 0;
}

/************************************************************************/
/*                       mitab_c_get_field_name()                       */
/************************************************************************/

/**
 * Return the name of an attribute fields in a dataset's schema.
 *
 * @param handle the dataset's handle.
 * @param field the index of the field to look at, with 0 being the first 
 *        field.
 * @return the field name.  The returned string pointer is a reference to an
 *        internal buffer and should not be modified or freed by the caller.
 */

const char MITAB_DLL *
mitab_c_get_field_name( mitab_handle handle, int field )
{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;
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

const char MITAB_DLL *
mitab_c_get_field_as_string( mitab_feature feature, int field )
{
    TABFeature		*poFeature = (TABFeature *) feature;

    if (poFeature)
        return poFeature->GetFieldAsString(field);

    return "";
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
    TABFeature	*poFeature = (TABFeature *) feature;
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

    return -1;  /* Requested info could not be returned */
}

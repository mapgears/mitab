/**********************************************************************
 * $Id: mitab_capi.cpp,v 1.4 2000-01-15 22:30:43 daniel Exp $
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
 * Revision 1.4  2000-01-15 22:30:43  daniel
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

#include "mitab.h"
#include "mitab_capi.h"

/************************************************************************/
/*                       mitab_c_getlasterrorno()                       */
/************************************************************************/

int MITAB_STDCALL
mitab_c_getlasterrorno()

{
    return CPLGetLastErrorNo();
}

/************************************************************************/
/*                      mitab_c_getlasterrormsg()                       */
/************************************************************************/

const char MITAB_STDCALL *
mitab_c_getlasterrormsg()

{
    const char      *pszLastMessage = CPLGetLastErrorMsg();

    if( pszLastMessage == NULL )
        return "";
    else
        return pszLastMessage;
}

/************************************************************************/
/*                            mitab_c_open()                            */
/*                                                                      */
/*      Open an existing tab file for read access.                      */
/************************************************************************/

mitab_handle MITAB_STDCALL
mitab_c_open( const char * pszFilename )

{
    CPLSetErrorHandler( CPLQuietErrorHandler );
    return (mitab_handle) IMapInfoFile::SmartOpen( pszFilename );
}

/************************************************************************/
/*                           mitab_c_close()                            */
/************************************************************************/

void MITAB_STDCALL
mitab_c_close( mitab_handle handle )

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

void MITAB_STDCALL
mitab_c_destroy_feature( mitab_feature feature )

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

int MITAB_STDCALL
mitab_c_next_feature_id( mitab_handle handle, int last_feature_id )

{
    IMapInfoFile	*poFile = (IMapInfoFile *) handle;

    return poFile->GetNextFeatureId( last_feature_id );
}

/************************************************************************/
/*                        mitab_c_read_feature()                        */
/************************************************************************/

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

/************************************************************************/
/*                          mitab_c_set_text()                          */
/************************************************************************/

void MITAB_STDCALL
mitab_c_set_text( mitab_feature feature, const char * text )

{
    TABText	*poFeature = (TABText *) feature;

    if( poFeature->GetFeatureClass() == TABFC_Text )
        poFeature->SetTextString( text );
}

/************************************************************************/
/*                          mitab_c_set_text()                          */
/************************************************************************/

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

void MITAB_STDCALL
mitab_c_set_pen( mitab_feature feature,
                 int width, int pattern, int style, int color )

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
        poPen->SetPenWidth( width );
        poPen->SetPenPattern( pattern );
        poPen->SetPenStyle( style );
        poPen->SetPenColor( color );
    }
}

/************************************************************************/
/*                         mitab_c_set_symbol()                         */
/************************************************************************/

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


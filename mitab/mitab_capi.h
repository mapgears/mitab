/**********************************************************************
 * $Id: mitab_capi.h,v 1.5 2000-01-15 22:30:43 daniel Exp $
 *
 * Name:     mitab_capi.h
 * Project:  MapInfo TAB Read/Write library
 * Language: C
 * Purpose:  Simple C access API prototypes.  Suitable for limited access
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
 * $Log: mitab_capi.h,v $
 * Revision 1.5  2000-01-15 22:30:43  daniel
 * Switch to MIT/X-Consortium OpenSource license
 *
 * Revision 1.4  2000/01/14 21:58:40  warmerda
 * added error handling
 *
 * Revision 1.3  2000/01/14 17:30:39  warmerda
 * added capi dll support
 *
 * Revision 1.2  2000/01/14 16:33:24  warmerda
 * initial implementation complete
 *
 * Revision 1.1  2000/01/14 14:53:59  warmerda
 * New
 *
 */

#ifndef _MITAB_CAPI_H_INCLUDED_
#define _MITAB_CAPI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define MITAB_DLL __declspec(dllexport) /* __stdcall */
#define MITAB_STDCALL /* __stdcall */
#else
#define MITAB_DLL
#define MITAB_STDCALL
#endif
    

typedef void * mitab_handle;
typedef void * mitab_feature;

/* feature type values */
#define TABFC_Point	1
#define TABFC_Text	4
#define TABFC_Polyline	5
#define TABFC_Region    7

/* field types */
#define TABFT_Char	1
#define TABFT_Integer	2
#define TABFT_Float	5

const char MITAB_DLL *mitab_c_getlasterrormsg();
int MITAB_DLL mitab_c_getlasterrorno();
    
mitab_handle MITAB_DLL mitab_c_open( const char * filename );
void MITAB_DLL mitab_c_close( mitab_handle handle );

mitab_handle MITAB_DLL mitab_c_create( const char * filename,
                                       const char * mif_or_tab,
                                       const char * mif_projectiondef,
                                       double north, double south,
                                       double east, double west );

int MITAB_DLL mitab_c_add_field( mitab_handle handle, const char * field_name,
                                 int field_type, int width, int precision );

int MITAB_DLL
mitab_c_write_feature( mitab_handle handle, mitab_feature feature );

int MITAB_DLL
mitab_c_next_feature_id( mitab_handle handle, int last_feature_id );

mitab_feature MITAB_DLL
mitab_c_read_feature( mitab_handle handle, int feature_id );

void MITAB_DLL mitab_c_destroy_feature( mitab_feature );
int MITAB_DLL mitab_c_get_feature_id( mitab_feature );

mitab_feature MITAB_DLL
mitab_c_create_feature( mitab_handle, int feature_type );

void MITAB_DLL
mitab_c_set_field( mitab_feature, int field_index, const char * value );
    
void MITAB_DLL
mitab_c_set_text( mitab_feature feature, const char * text );
void MITAB_DLL
mitab_c_set_text_display( mitab_feature feature,
                          double angle, double height, double width,
                          int fg_color, int bg_color,
                          int justification, int spacing, int linetype );

void MITAB_DLL mitab_c_set_pen( mitab_feature feature,
                                int width, int pattern, int style, int color );

void MITAB_DLL mitab_c_set_brush( mitab_feature feature,
                                  int fg_color, int bg_color, int pattern,
                                  int transparent );
void MITAB_DLL
mitab_c_set_font( mitab_feature feature, const char * font_name );

void MITAB_DLL mitab_c_set_symbol( mitab_feature feature, int symbol_no,
                                   int symbol_size, int symbol_color );

void MITAB_DLL mitab_c_set_points( mitab_feature feature, int part,
                                   int vertex_count, double * x, double * y );

/* -------------------------------------------------------------------- */
/*      Not implemented                                                 */
/* -------------------------------------------------------------------- */
int mitab_c_get_type( mitab_feature feature );
int mitab_c_get_parts( mitab_feature feature );
int mitab_c_get_vertex_count( mitab_feature feature, int part );
double mitab_c_get_vertex_x( mitab_feature, int part, int vertex );
double mitab_c_get_vertex_y( mitab_feature, int part, int vertex );


#ifdef __cplusplus
}
#endif
    
#endif /* def _MITAB_CAPI_H_INCLUDED */

/**********************************************************************
 * $Id: mitab_capi.h,v 1.1 2000-01-14 14:53:59 warmerda Exp $
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
 * $Log: mitab_capi.h,v $
 * Revision 1.1  2000-01-14 14:53:59  warmerda
 * New
 *
 */

#ifndef _MITAB_CAPI_H_INCLUDED_
#define _MITAB_CAPI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
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

mitab_handle mitab_c_open( const char * filename );
void mitab_c_close( mitab_handle handle );

mitab_handle mitab_c_create( const char * filename,
                             const char * mif_or_tab,
                             const char * mif_projectiondef,
                             double north, double south,
                             double east, double west );

int mitab_c_add_field( mitab_handle handle, const char * field_name,
                       int field_type );

int mitab_c_write_feature( mitab_handle handle, mitab_feature feature );

int mitab_c_next_feature_id( mitab_handle handle, int last_feature_id );

mitab_feature mitab_c_read_feature( mitab_handle handle, int feature_id );

void mitab_c_destroy_feature( mitab_feature );
int mitab_c_get_feature_id( mitab_feature );

mitab_feature mitab_c_create_feature( mitab_handle, int feature_type );

void mitab_c_set_field( mitab_feature, int field_index, const char * value );

void mitab_c_set_text( mitab_feature, const char * text );

void mitab_c_set_pen( mitab_feature feature,
                      int width, int pattern, int style, int color );

void mitab_c_set_brush( mitab_feature feature,
                        int fg_color, int bg_color, int pattern,
                        int transparent );
void mitab_c_set_font( mitab_feature feature, const char * font_name );

void mitab_c_set_symbol( mitab_feature feature, int symbol_no,
                         int symbol_size, int symbol_color );

void mitab_c_set_points( mitab_feature feature, int part,
                         int vertex_count, double * x, double * y );

int mitab_c_get_type( mitab_feature feature );
int mitab_c_get_parts( mitab_feature feature );
int mitab_c_get_vertex_count( mitab_feature feature, int part );
double mitab_c_get_vertex_x( mitab_feature, int part, int vertex );
double mitab_c_get_vertex_y( mitab_feature, int part, int vertex );


#ifdef __cplusplus
}
#endif
    
#endif /* def _MITAB_CAPI_H_INCLUDED */

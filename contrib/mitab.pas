{**********************************************************************
 * $Id: mitab.pas,v 1.4 2002-08-01 13:53:11 daniel Exp $
 *
 * Name:     mitab.pas
 * Project:  MapInfo TAB Read/Write library
 * Language: Pascal
 * Purpose:  Pascal interface unit for MITAB API prototypes using STDCALL
 *           calling convention. Used with MITAB dll compiled with
 *           the /Gz qualifier.
 * Author:   Bo Thomsen, bvt@sns.dk
 *
 **********************************************************************
 * Copyright (c) 2002, Bo Thomsen
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
 * $Log: mitab.pas,v $
 * Revision 1.4  2002-08-01 13:53:11  daniel
 * Update for v1.2.2 from BVT
 *
 * Revision 1.4  2002/07/07 22:04:22  bvt
 * Added declaration for mitab_c_is_interior_ring
 *
 * Revision 1.3  2002/05/09 22:04:22  daniel
 * Added missing TABFC_MultiPoint=10
 *
 * Revision 1.2  2002/05/08 20:23:53  daniel
 * Update from Bo, and re-re-renamed mitab_vb.dll to mitab.dll (stdcall is default now)
 *
 * Revision 1.21  2002/05/07 13:43:30  bvt
 * changed the dll name to mitab_vb.dll. Changed the pas file name
 * to mitab_vb.pas to reflect the new name of the dll.
 *
 * Revision 1.2  2002/05/05 13:43:30  bvt
 * Added mitab_c_get_field_width and mitab_c_get_field_precision to api.
 *
 * Revision 1.1  2002/02/22 13:43:30  daniel
 * Initial Revision from Bo Thomsen
 *
 * Revision 1.00  2002/02/20 12:35:00  bvt
 * New - used in conjunction with mitab_capi.cpp,v 1.15 compiled with
 * /Gz (stdcall calling convention)
 *
 *}

unit Mitab;

interface

Type

  mitab_handle = Longword;
  mitab_feature = Longword;
  mitab_projinfo = Longword;

const

// feature type values
  TABFC_NoGeom      = 0;
  TABFC_Point       = 1;
  TABFC_FontPoint   = 2;
  TABFC_CustomPoint = 3;
  TABFC_Text        = 4;
  TABFC_Polyline    = 5;
  TABFC_Arc         = 6;
  TABFC_Region      = 7;
  TABFC_Rectangle   = 8;
  TABFC_Ellipse     = 9;
  TABFC_MultiPoint  = 10;

// field types
  TABFT_Char        = 1;
  TABFT_Integer     = 2;
  TABFT_SmallInt    = 3;
  TABFT_Decimal     = 4;
  TABFT_Float       = 5;
  TABFT_Date        = 6;
  TABFT_Logical     = 7;

// text justification
  TABTJ_Left        = 0;
  TABTJ_Center      = 1;
  TABTJ_Right       = 2;

// text spacing
  TABTS_Single      = 0;
  TABTS_1_5         = 1;
  TABTS_Double      = 2;

// test linetype
  TABTL_NoLine      = 0;
  TABTL_Simple      = 1;
  TABTL_Arrow       = 2;

const

  DLL = 'mitab.dll';

function  mitab_c_add_field              (handle: mitab_handle; field_name: pchar;field_type, width, precision: longint): longint;                              stdcall; external DLL Name '_mitab_c_add_field@20'            ;
procedure mitab_c_close                  (handle: mitab_handle);                                                                                                stdcall; external DLL Name '_mitab_c_close@4'                 ;
function  mitab_c_create                 (filename, mif_or_tab, mif_projectiondef: pchar; north, south, east, west: double): mitab_handle;                      stdcall; external DLL Name '_mitab_c_create@44'               ;
function  mitab_c_create_feature         (handle: mitab_handle; feature_type: longint): mitab_feature;                                                          stdcall; external DLL Name '_mitab_c_create_feature@8'        ;
procedure mitab_c_destroy_feature        (feature: mitab_feature);                                                                                              stdcall; external DLL Name '_mitab_c_destroy_feature@4'       ;
function  mitab_c_get_brush_bgcolor      (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_brush_bgcolor@4'     ;
function  mitab_c_get_brush_fgcolor      (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_brush_fgcolor@4'     ;
function  mitab_c_get_brush_pattern      (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_brush_pattern@4'     ;
function  mitab_c_get_brush_transparent  (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_brush_transparent@4' ;
function  mitab_c_get_field_as_string_vb (feature: mitab_feature; field: longint; value: pchar; l: longint): longint;                                           stdcall; external DLL Name '_mitab_c_get_field_as_string_vb@16';
function  mitab_c_get_field_as_string    (feature: mitab_feature; field: longint): pchar;                                                                       stdcall; external DLL Name '_mitab_c_get_field_as_string@8'   ;
function  mitab_c_get_field_count        (handle: mitab_handle): longint;                                                                                       stdcall; external DLL Name '_mitab_c_get_field_count@4'       ;
function  mitab_c_get_field_name_vb      (handle: mitab_handle; field: longint; name: pchar; l: longint): longint;						stdcall; external DLL Name '_mitab_c_get_field_name_vb@16'    ;
function  mitab_c_get_field_name         (handle: mitab_handle; field: longint): pchar;                                                                         stdcall; external DLL Name '_mitab_c_get_field_name@8'        ;
function  mitab_c_get_field_type         (handle: mitab_handle; field: longint): longint;                                                                       stdcall; external DLL Name '_mitab_c_get_field_type@8'        ;
function  mitab_c_get_font_vb            (feature: mitab_feature; font: pchar; l: longint): longint;                                                            stdcall; external DLL Name '_mitab_c_get_font_vb@12'          ;
function  mitab_c_get_font               (feature: mitab_feature): pchar;                                                                                       stdcall; external DLL Name '_mitab_c_get_font@4'              ;
function  mitab_c_get_mif_coordsys_vb    (dataset: mitab_handle; coordsys: pchar; l: longint): longint;								stdcall; external DLL Name '_mitab_c_get_mif_coordsys_vb@12'  ;
function  mitab_c_get_mif_coordsys       (dataset: mitab_handle): pchar;                                                                                        stdcall; external DLL Name '_mitab_c_get_mif_coordsys@4'      ;
function  mitab_c_get_parts              (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_parts@4'             ;
function  mitab_c_get_pen_color          (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_pen_color@4'         ;
function  mitab_c_get_pen_pattern        (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_pen_pattern@4'       ;
function  mitab_c_get_pen_width          (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_pen_width@4'         ;
function  mitab_c_get_projinfo           (dataset: mitab_handle): mitab_projinfo;                                                                               stdcall; external DLL Name '_mitab_c_get_projinfo@4'          ;
function  mitab_c_get_symbol_color       (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_symbol_color@4'      ;
function  mitab_c_get_symbol_no          (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_symbol_no@4'         ;
function  mitab_c_get_symbol_size        (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_symbol_size@4'       ;
function  mitab_c_get_text_vb            (feature: mitab_feature; text: pchar; l: longint): longint;                                                            stdcall; external DLL Name '_mitab_c_get_text_vb@12'          ;
function  mitab_c_get_text               (feature: mitab_feature): pchar;                                                                                       stdcall; external DLL Name '_mitab_c_get_text@4'              ;
function  mitab_c_get_text_angle         (feature: mitab_feature): double;                                                                                      stdcall; external DLL Name '_mitab_c_get_text_angle@4'        ;
function  mitab_c_get_text_bgcolor       (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_text_bgcolor@4'      ;
function  mitab_c_get_text_fgcolor       (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_text_fgcolor@4'      ;
function  mitab_c_get_text_height        (feature: mitab_feature): double;                                                                                      stdcall; external DLL Name '_mitab_c_get_text_height@4'       ;
function  mitab_c_get_text_justification (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_text_justification@4';
function  mitab_c_get_text_linetype      (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_text_linetype@4'     ;
function  mitab_c_get_text_spacing       (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_text_spacing@4'      ;
function  mitab_c_get_text_width         (feature: mitab_feature): double;                                                                                      stdcall; external DLL Name '_mitab_c_get_text_width@4'        ;
function  mitab_c_get_type               (feature: mitab_feature): longint;                                                                                     stdcall; external DLL Name '_mitab_c_get_type@4'              ;
function  mitab_c_get_vertex_count       (feature: mitab_feature; part: longint): longint;                                                                      stdcall; external DLL Name '_mitab_c_get_vertex_count@8'      ;
function  mitab_c_get_vertex_x           (feature: mitab_feature; part, vertex: longint): double;                                                               stdcall; external DLL Name '_mitab_c_get_vertex_x@12'         ;
function  mitab_c_get_vertex_y           (feature: mitab_feature; part, vertex: longint): double;                                                               stdcall; external DLL Name '_mitab_c_get_vertex_y@12'         ;
function  mitab_c_getlasterrormsg_vb     (errormsg: pchar; l: longint): longint;                                                                                stdcall; external DLL Name '_mitab_c_getlasterrormsg_vb@8'    ;
function  mitab_c_getlasterrormsg        (): pchar;                                                                                                             stdcall; external DLL Name '_mitab_c_getlasterrormsg@0'       ;
function  mitab_c_getlasterrorno         (): longint;                                                                                                           stdcall; external DLL Name '_mitab_c_getlasterrorno@0'        ;
function  mitab_c_next_feature_id        (handle: mitab_handle; last_feature_id: longint): longint;                                                             stdcall; external DLL Name '_mitab_c_next_feature_id@8'       ;
function  mitab_c_open                   (filename: pchar): mitab_handle;                                                                                       stdcall; external DLL Name '_mitab_c_open@4'                  ;
function  mitab_c_read_feature           (handle:  mitab_handle; feature_id: longint): mitab_feature;                                                           stdcall; external DLL Name '_mitab_c_read_feature@8'          ;
procedure mitab_c_set_arc                (feature: mitab_feature; center_x, center_y, x_radius, y_radius, start_angle, end_angle: double);                      stdcall; external DLL Name '_mitab_c_set_arc@52'              ;
procedure mitab_c_set_brush              (feature: mitab_feature; fg_color, bg_color, pattern, transparent: longint);                                           stdcall; external DLL Name '_mitab_c_set_brush@20'            ;
procedure mitab_c_set_field              (feature: mitab_feature; field_index: longint; value: pchar);                                                          stdcall; external DLL Name '_mitab_c_set_field@12'            ;
procedure mitab_c_set_font               (feature: mitab_feature; font_name: pchar);                                                                            stdcall; external DLL Name '_mitab_c_set_font@8'              ;
procedure mitab_c_set_pen                (feature: mitab_feature; width, pattern, color: longint);                                                              stdcall; external DLL Name '_mitab_c_set_pen@16'              ;
procedure mitab_c_set_points             (feature: mitab_feature; part, vertex_count: longint; var x,y: double);                                                stdcall; external DLL Name '_mitab_c_set_points@20'           ;
function  mitab_c_set_projinfo           (dataset: mitab_handle; projinfo: mitab_projinfo): longint;                                                            stdcall; external DLL Name '_mitab_c_set_projinfo@8'          ;
procedure mitab_c_set_symbol             (feature: mitab_feature; symbol_no, symbol_size, symbol_color: longint);                                               stdcall; external DLL Name '_mitab_c_set_symbol@16'           ;
procedure mitab_c_set_text               (feature: mitab_feature; text: pchar);                                                                                 stdcall; external DLL Name '_mitab_c_set_text@8'              ;
procedure mitab_c_set_text_display       (feature: mitab_feature; angle, height, width: double; fg_color, bg_color, justification, spacing, linetype: longint); stdcall; external DLL Name '_mitab_c_set_text_display@48'     ;
function  mitab_c_write_feature          (handle: mitab_handle; feature: mitab_feature): longint;                                                               stdcall; external DLL Name '_mitab_c_write_feature@8'         ;
function  mitab_c_get_field_width        (handle: mitab_handle; field: longint): longint;                                                                       stdcall; external DLL Name '_mitab_c_get_field_width@8'       ;
function  mitab_c_get_field_precision    (handle: mitab_handle; field: longint): longint;                                                                       stdcall; external DLL Name '_mitab_c_get_field_precision@8'   ;
function  mitab_c_is_interior_ring       (feature: mitab_feature; requestedringindex: longint): longint;                                                        stdcall; external DLL Name '_mitab_c_is_interior_ring@8'      ;

implementation
end.


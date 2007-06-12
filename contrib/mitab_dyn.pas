{**********************************************************************
 * $Id: mitab_dyn.pas,v 1.13 2007-06-12 14:49:34 dmorissette Exp $
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
 * $Log: mitab_dyn.pas,v $
 * Revision 1.13  2007-06-12 14:49:34  dmorissette
 * Upgrade to Libversion = 1006002 (no API changes)
 *
 * Revision 1.12  2007/04/16 14:58:29  dmorissette
 * Added missing mitab_c_set_quick_spatial_index_mode defn in 1.6.1 (Uffe)
 *
 * Revision 1.11  2007/03/22 21:01:37  dmorissette
 * Update for v1.6.1
 *
 * Revision 1.10  2006/12/04 20:45:36  dmorissette
 * Upgrade for MITAB 1.6.0 from Uffe (bug 1621)
 *
 * Revision 1.9  2006/07/25 13:24:47  dmorissette
 * Updated for 1.5.1 release
 *
 * Revision 1.8  2006/02/16 15:13:38  dmorissette
 * Updated for 1.5.0 release
 *
 * Revision 1.7  2005/03/24 17:21:05  dmorissette
 * Updated Libversion to 1004000 for 1.4.0 release
 *
 * Revision 1.6  2004/07/05 18:45:24  dmorissette
 * Added mitab_c_load_coordsys_table defn (Uffe K.)
 *
 * Revision 1.5  2004/06/30 20:18:53  dmorissette
 * Update from Uffe K. for V1.3.0 release
 *
 * Revision 1.4  2003/08/07 03:24:30  dmorissette
 * Added function for checking of version of library (Uffe K. - bug 21)
 *
 * Revision 1.25  2003/08/06 22:50:00  uffe
 * Added function for checking of version of library
 *
 * Revision 1.24  2003/07/25  9:45:00  uffe
 * Updated to version 1.2.4 (mitab_c_add_field, now with correct function name)
 *
 * Revision 1.1  2002/08/01 13:56:01  daniel
 * Contributed by Uffe K. - header for dynamic loading of mitab.dll in Delphi
 *
 * Revision 1.22  2002/07/07 11:20:00  uffe
 * Updated to version 1.22 (mitab_c_is_interior_ring)
 *
 * Revision 1.21  2002/05/09  9:36:00  uffe
 * Changed to allow dynamic loading of DLL
 * Added subrange type definitions for constants
 *
 * Revision 1.21  2002/05/08 20:23:53  daniel
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

unit mitab_dyn;

interface

// This define is missing in D5 and before
{$IFDEF VER130}
  {$DEFINE MSWINDOWS}
{$ENDIF}

Type
  mitab_handle = Longword;
  mitab_feature = Longword;
  mitab_projinfo = Longword;
  TABFC = 0..10;
  TABFT = 1..7;
  TABTJ = 0..2;
  TABTS = 0..2;
  TABTL = 0..2;
  TadProjParams = array [1..6] of double;
  TadDatumParams = array [1..5] of double;
  TadAffineParams = array [1..6] of double;

const
// update to match mitab.h (app. line 97), when new versions are released
  Libversion = 1006002;
  
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
  TABFC_Multipoint  = 10 ; //1.2.0

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

type
  Tmitab_c_add_field              = function(handle: mitab_handle; field_name: pchar; field_type, width, precision, indexed, unique: longint): longint; stdcall;
  Tmitab_c_close                  = procedure(handle: mitab_handle); stdcall;
  Tmitab_c_create                 = function(filename, mif_or_tab, mif_projectiondef: pchar; north, south, east, west: double): mitab_handle; stdcall;
  Tmitab_c_create_feature         = function(handle: mitab_handle; feature_type: longint): mitab_feature; stdcall;
  Tmitab_c_destroy_feature        = procedure(feature: mitab_feature); stdcall;
  Tmitab_c_getlasterrormsg        = function(): pchar; stdcall;
  Tmitab_c_getlasterrormsg_vb     = function(errormsg: pchar; l: longint): longint; stdcall;
  Tmitab_c_getlasterrorno         = function(): longint; stdcall;
  Tmitab_c_getlibversion          = function(): longint; stdcall;
  Tmitab_c_get_brush_bgcolor      = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_brush_fgcolor      = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_brush_pattern      = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_brush_transparent  = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_extended_mif_coordsys = function(dataset: mitab_handle): pchar; stdcall;
  Tmitab_c_get_feature_count      = function(feature: mitab_handle): longint; stdcall;
  Tmitab_c_get_field_as_double    = function(feature: mitab_feature; field: longint): double; stdcall;
  Tmitab_c_get_field_as_string    = function(feature: mitab_feature; field: longint): pchar; stdcall;
  Tmitab_c_get_field_as_string_vb = function(feature: mitab_feature; field: longint; value: pchar; l: longint): longint; stdcall;
  Tmitab_c_get_field_count        = function(handle: mitab_handle): longint; stdcall;
  Tmitab_c_get_field_name         = function(handle: mitab_handle; field: longint): pchar; stdcall;
  Tmitab_c_get_field_name_vb      = function(handle: mitab_handle; field: longint; name: pchar; l: longint): longint; stdcall;
  Tmitab_c_get_field_precision    = function(handle: mitab_handle; field: longint): longint; stdcall;
  Tmitab_c_get_field_type         = function(handle: mitab_handle; field: longint): longint; stdcall;
  Tmitab_c_get_field_width        = function(handle: mitab_handle; field: longint): longint; stdcall;
  Tmitab_c_get_font               = function(feature: mitab_feature): pchar; stdcall;
  Tmitab_c_get_font_vb            = function(feature: mitab_feature; font: pchar; l: longint): longint; stdcall;
  Tmitab_c_get_mif_coordsys       = function(dataset: mitab_handle): pchar; stdcall;
  Tmitab_c_get_mif_coordsys_vb    = function(dataset: mitab_handle; coordsys: pchar; l: longint): longint; stdcall;
  Tmitab_c_get_parts              = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_pen_color          = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_pen_pattern        = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_pen_width          = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_projinfo           = function(dataset: mitab_handle): mitab_projinfo; stdcall;
  Tmitab_c_get_symbol_color       = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_symbol_no          = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_symbol_size        = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text               = function(feature: mitab_feature): pchar; stdcall;
  Tmitab_c_get_text_angle         = function(feature: mitab_feature): double; stdcall;
  Tmitab_c_get_text_bgcolor       = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text_fgcolor       = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text_height        = function(feature: mitab_feature): double; stdcall;
  Tmitab_c_get_text_justification = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text_linetype      = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text_spacing       = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_text_vb            = function(feature: mitab_feature; text: pchar; l: longint): longint; stdcall;
  Tmitab_c_get_text_width         = function(feature: mitab_feature): double; stdcall;
  Tmitab_c_get_type               = function(feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_vertex_count       = function(feature: mitab_feature; part: longint): longint; stdcall;
  Tmitab_c_get_vertex_x           = function(feature: mitab_feature; part, vertex: longint): double; stdcall;
  Tmitab_c_get_vertex_y           = function(feature: mitab_feature; part, vertex: longint): double; stdcall;
  Tmitab_c_is_interior_ring       = function(feature: mitab_feature; requestedringindex: longint): longint; stdcall;
  Tmitab_c_load_coordsys_table    = function(filename: pchar): longint; stdcall;
  Tmitab_c_next_feature_id        = function(handle: mitab_handle; last_feature_id: longint): longint; stdcall;
  Tmitab_c_open                   = function(filename: pchar): mitab_handle; stdcall;
  Tmitab_c_read_feature           = function(handle:  mitab_handle; feature_id: longint): mitab_feature; stdcall;
  Tmitab_c_set_arc                = procedure(feature: mitab_feature; center_x, center_y, x_radius, y_radius, start_angle, end_angle: double); stdcall;
  Tmitab_c_set_brush              = procedure(feature: mitab_feature; fg_color, bg_color, pattern, transparent: longint); stdcall;
  Tmitab_c_set_field              = procedure(feature: mitab_feature; field_index: longint; value: pchar); stdcall;
  Tmitab_c_set_font               = procedure(feature: mitab_feature; font_name: pchar); stdcall;
  Tmitab_c_set_pen                = procedure(feature: mitab_feature; width, pattern, color: longint); stdcall;
  Tmitab_c_set_points             = procedure(feature: mitab_feature; part, vertex_count: longint; var x,y: double); stdcall;
  Tmitab_c_set_projinfo           = function(dataset: mitab_handle; projinfo: mitab_projinfo): longint; stdcall;
  Tmitab_c_set_symbol             = procedure(feature: mitab_feature; symbol_no, symbol_size, symbol_color: longint); stdcall;
  Tmitab_c_set_text               = procedure(feature: mitab_feature; text: pchar); stdcall;
  Tmitab_c_set_text_display       = procedure(feature: mitab_feature; angle, height, width: double; fg_color, bg_color, justification, spacing, linetype: longint); stdcall;
  Tmitab_c_write_feature          = function(handle: mitab_handle; feature: mitab_feature): longint; stdcall;
  Tmitab_c_get_collection_region_ref      = function(feature: mitab_feature): mitab_feature; stdcall;
  Tmitab_c_get_collection_polyline_ref    = function(feature: mitab_feature): mitab_feature; stdcall;
  Tmitab_c_get_collection_multipoint_ref  = function(feature: mitab_feature): mitab_feature; stdcall;
  Tmitab_c_set_collection_region          = function(feature,region: mitab_feature; make_copy: integer): longint; stdcall;
  Tmitab_c_set_collection_polyline        = function(feature,polyline: mitab_feature; make_copy: integer): longint; stdcall;
  Tmitab_c_set_collection_multipoint      = function(feature,multipoint: mitab_feature; make_copy: integer): longint; stdcall;
  Tmitab_c_get_projection_info            = procedure(projInfo: mitab_projinfo; var nProjId,nEllipsoidId,nUnitsId: longint; var adProjParams: TadProjParams); stdcall;
  Tmitab_c_set_projection_info            = procedure(projInfo: mitab_projinfo; nProjId,nEllipsoidId,nUnitsId: longint; var adProjParams: TadProjParams); stdcall;
  Tmitab_c_get_datum_info                 = procedure(projInfo: mitab_projinfo; var dDatumShiftX,dDatumShiftY,dDatumShiftZ: double; var adDatumParams: TadDatumParams); stdcall;
  Tmitab_c_set_datum_info                 = procedure(projInfo: mitab_projinfo; dDatumShiftX,dDatumShiftY,dDatumShiftZ: double; var adDatumParams: TadDatumParams); stdcall;
  Tmitab_c_get_affine_params              = function(projInfo: mitab_projinfo; var nAffineUnits: longint; var adAffineParams: TadAffineParams): longint; stdcall;
  Tmitab_c_set_affine_params              = function(projInfo: mitab_projinfo; nAffineUnits: longint; var adAffineParams: TadAffineParams): longint; stdcall;
  Tmitab_c_is_field_indexed               = function(handle: mitab_handle; field: longint): longint; stdcall;
  Tmitab_c_is_field_unique                = function(handle: mitab_handle; field: longint): longint; stdcall;
  Tmitab_c_set_quick_spatial_index_mode   = function(handle: mitab_handle): longint; stdcall;
    
var
  MITAB_LibOK: boolean;   // false if DLL isn't loaded, true if loaded OK and correct version
  LibPath: string;        // Full path to the MITAB DLL, default value is just 'mitab.dll'

  mitab_c_add_field: Tmitab_c_add_field;
  mitab_c_close: Tmitab_c_close;
  mitab_c_create: Tmitab_c_create;
  mitab_c_create_feature: Tmitab_c_create_feature;
  mitab_c_destroy_feature: Tmitab_c_destroy_feature;
  mitab_c_getlasterrormsg: Tmitab_c_getlasterrormsg;
  mitab_c_getlasterrormsg_vb: Tmitab_c_getlasterrormsg_vb;
  mitab_c_getlasterrorno: Tmitab_c_getlasterrorno;
  mitab_c_getlibversion: Tmitab_c_getlibversion;
  mitab_c_get_brush_bgcolor: Tmitab_c_get_brush_bgcolor;
  mitab_c_get_brush_fgcolor: Tmitab_c_get_brush_fgcolor;
  mitab_c_get_brush_pattern: Tmitab_c_get_brush_pattern;
  mitab_c_get_brush_transparent: Tmitab_c_get_brush_transparent;
  mitab_c_get_extended_mif_coordsys: Tmitab_c_get_extended_mif_coordsys;
  mitab_c_get_feature_count: Tmitab_c_get_feature_count;
  mitab_c_get_field_as_double: Tmitab_c_get_field_as_double;
  mitab_c_get_field_as_string: Tmitab_c_get_field_as_string;
  mitab_c_get_field_as_string_vb: Tmitab_c_get_field_as_string_vb;
  mitab_c_get_field_count: Tmitab_c_get_field_count;
  mitab_c_get_field_name: Tmitab_c_get_field_name;
  mitab_c_get_field_name_vb: Tmitab_c_get_field_name_vb;
  mitab_c_get_field_precision: Tmitab_c_get_field_precision;
  mitab_c_get_field_type: Tmitab_c_get_field_type;
  mitab_c_get_field_width: Tmitab_c_get_field_width;
  mitab_c_get_font: Tmitab_c_get_font;
  mitab_c_get_font_vb: Tmitab_c_get_font_vb;
  mitab_c_get_mif_coordsys: Tmitab_c_get_mif_coordsys;
  mitab_c_get_mif_coordsys_vb: Tmitab_c_get_mif_coordsys_vb;
  mitab_c_get_parts: Tmitab_c_get_parts;
  mitab_c_get_pen_color: Tmitab_c_get_pen_color;
  mitab_c_get_pen_pattern: Tmitab_c_get_pen_pattern;
  mitab_c_get_pen_width: Tmitab_c_get_pen_width;
  mitab_c_get_projinfo: Tmitab_c_get_projinfo;
  mitab_c_get_symbol_color: Tmitab_c_get_symbol_color;
  mitab_c_get_symbol_no: Tmitab_c_get_symbol_no;
  mitab_c_get_symbol_size: Tmitab_c_get_symbol_size;
  mitab_c_get_text: Tmitab_c_get_text;
  mitab_c_get_text_angle: Tmitab_c_get_text_angle;
  mitab_c_get_text_bgcolor: Tmitab_c_get_text_bgcolor;
  mitab_c_get_text_fgcolor: Tmitab_c_get_text_fgcolor;
  mitab_c_get_text_height: Tmitab_c_get_text_height;
  mitab_c_get_text_justification: Tmitab_c_get_text_justification;
  mitab_c_get_text_linetype: Tmitab_c_get_text_linetype;
  mitab_c_get_text_spacing: Tmitab_c_get_text_spacing;
  mitab_c_get_text_vb: Tmitab_c_get_text_vb;
  mitab_c_get_text_width: Tmitab_c_get_text_width;
  mitab_c_get_type: Tmitab_c_get_type;
  mitab_c_get_vertex_count: Tmitab_c_get_vertex_count;
  mitab_c_get_vertex_x: Tmitab_c_get_vertex_x;
  mitab_c_get_vertex_y: Tmitab_c_get_vertex_y;
  mitab_c_is_interior_ring: Tmitab_c_is_interior_ring;
  mitab_c_load_coordsys_table: Tmitab_c_load_coordsys_table;
  mitab_c_next_feature_id: Tmitab_c_next_feature_id;
  mitab_c_open: Tmitab_c_open;
  mitab_c_read_feature: Tmitab_c_read_feature;
  mitab_c_set_arc: Tmitab_c_set_arc;
  mitab_c_set_brush: Tmitab_c_set_brush;
  mitab_c_set_field: Tmitab_c_set_field;
  mitab_c_set_font: Tmitab_c_set_font;
  mitab_c_set_pen: Tmitab_c_set_pen;
  mitab_c_set_points: Tmitab_c_set_points;
  mitab_c_set_projinfo: Tmitab_c_set_projinfo;
  mitab_c_set_symbol: Tmitab_c_set_symbol;
  mitab_c_set_text: Tmitab_c_set_text;
  mitab_c_set_text_display: Tmitab_c_set_text_display;
  mitab_c_write_feature: Tmitab_c_write_feature;
  mitab_c_get_collection_region_ref: Tmitab_c_get_collection_region_ref;
  mitab_c_get_collection_polyline_ref: Tmitab_c_get_collection_polyline_ref;
  mitab_c_get_collection_multipoint_ref: Tmitab_c_get_collection_multipoint_ref;
  mitab_c_set_collection_region: Tmitab_c_set_collection_region;
  mitab_c_set_collection_polyline: Tmitab_c_set_collection_polyline;
  mitab_c_set_collection_multipoint: Tmitab_c_set_collection_multipoint;
  mitab_c_get_projection_info: Tmitab_c_get_projection_info;
  mitab_c_set_projection_info: Tmitab_c_set_projection_info;
  mitab_c_get_datum_info: Tmitab_c_get_datum_info;
  mitab_c_set_datum_info: Tmitab_c_set_datum_info;
  mitab_c_get_affine_params: Tmitab_c_get_affine_params;
  mitab_c_set_affine_params: Tmitab_c_set_affine_params;
  mitab_c_is_field_indexed: Tmitab_c_is_field_indexed;
  mitab_c_is_field_unique: Tmitab_c_is_field_unique;
  mitab_c_set_quick_spatial_index_mode: Tmitab_c_set_quick_spatial_index_mode;


// This allows compilation with Kylix. Later there may be a "mitab.so" ? 
// On Kylix MITAB_LibOK will always be false and MITAB_load does nothing

procedure MITAB_load;

implementation

{$IFDEF MSwindows}
uses
  windows;

var
  MITABDLL_Handle: THandle;
{$ENDIF}

// To test if a function was found in the DLL before using it, do like this:
// if @mitab_c_create<>nil then mitab_c_create(.....)

procedure MITAB_load;
begin
  {$IFDEF MSwindows}
  MITABDLL_Handle:= LoadLibrary(pchar(LibPath));
  if MITABDLL_Handle<>0 then
  begin
    @mitab_c_getlibversion:= GetProcAddress(MITABDLL_Handle,'_mitab_c_getlibversion@0');
    if @mitab_c_getlibversion<>nil then  // does the function exist?
    begin
      if mitab_c_getlibversion=Libversion then  // does it return the correct number?
      begin
        MITAB_LibOK:= true;
        @mitab_c_add_field:=              GetProcAddress(MITABDLL_Handle,'_mitab_c_add_field@28');
        @mitab_c_close:=                  GetProcAddress(MITABDLL_Handle,'_mitab_c_close@4');
        @mitab_c_create:=                 GetProcAddress(MITABDLL_Handle,'_mitab_c_create@44');
        @mitab_c_create_feature:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_create_feature@8');
        @mitab_c_destroy_feature:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_destroy_feature@4');
        @mitab_c_getlasterrormsg:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_getlasterrormsg@0');
        @mitab_c_getlasterrormsg_vb:=     GetProcAddress(MITABDLL_Handle,'_mitab_c_getlasterrormsg_vb@8');
        @mitab_c_getlasterrorno:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_getlasterrorno@0');
        @mitab_c_get_brush_bgcolor:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_brush_bgcolor@4');
        @mitab_c_get_brush_fgcolor:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_brush_fgcolor@4');
        @mitab_c_get_brush_pattern:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_brush_pattern@4');
        @mitab_c_get_brush_transparent:=  GetProcAddress(MITABDLL_Handle,'_mitab_c_get_brush_transparent@4');
        @mitab_c_get_extended_mif_coordsys:= GetProcAddress(MITABDLL_Handle,'_mitab_c_get_extended_mif_coordsys@4');
        @mitab_c_get_feature_count:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_feature_count@4');
        @mitab_c_get_field_as_double:=    GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_as_double@8');
        @mitab_c_get_field_as_string:=    GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_as_string@8');
        @mitab_c_get_field_as_string_vb:= GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_as_string_vb@16');
        @mitab_c_get_field_count:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_count@4');
        @mitab_c_get_field_name:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_name@8');
        @mitab_c_get_field_name_vb:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_name_vb@16');
        @mitab_c_get_field_precision:=    GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_precision@8');
        @mitab_c_get_field_type:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_type@8');
        @mitab_c_get_field_width:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_get_field_width@8');
        @mitab_c_get_font:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_get_font@4');
        @mitab_c_get_font_vb:=            GetProcAddress(MITABDLL_Handle,'_mitab_c_get_font_vb@12');
        @mitab_c_get_mif_coordsys:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_mif_coordsys@4');
        @mitab_c_get_mif_coordsys_vb:=    GetProcAddress(MITABDLL_Handle,'_mitab_c_get_mif_coordsys_vb@12');
        @mitab_c_get_parts:=              GetProcAddress(MITABDLL_Handle,'_mitab_c_get_parts@4');
        @mitab_c_get_pen_color:=          GetProcAddress(MITABDLL_Handle,'_mitab_c_get_pen_color@4');
        @mitab_c_get_pen_pattern:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_get_pen_pattern@4');
        @mitab_c_get_pen_width:=          GetProcAddress(MITABDLL_Handle,'_mitab_c_get_pen_width@4');
        @mitab_c_get_projinfo:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_get_projinfo@4');
        @mitab_c_get_symbol_color:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_symbol_color@4');
        @mitab_c_get_symbol_no:=          GetProcAddress(MITABDLL_Handle,'_mitab_c_get_symbol_no@4');
        @mitab_c_get_symbol_size:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_get_symbol_size@4');
        @mitab_c_get_text:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text@4');
        @mitab_c_get_text_angle:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_angle@4');
        @mitab_c_get_text_bgcolor:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_bgcolor@4');
        @mitab_c_get_text_fgcolor:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_fgcolor@4');
        @mitab_c_get_text_height:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_height@4');
        @mitab_c_get_text_justification:= GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_justification@4');
        @mitab_c_get_text_linetype:=      GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_linetype@4');
        @mitab_c_get_text_spacing:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_spacing@4');
        @mitab_c_get_text_vb:=            GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_vb@12');
        @mitab_c_get_text_width:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_get_text_width@4');
        @mitab_c_get_type:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_get_type@4');
        @mitab_c_get_vertex_count:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_get_vertex_count@8');
        @mitab_c_get_vertex_x:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_get_vertex_x@12');
        @mitab_c_get_vertex_y:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_get_vertex_y@12');
        @mitab_c_is_interior_ring:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_is_interior_ring@8');
        @mitab_c_load_coordsys_table:=    GetProcAddress(MITABDLL_Handle,'_mitab_c_load_coordsys_table@4');
        @mitab_c_next_feature_id:=        GetProcAddress(MITABDLL_Handle,'_mitab_c_next_feature_id@8');
        @mitab_c_open:=                   GetProcAddress(MITABDLL_Handle,'_mitab_c_open@4');
        @mitab_c_read_feature:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_read_feature@8');
        @mitab_c_set_arc:=                GetProcAddress(MITABDLL_Handle,'_mitab_c_set_arc@52');
        @mitab_c_set_brush:=              GetProcAddress(MITABDLL_Handle,'_mitab_c_set_brush@20');
        @mitab_c_set_field:=              GetProcAddress(MITABDLL_Handle,'_mitab_c_set_field@12');
        @mitab_c_set_font:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_set_font@8');
        @mitab_c_set_pen:=                GetProcAddress(MITABDLL_Handle,'_mitab_c_set_pen@16');
        @mitab_c_set_points:=             GetProcAddress(MITABDLL_Handle,'_mitab_c_set_points@20');
        @mitab_c_set_projinfo:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_set_projinfo@8');
        @mitab_c_set_symbol:=             GetProcAddress(MITABDLL_Handle,'_mitab_c_set_symbol@16');
        @mitab_c_set_text:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_set_text@8');
        @mitab_c_set_text_display:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_set_text_display@48');
        @mitab_c_write_feature:=          GetProcAddress(MITABDLL_Handle,'_mitab_c_write_feature@8');
        @mitab_c_get_collection_region_ref:=     GetProcAddress(MITABDLL_Handle,'_mitab_c_get_collection_region_ref@4');
        @mitab_c_get_collection_polyline_ref:=   GetProcAddress(MITABDLL_Handle,'_mitab_c_get_collection_polyline_ref@4');
        @mitab_c_get_collection_multipoint_ref:= GetProcAddress(MITABDLL_Handle,'_mitab_c_get_collection_multipoint_ref@4');
        @mitab_c_set_collection_region:=         GetProcAddress(MITABDLL_Handle,'_mitab_c_set_collection_region@12');
        @mitab_c_set_collection_polyline:=       GetProcAddress(MITABDLL_Handle,'_mitab_c_set_collection_polyline@12');
        @mitab_c_set_collection_multipoint:=     GetProcAddress(MITABDLL_Handle,'_mitab_c_set_collection_multipoint@12');
        @mitab_c_get_projection_info:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_get_projection_info@20');
        @mitab_c_set_projection_info:=           GetProcAddress(MITABDLL_Handle,'_mitab_c_set_projection_info@20');
        @mitab_c_get_datum_info:=                GetProcAddress(MITABDLL_Handle,'_mitab_c_get_datum_info@20');
        @mitab_c_set_datum_info:=                GetProcAddress(MITABDLL_Handle,'_mitab_c_set_datum_info@32');
        @mitab_c_get_affine_params:=             GetProcAddress(MITABDLL_Handle,'_mitab_c_get_affine_params@12');
        @mitab_c_set_affine_params:=             GetProcAddress(MITABDLL_Handle,'_mitab_c_set_affine_params@12');
        @mitab_c_is_field_indexed:=              GetProcAddress(MITABDLL_Handle,'_mitab_c_is_field_indexed@8');
        @mitab_c_is_field_unique:=               GetProcAddress(MITABDLL_Handle,'_mitab_c_is_field_unique@8');
        @mitab_c_set_quick_spatial_index_mode:=  GetProcAddress(MITABDLL_Handle,'_mitab_c_set_quick_spatial_index_mode@4');
      end;
    end;
  end;
  {$ENDIF}
end;

initialization
  MITAB_libOK:= false;
  LibPath:= 'mitab.dll';

end.

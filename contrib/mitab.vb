' **********************************************************************
' * $Id: mitab.vb,v 1.2 2002-05-08 20:23:53 daniel Exp $
' *
' * Name:     mitab.vb
' * Project:  MapInfo TAB Read/Write library
' * Language: Visual Basic
' * Purpose:  Visual Basic include file for MITAB API prototypes using STDCALL
' *           calling convention. Used with MITAB dll compiled with
' *           the /Gz qualifier.
' * Author:   Bo Thomsen, bvt@sns.dk
' *
' **********************************************************************
' * Copyright (c) 2002, Bo Thomsen
' *
' * Permission is hereby granted, free of charge, to any person obtaining a
' * copy of this software and associated documentation files (the "Software"),
' * to deal in the Software without restriction, including without limitation
' * the rights to use, copy, modify, merge, publish, distribute, sublicense,
' * and/or sell copies of the Software, and to permit persons to whom the
' * Software is furnished to do so, subject to the following conditions:
' *
' * The above copyright notice and this permission notice shall be included
' * in all copies or substantial portions of the Software.
' *
' * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
' * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
' * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
' * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
' * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
' * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
' * DEALINGS IN THE SOFTWARE.
' **********************************************************************
' *
' * $Log: mitab.vb,v $
' * Revision 1.2  2002-05-08 20:23:53  daniel
' * Update from Bo, and re-re-renamed mitab_vb.dll to mitab.dll (stdcall is default now)
' *
' * Revision 1.21  2002/05/07 13:43:30  bvt
' * changed the dll name to mitab_vb.dll. Changed the vb file name
' * to mitab_vb.vb to reflect the new name of the dll.
' *
' * Revision 1.2  2002/05/05 13:43:30  bvt
' * Added mitab_c_get_field_width and mitab_c_get_field_precision to api.
' *
' * Revision 1.1  2002/02/22 13:43:30  daniel
' * Initial Revision from Bo Thomsen
' *
' * Revision 1.00  2002/02/20 12:35:00  bvt
' * New - used in conjunction with mitab_capi.cpp,v 1.15 compiled with
' * /Gz (stdcall calling convention)
' *
' *

Option Compare Database
Option Explicit
Type mitab_projinfo
  ph(255) As Byte
End Type

Rem feature type values

Public Const TABFC_NoGeom = 0
Public Const TABFC_Point = 1
Public Const TABFC_FontPoint = 2
Public Const TABFC_CustomPoint = 3
Public Const TABFC_Text = 4
Public Const TABFC_Polyline = 5
Public Const TABFC_Arc = 6
Public Const TABFC_Region = 7
Public Const TABFC_Rectangle = 8
Public Const TABFC_Ellipse = 9

Rem field types

Public Const TABFT_Char = 1
Public Const TABFT_Integer = 2
Public Const TABFT_SmallInt = 3
Public Const TABFT_Decimal = 4
Public Const TABFT_Float = 5
Public Const TABFT_Date = 6
Public Const TABFT_Logical = 7

Rem text justification

Public Const TABTJ_Left = 0
Public Const TABTJ_Center = 1
Public Const TABTJ_Right = 2

Rem text spacing

Public Const TABTS_Single = 0
Public Const TABTS_1_5 = 1
Public Const TABTS_Double = 2

Rem test linetype

Public Const TABTL_NoLine = 0
Public Const TABTL_Simple = 1
Public Const TABTL_Arrow = 2

Public Declare Function mitab_c_add_field Lib "mitab.dll" Alias "_mitab_c_add_field@20" (ByVal handle As Long, ByVal field_name As String, ByVal field_type As Long, ByVal width As Long, ByVal precision As Long) As Long
Public Declare Sub mitab_c_close Lib "mitab.dll" Alias "_mitab_c_close@4" (ByVal handle As Long)
Public Declare Function mitab_c_create Lib "mitab.dll" Alias "_mitab_c_create@44" (ByVal filename As String, ByVal mif_or_tab As String, ByVal mif_projectiondef As String, ByVal north As Double, ByVal south As Double, ByVal east As Double, ByVal west As Double) As Long
Public Declare Function mitab_c_create_feature Lib "mitab.dll" Alias "_mitab_c_create_feature@8" (ByVal handle As Long, ByVal feature_type As Long) As Long
Public Declare Sub mitab_c_destroy_feature Lib "mitab.dll" Alias "_mitab_c_destroy_feature@4" (ByVal feature As Long)
Public Declare Function mitab_c_get_brush_bgcolor Lib "mitab.dll" Alias "_mitab_c_get_brush_bgcolor@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_brush_fgcolor Lib "mitab.dll" Alias "_mitab_c_get_brush_fgcolor@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_brush_pattern Lib "mitab.dll" Alias "_mitab_c_get_brush_pattern@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_brush_transparent Lib "mitab.dll" Alias "_mitab_c_get_brush_transparent@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_field_as_string_vb Lib "mitab.dll" Alias "_mitab_c_get_field_as_string_vb@16" (ByVal feature As Long, ByVal field As Long, ByVal value As String, ByVal l as Long) As Long
Public Declare Function mitab_c_get_field_count Lib "mitab.dll" Alias "_mitab_c_get_field_count@4" (ByVal handle As Long) As Long
Public Declare Function mitab_c_get_field_name_vb Lib "mitab.dll" Alias "_mitab_c_get_field_name_vb@16" (ByVal handle As Long, ByVal field As Long, ByVal name As String, ByVal l As Long) As Long
Public Declare Function mitab_c_get_field_type Lib "mitab.dll" Alias "_mitab_c_get_field_type@8" (ByVal handle As Long, ByVal field As Long) As Long
Public Declare Function mitab_c_get_font_vb Lib "mitab.dll" Alias "_mitab_c_get_font_vb@12" (ByVal feature As Long, ByVal font As String, ByVal l As Long) As Long
Public Declare Function mitab_c_get_mif_coordsys_vb Lib "mitab.dll" Alias "_mitab_c_get_mif_coordsys_vb@12" (ByVal dataset As Long, ByVal coordsys As String) As Long
Public Declare Function mitab_c_get_parts Lib "mitab.dll" Alias "_mitab_c_get_parts@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_pen_color Lib "mitab.dll" Alias "_mitab_c_get_pen_color@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_pen_pattern Lib "mitab.dll" Alias "_mitab_c_get_pen_pattern@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_pen_width Lib "mitab.dll" Alias "_mitab_c_get_pen_width@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_projinfo Lib "mitab.dll" Alias "_mitab_c_get_projinfo@4" (ByVal dataset As Long) As mitab_projinfo
Public Declare Function mitab_c_get_symbol_color Lib "mitab.dll" Alias "_mitab_c_get_symbol_color@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_symbol_no Lib "mitab.dll" Alias "_mitab_c_get_symbol_no@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_symbol_size Lib "mitab.dll" Alias "_mitab_c_get_symbol_size@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_vb Lib "mitab.dll" Alias "_mitab_c_get_text_vb@12" (ByVal feature As Long, ByVal text As String, ByVal l As Long) As Long
Public Declare Function mitab_c_get_text_angle Lib "mitab.dll" Alias "_mitab_c_get_text_angle@4" (ByVal feature As Long) As Double
Public Declare Function mitab_c_get_text_bgcolor Lib "mitab.dll" Alias "_mitab_c_get_text_bgcolor@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_fgcolor Lib "mitab.dll" Alias "_mitab_c_get_text_fgcolor@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_height Lib "mitab.dll" Alias "_mitab_c_get_text_height@4" (ByVal feature As Long) As Double
Public Declare Function mitab_c_get_text_justification Lib "mitab.dll" Alias "_mitab_c_get_text_justification@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_linetype Lib "mitab.dll" Alias "_mitab_c_get_text_linetype@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_spacing Lib "mitab.dll" Alias "_mitab_c_get_text_spacing@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_text_width Lib "mitab.dll" Alias "_mitab_c_get_text_width@4" (ByVal feature As Long) As Double
Public Declare Function mitab_c_get_type Lib "mitab.dll" Alias "_mitab_c_get_type@4" (ByVal feature As Long) As Long
Public Declare Function mitab_c_get_vertex_count Lib "mitab.dll" Alias "_mitab_c_get_vertex_count@8" (ByVal feature As Long, ByVal part As Long) As Long
Public Declare Function mitab_c_get_vertex_x Lib "mitab.dll" Alias "_mitab_c_get_vertex_x@12" (ByVal feature As Long, ByVal part, ByVal vertex As Long) As Double
Public Declare Function mitab_c_get_vertex_y Lib "mitab.dll" Alias "_mitab_c_get_vertex_y@12" (ByVal feature As Long, ByVal part, ByVal vertex As Long) As Double
Public Declare Function mitab_c_getlasterrormsg_vb Lib "mitab.dll" Alias "_mitab_c_getlasterrormsg_vb@8" (ByVal errormsg As String, ByVal l As Long) As Long
Public Declare Function mitab_c_getlasterrorno Lib "mitab.dll" Alias "_mitab_c_getlasterrorno@0" () As Long
Public Declare Function mitab_c_next_feature_id Lib "mitab.dll" Alias "_mitab_c_next_feature_id@8" (ByVal handle As Long, ByVal last_feature_id As Long) As Long
Public Declare Function mitab_c_open Lib "mitab.dll" Alias "_mitab_c_open@4" (ByVal filename As String) As Long
Public Declare Function mitab_c_read_feature Lib "mitab.dll" Alias "_mitab_c_read_feature@8" (ByVal handle As Long, ByVal feature_id As Long) As Long
Public Declare Sub mitab_c_set_arc Lib "mitab.dll" Alias "_mitab_c_set_arc@52" (ByVal feature As Long, ByVal center_x, ByVal center_y, ByVal x_radius, ByVal y_radius, ByVal start_angle, ByVal end_angle As Double)
Public Declare Sub mitab_c_set_brush Lib "mitab.dll" Alias "_mitab_c_set_brush@20" (ByVal feature As Long, ByVal fg_color, ByVal bg_color, ByVal pattern, ByVal transparent As Long)
Public Declare Sub mitab_c_set_field Lib "mitab.dll" Alias "_mitab_c_set_field@12" (ByVal feature As Long, ByVal field_index As Long, ByVal value As String)
Public Declare Sub mitab_c_set_font Lib "mitab.dll" Alias "_mitab_c_set_font@8" (ByVal feature As Long, ByVal font_name As String)
Public Declare Sub mitab_c_set_pen Lib "mitab.dll" Alias "_mitab_c_set_pen@16" (ByVal feature As Long, ByVal width As Long, ByVal pattern As Long, ByVal color As Long)
Public Declare Sub mitab_c_set_points Lib "mitab.dll" Alias "_mitab_c_set_points@20" (ByVal feature As Long, ByVal part As Long, ByVal vertex_count As Long, x As Double, y As Double)
Public Declare Function mitab_c_set_projinfo Lib "mitab.dll" Alias "_mitab_c_set_projinfo@8" (ByVal dataset As Long, ByVal projinfo As mitab_projinfo) As Long
Public Declare Sub mitab_c_set_symbol Lib "mitab.dll" Alias "_mitab_c_set_symbol@16" (ByVal feature As Long, ByVal symbol_no, ByVal symbol_size, ByVal symbol_color As Long)
Public Declare Sub mitab_c_set_text Lib "mitab.dll" Alias "_mitab_c_set_text@8" (ByVal feature As Long, ByVal text As String)
Public Declare Sub mitab_c_set_text_display Lib "mitab.dll" Alias "_mitab_c_set_text_display@48" (ByVal feature As Long, ByVal angle, ByVal height, ByVal width As Double, ByVal fg_color, ByVal bg_color, ByVal justification, ByVal spacing, ByVal linetype As Long)
Public Declare Function mitab_c_write_feature Lib "mitab.dll" Alias "_mitab_c_write_feature@8" (ByVal handle As Long, ByVal feature As Long) As Long
Public Declare Function mitab_c_get_field_width Lib "mitab.dll" Alias "_mitab_c_get_field_width@8" (ByVal handle  As Long, ByVal field  As Long)  As Long
Public Declare Function mitab_c_get_field_precision Lib "mitab.dll" Alias "_mitab_c_get_field_precision@8" (ByVal handle  As Long, ByVal field As Long)  As Long



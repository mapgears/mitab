// .NET bindings for MITAB.
// Contributed by Graham Sims. See README_DOTNET.TXT for more details.
//
// $Id: MiApi.cs,v 1.1 2005-03-23 19:53:42 dmorissette Exp $
//

using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Collections;

namespace EBop.MapObjects.MapInfo {

	// feature type values
	public enum FeatureType {
		TABFC_NoGeom = 0,
		TABFC_Point = 1,
		TABFC_FontPoint = 2,
		TABFC_CustomPoint = 3,
		TABFC_Text = 4,
		TABFC_Polyline = 5,
		TABFC_Arc = 6,
		TABFC_Region = 7,
		TABFC_Rectangle = 8,
		TABFC_Ellipse = 9,
		TABFC_MultiPoint = 10};

	public enum FieldType { 
		TABFT_Char = 1,
		TABFT_Integer = 2,
		TABFT_SmallInt = 3,
		TABFT_Decimal = 4,
		TABFT_Float = 5,
		TABFT_Date = 6,
		TABFT_Logical = 7};

	public enum Justification {
		TABTJ_Left = 0,
		TABTJ_Center = 1,
		TABTJ_Right = 2};

	public enum TextSpacing {
		TABTS_Single = 0,
		TABTS_1_5 = 1,
		TABTS_Double = 2};

	// test linetype
	public enum LineSpacing {
		TABTL_NoLine = 0,
		TABTL_Simple = 1,
		TABTL_Arrow = 2};

	/// <summary>
	/// Wrapper functions for the version 1.3.0 of the MapInfo Tab API.
	/// </summary>
	/// <remarks>
	/// Requires mitab.dll.
	/// See http://mitab.maptools.org/
	/// Graham Sims
	/// </remarks>
	
	public class MiApi {

		private MiApi() {
		}
		
		/// <summary>
		/// Returns the version of the library. 
		/// </summary>
		/// <returns>An integer representing the current version of the MITAB library in the 
		/// format xxxyyyzzz, e.g. returns 1002004 for v1.2.4.  
		/// </returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_getlibversion();

		/// <summary>
		/// Get the last error message. 
		/// </summary>
		/// <remarks>
		/// Fetches the last error message posted with CPLError(), that hasn't been cleared by CPLErrorReset(). 
		/// The returned pointer is to an internal string that should not be altered or freed. 
		/// </remarks>
		/// <returns>A pointer to the last error message, or an empty string if there is no posted error message</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_getlasterrormsg")]
		private static extern IntPtr _mitab_c_getlasterrormsg();
		public static string mitab_c_getlasterrormsg() {
			return Marshal.PtrToStringAnsi(_mitab_c_getlasterrormsg());
		}
		
//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_getlasterrormsg_vb (
//			[MarshalAs(UnmanagedType.LPStr, SizeConst = 260)] string errormsg, int l);

		/// <summary>
		/// Fetch the last error number. 
		/// </summary>
		/// <remarks>This is the error number, not the error class. </remarks>
		/// <returns>The error number of the last error to occur, or CPLE_None (0) if there are no posted errors</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_getlasterrorno();

		/// <summary>
		/// Open an existing .TAB or .MIF dataset for read access. 
		/// </summary>
		/// <remarks>The function automatically detects the format (.MIF or .TAB) of the specified file.
		/// Note that it is not possible to open a file for update (i.e. read+write) with the current 
		/// version of the library.</remarks>
		/// <param name="filename">The complete filename (including extension .TAB or .MIF) of the file to open.</param>
		/// <returns>A valid mitab_handle, or NULL if the file could not be opened.</returns>
		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_open([MarshalAs(UnmanagedType.LPStr, SizeConst = 260)] string filename);

		/// <summary>
		/// Close a dataset previously opened using mitab_c_open() or created using mitab_c_create(). 
		/// </summary>
		/// <param name="mitab_handle">The mitab_handle of the dataset to close.</param>
		[DllImport("mitab.dll")]
		public static extern void mitab_c_close(IntPtr mitab_handle);

		/// <summary>
		/// Create a new .TAB or .MIF dataset.
		/// </summary>
		/// <remarks>
		/// Note that it is not possible to open a file for update (i.e. read+write) with the current version 
		/// of the library.
		/// </remarks>
		/// <param name="filename">The complete filename (including extension .TAB or .MIF) of the file to create.</param>
		/// <param name="mif_or_tab">One of "mif" to create a .MIF dataset or "tab" to create a .TAB dataset. 
		/// The default is to create a TAB dataset if this parameter's value is NULL or an empty string.</param>
		/// <param name="mif_projectiondef">The projection to use for the dataset, in the same format that is 
		/// used in the "CoordSys" line of a MIF file header. If this parameter's value is NULL or empty then 
		/// a LAT/LON coordsys is assumed. See also mitab_c_get_mif_coordsys().</param>
		/// <param name="north">The upper dataset bound. Note that valid bounds are required for a .TAB dataset 
		/// otherwise data may not be stored properly in the file. MITAB knows the default bounds only for the 
		/// most common MapInfo coordinate systems, passing north,south,east,west as 0,0,0,0 will instruct MITAB
		///  to attempt to use the default bounds for that projection. If no default bounds are found for this 
		///  projection then your data may not be stored properly in the file unless you provide valid bounds 
		///  via the north,south,east,west parameters.</param>
		/// <param name="south">The lower dataset bound.</param>
		/// <param name="east">The right dataset bound. </param>
		/// <param name="west">The left dataset bound.</param>
		/// <returns>A valid mitab_handle, or NULL if the file could not be created.</returns>
		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_create( 
			[MarshalAs(UnmanagedType.LPStr)] string filename,
			[MarshalAs(UnmanagedType.LPStr)] string mif_or_tab,
			[MarshalAs(UnmanagedType.LPStr)] string mif_projectiondef,
			double north, double south,
			double east, double west );
												   
		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_add_field( IntPtr handle, 
			[MarshalAs(UnmanagedType.LPStr)] string field_name,
			int field_type, int width, int precision, 
			int indexed, int unique );

		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_write_feature(IntPtr handle, IntPtr feature );


		/// <summary>
		/// Iterator to get the next valid feature id when reading a dataset opened with mitab_c_open(). 
		/// </summary>
		/// <param name="handle">the mitab_handle of the file opened for read access.</param>
		/// <param name="last_feature_id">The id of the last feature that was read. 
		/// Use -1 to get first feature id in the dataset.</param>
		/// <returns>The next valid feature id in the dataset, or -1 when there are no more feature ids.</returns> 
		[DllImport("mitab.dll")]
		public static extern int mitab_c_next_feature_id(IntPtr handle, int last_feature_id );

		/// <summary>
		/// Read a mitab_feature object from the file. 
		/// </summary>
		/// <remarks>Works only with datasets opened with mitab_c_open().</remarks>
		/// <param name="handle">The mitab_handle of the file opened for read access.</param>
		/// <param name="feature_id">The id of the feature to read, obtained by calling mitab_c_next_feature_id()</param>
		/// <returns>The mitab_feature object that was read. 
		/// The object will have to be destroyed using mitab_c_destroy_feature() once you are done with it.</returns>
		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_read_feature( IntPtr handle, int feature_id );

		/// <summary>
		/// Destroy a mitab_feature object and release all memory associated with it. 
		/// </summary>
		/// <param name="feature">The mitab_feature to destroy.</param>
		[DllImport("mitab.dll")]
		public static extern void mitab_c_destroy_feature( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_feature_id( IntPtr feature );

		/// <summary>
		/// Create a new mitab_feature object to be written to a dataset created using mitab_c_create(). 
		/// </summary>
		/// <param name="handle">The handle of the dataset opened for write access.</param>
		/// <param name="feature_type">A member of the feature type enumeration. At this point, only the following 
		/// types can be created by this C API function: TABFC_NoGeom (0), TABFC_Point (1), TABFC_FontPoint (2), 
		/// TABFC_CustomPoint (3), TABFC_Text (4), TABFC_Polyline (5), TABFC_Arc (6), TABFC_Region (7), 
		/// TABFC_Rectangle (8), TABFC_Ellipse (9) and TABFC_MultiPoint (10)</param>
		/// <returns>The new mitab_feature object, or NULL if creation failed. Note that the new object will 
		/// have to be released using mitab_c_destroy_feature().</returns>
		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_create_feature( IntPtr handle, int feature_type );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_field( IntPtr feature, int field_index, 
			[MarshalAs(UnmanagedType.LPStr)] string value );
    
		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_text( IntPtr feature, 
			[MarshalAs(UnmanagedType.LPStr)] string text );


		/// <summary>
		/// Get the text string on a TABFC_Text object. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>A pointer to the text string in the object.</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_text")]
		public static extern IntPtr _mitab_c_get_text( IntPtr feature );
		public static string mitab_c_get_text( IntPtr feature ) {
			return Marshal.PtrToStringAnsi(_mitab_c_get_text(feature));
		}

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_text_vb( IntPtr  feature, 
//			[MarshalAs(UnmanagedType.LPStr)] string text, int l );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_text_display( IntPtr feature,
			double angle, double height, double width,
			int fg_color, int bg_color,
			int justification, int spacing, int linetype );

		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_text_angle( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_text_height( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_text_width( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_text_fgcolor( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_text_bgcolor( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_text_justification( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_text_spacing( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_text_linetype( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_pen( IntPtr feature, int width, int pattern, int color );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_pen_color( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_pen_width( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_pen_pattern( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_brush( IntPtr feature,
			int fg_color, int bg_color, int pattern,
			int transparent );
		
		/// <summary>
		/// Get an object's brush foreground color property. Applies to region, ellipse and rectangle objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The brush foreground color (24 bits RGB value).</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_brush_fgcolor( IntPtr feature );

		/// <summary>
		/// Get an object's brush background color property. Applies to region, ellipse and rectangle objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The brush foreground color (24 bits RGB value).</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_brush_bgcolor( IntPtr feature );

		/// <summary>
		/// Get an object's brush pattern property. Applies to region, ellipse and rectangle objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The brush pattern number (1 is none, 2 is solid fill, etc.).</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_brush_pattern( IntPtr feature );

		/// <summary>
		/// Get an object's brush transparency property. Applies to region, ellipse and rectangle objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The brush transparency value, either 0 for an opaque brush (using bg color) or 1 for 
		/// transparent (ignore bg color).</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_brush_transparent( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_font( IntPtr feature, 
			[MarshalAs(UnmanagedType.LPStr, SizeConst = 80)] string font_name );

		/// <summary>
		/// Get the font name from a TABFC_Text or TABFC_FontPoint object, or the symbol name from a 
		/// TABFC_CustomPoint. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The text font name.</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_font")]
		private static extern IntPtr _mitab_c_get_font( IntPtr feature );
		public static string mitab_c_get_font( IntPtr feature ) {
			return Marshal.PtrToStringAnsi(_mitab_c_get_font( feature ));
		}

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_font_vb( IntPtr feature, 
//			[MarshalAs(UnmanagedType.LPStr, SizeConst = 80)] string font, int l );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_symbol( IntPtr feature, int symbol_no,
		   int symbol_size, int symbol_color );

		/// <summary>
		/// Get an object's symbol color property. Applies only to point and multipoint objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The symbol color (24 bits RGB value).</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_symbol_color( IntPtr feature );

		/// <summary>
		/// Get an object's symbol number property. Applies only to point and multipoint objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The symbol number (valid range: 32 to 67)</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_symbol_no( IntPtr feature );

		/// <summary>
		/// Get an object's symbol size property. Applies only to point and multipoint objects.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The symbol size in pixels (valid range 1 to 48)</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_symbol_size( IntPtr feature );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_points( IntPtr feature, int part,
			int vertex_count, ref double x, ref double y );

		[DllImport("mitab.dll")]
		public static extern void mitab_c_set_arc( IntPtr feature, 
			double center_x, double center_y,
			double x_radius, double y_radius,
			double start_angle, double end_angle);

		/// <summary>
		/// Return a mitab_feature's object type. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>A member of the FeatureType enumeration, one of TABFC_NoGeom (0), TABFC_Point (1), 
		/// TABFC_FontPoint (2), TABFC_CustomPoint (3), TABFC_Text (4), TABFC_Polyline (5), TABFC_Arc (6),
		/// TABFC_Region (7), TABFC_Rectangle (8), TABFC_Ellipse (9) or TABFC_MultiPoint (10).</returns>
		[DllImport("mitab.dll")]
		public static extern FeatureType mitab_c_get_type( IntPtr feature );

		/// <summary>
		/// Return the number of parts (rings or polyline segments) in an object. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <returns>The number of parts (in a region or polyline) or 0 if the object's 
		/// geometry was not set. For all object types other than polyline region, returns 1 
		/// if object geometry is set.
		/// </returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_parts( IntPtr feature );

		/// <summary>
		/// Return the number of points in a part of a mitab_feature object. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <param name="part">The part number we are interested in. 
		/// Use 0 for objects which cannot have multiple parts.</param>
		/// <returns>The number of points in that feature's part or 0 if the object has no 
		/// geometry or the part number is invalid.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_vertex_count( IntPtr feature, int part );

		/// <summary>
		/// Return the X coordinate of a point in a part of a mitab_feature object.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <param name="part">The part number we are interested in. Use 0 for objects which cannot have multiple parts.</param>
		/// <param name="vertex">The point number, with 0 being the first point.</param>
		/// <returns>The X coordinate of the specified point or null if the object has no geometry or the part or the point number is invalid.</returns>
		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_vertex_x( IntPtr feature, int part, int vertex );

		/// <summary>
		/// Return the Y coordinate of a point in a part of a mitab_feature object.
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <param name="part">The part number we are interested in. Use 0 for objects which cannot have multiple parts.</param>
		/// <param name="vertex">The point number, with 0 being the first point.</param>
		/// <returns>The Y coordinate of the specified point or null if the object has no geometry or the part or the point number is invalid.</returns>
		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_vertex_y( IntPtr feature, int part, int vertex );

		/// <summary>
		/// Return the number of features in a dataset.
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <returns>The number of features in the dataset.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_feature_count( IntPtr handle );

		/// <summary>
		/// Return the number of attribute fields in a dataset's schema.
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <returns>The number of attribute fields defined in the dataset.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_field_count( IntPtr handle );
		
		/// <summary>
		/// Return the type of an attribute field in a dataset's schema. 
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>The field type, one of TABFT_Char (1), TABFT_Integer (2), TABFT_SmallInt (3),
		/// TABFT_Decimal (4), TABFT_Float (5), TABFT_Date (6), or TABFT_Logical (7)</returns>
		[DllImport("mitab.dll")]
		public static extern FieldType mitab_c_get_field_type( IntPtr handle, int field );
		
		/// <summary>
		/// Return the name of an attribute field in a dataset's schema. 
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>A pointer to the field name. The returned string pointer is a reference to an 
		/// internal buffer and should not be modified or freed by the caller.</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_field_name")]
		private static extern IntPtr _mitab_c_get_field_name(IntPtr handle, int field);
		public static string mitab_c_get_field_name(IntPtr handle, int field) {
			return Marshal.PtrToStringAnsi( _mitab_c_get_field_name(handle, field) );
		}

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_field_name_vb( IntPtr handle, int field, 
//			[MarshalAs(UnmanagedType.LPStr, SizeConst = 80)] string name, int l );
		
		/// <summary>
		/// Return the width of an attribute field in a dataset's schema. 
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>The field width.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_field_width( IntPtr handle, int field );

		/// <summary>
		/// Return the precision of an attribute field in a dataset's schema. 
		/// </summary>
		/// <param name="handle">The dataset's handle.</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>The field precision.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_get_field_precision( IntPtr handle, int field );

		/// <summary>
		/// Fetch an attribute field value in a mitab_feature as a string.
		/// </summary>
		/// <remarks>The function returns a reference to an internal string buffer that contains the string 
		/// representation of the attribute field's value (integer and floating point values are converted 
		/// to string using sprintf()).</remarks>
		/// <param name="feature">The mitab_feature object</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>A pointer to a string containing the value of the field. The returned string pointer 
		/// is a reference to an internal buffer and should not be modified or freed by the caller. 
		/// Its value will be valid only until the next call to mitab_c_get_field().</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_field_as_string")]
		private static extern IntPtr _mitab_c_get_field_as_string( IntPtr feature, int field );
		public static string mitab_c_get_field_as_string( IntPtr feature, int field ) {
			return Marshal.PtrToStringAnsi( _mitab_c_get_field_as_string( feature, field ));
		}


		/// <summary>
		/// Fetch an attribute field value in a mitab_feature as a double. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <param name="field">The index of the field to look at, with 0 being the first field.</param>
		/// <returns>The value of the field converted to double.</returns>
		[DllImport("mitab.dll")]
		public static extern double mitab_c_get_field_as_double( IntPtr feature, int field);

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_field_as_string_vb( IntPtr feature, int field, 
//			[MarshalAs(UnmanagedType.LPStr, SizeConst = 80)]  string value, int l );

		[DllImport("mitab.dll")]
		public static extern IntPtr mitab_c_get_projinfo( IntPtr dataset );
		
		[DllImport("mitab.dll")]
		public static extern int mitab_c_set_projinfo( IntPtr dataset, IntPtr projinfo );
		
		/// <returns>A pointer to a string</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_mif_coordsys")]
		private static extern IntPtr _mitab_c_get_mif_coordsys(IntPtr dataset);
		public static string mitab_c_get_mif_coordsys(IntPtr dataset) {
			return Marshal.PtrToStringAnsi(_mitab_c_get_mif_coordsys(dataset));
		}
		

		///<returns>A pointer to a string</returns>
		[DllImport("mitab.dll", EntryPoint="mitab_c_get_extended_mif_coordsys")]
		private static extern IntPtr _mitab_c_get_extended_mif_coordsys(IntPtr dataset);
		public static string mitab_c_get_extended_mif_coordsys(IntPtr dataset) {
			return Marshal.PtrToStringAnsi(_mitab_c_get_extended_mif_coordsys(dataset));
		}

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_mif_coordsys_vb( IntPtr dataset, 
//			[MarshalAs(UnmanagedType.LPStr)] string coordsys, int l);

//		[DllImport("mitab.dll")]
//		public static extern int mitab_c_get_extended_mif_coordsys_vb( IntPtr dataset, 
//			[MarshalAs(UnmanagedType.LPStr)] string coordsys, int l);

		[DllImport("mitab.dll")]
		public static extern int mitab_c_load_coordsys_table(
			[MarshalAs(UnmanagedType.LPStr)] string filename);

		/// <summary>
		/// Return a false if the ring is the first of a polygon. 
		/// </summary>
		/// <param name="feature">The mitab_feature object.</param>
		/// <param name="requestedringindex">The requested ring index</param>
		/// <returns>True or false depends on the part number of the ring in the polygon, true if it's not the 
		/// first part of a polygon. If the feature is not a region the return value will be false.</returns>
		[DllImport("mitab.dll")]
		public static extern int mitab_c_is_interior_ring( IntPtr feature, int requestedringindex );
	}
}

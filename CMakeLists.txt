cmake_minimum_required(VERSION 3.10)
project(MITAB LANGUAGES C CXX)

# Visual Studio 2017 / Win32 / v120_xp. Configure using -T v120_xp.
# Static MSVC runtime for Release and Debug, including VS2017/CMake older than 3.15.
if(MSVC)
  foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
    if(DEFINED ${flag_var})
      string(REGEX REPLACE "/MDd?" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

# The original nmake build copies cpl_config.h.vc to cpl_config.h.
# Generate it in the build tree to keep source tree untouched.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cpl/cpl_config.h.vc" "${CMAKE_CURRENT_BINARY_DIR}/generated/cpl_config.h" COPYONLY)

set(MITAB_INCLUDE_DIRS
  "${CMAKE_CURRENT_BINARY_DIR}/generated"
  "${CMAKE_CURRENT_SOURCE_DIR}/cpl"
  "${CMAKE_CURRENT_SOURCE_DIR}/ogr"
  "${CMAKE_CURRENT_SOURCE_DIR}/mitab"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(CPL_SOURCES
  cpl/cpl_conv.cpp
  cpl/cpl_error.cpp
  cpl/cpl_string.cpp
  cpl/cpl_vsisimple.cpp
  cpl/cpl_path.cpp
  cpl/cpl_csv.cpp
  cpl/cplgetsymbol.cpp
  cpl/cpl_findfile.cpp
  cpl/cpl_minixml.cpp
  cpl/cpl_vsil_win32.cpp
  cpl/cpl_vsil.cpp
  cpl/cpl_vsi_mem.cpp
  cpl/cpl_vsil_unix_stdio_64.cpp
  cpl/cpl_multiproc.cpp
  cpl/cplstring.cpp
  cpl/cpl_getexecpath.cpp
  cpl/cpl_atomic_ops.cpp
  cpl/cpl_http.cpp
  cpl/cpl_strtod.cpp
  cpl/cpl_vsil_subfile.cpp
  cpl/cpl_recode_stub.cpp
  cpl/cpl_vsil_stdout.cpp
)

set(OGR_SOURCES
  ogr/ogr_srsnode.cpp
  ogr/ogrcurve.cpp
  ogr/ogrfeature.cpp
  ogr/ogrfeaturedefn.cpp
  ogr/ogrgeometry.cpp
  ogr/ogrgeometrycollection.cpp
  ogr/ogrgeometryfactory.cpp
  ogr/ogrlinearring.cpp
  ogr/ogrlinestring.cpp
  ogr/ogrmultilinestring.cpp
  ogr/ogrmultipoint.cpp
  ogr/ogrmultipolygon.cpp
  ogr/ogrpoint.cpp
  ogr/ogrpolygon.cpp
  ogr/ogrspatialreference.cpp
  ogr/ogrsurface.cpp
  ogr/ogrutils.cpp
  ogr/ogrlayer.cpp
  ogr/ogrsfdriverregistrar.cpp
  ogr/ogrfielddefn.cpp
  ogr/ogrdatasource.cpp
  ogr/ogrsfdriver.cpp
  ogr/ogr_srs_proj4.cpp
  ogr/ogrfeaturestyle.cpp
  ogr/ogr_fromepsg.cpp
  ogr/ogrfeaturequery.cpp
  ogr/swq.c
  ogr/ogrct.cpp
  ogr/ogr_gensql.cpp
  ogr/ogr_srs_xml.cpp
  ogr/ogr_srs_esri.cpp
  ogr/ogr_api.cpp
  ogr/gml2ogrgeometry.cpp
  ogr/ogr2gmlgeometry.cpp
  ogr/ogr_miattrind.cpp
  ogr/ogr_attrind.cpp
  ogr/ogr_srs_dict.cpp
)

set(MITAB_SOURCES
  mitab/mitab_rawbinblock.cpp
  mitab/mitab_mapheaderblock.cpp
  mitab/mitab_mapindexblock.cpp
  mitab/mitab_mapobjectblock.cpp
  mitab/mitab_mapcoordblock.cpp
  mitab/mitab_feature.cpp
  mitab/mitab_feature_mif.cpp
  mitab/mitab_mapfile.cpp
  mitab/mitab_idfile.cpp
  mitab/mitab_datfile.cpp
  mitab/mitab_tabfile.cpp
  mitab/mitab_miffile.cpp
  mitab/mitab_utils.cpp
  mitab/mitab_imapinfofile.cpp
  mitab/mitab_middatafile.cpp
  mitab/mitab_maptoolblock.cpp
  mitab/mitab_coordsys.cpp
  mitab/mitab_tooldef.cpp
  mitab/mitab_spatialref.cpp
  mitab/mitab_indfile.cpp
  mitab/mitab_tabview.cpp
  mitab/mitab_geometry.cpp
  mitab/mitab_bounds.cpp
  mitab/mitab_tabseamless.cpp
  mitab/mitab_capi.cpp
)

add_library(CPL STATIC ${CPL_SOURCES})
add_library(OGR STATIC ${OGR_SOURCES})
add_library(MITAB SHARED ${MITAB_SOURCES})
set_target_properties(CPL PROPERTIES OUTPUT_NAME cpl)
set_target_properties(OGR PROPERTIES OUTPUT_NAME ogr)
set_target_properties(MITAB PROPERTIES OUTPUT_NAME mitab)
foreach(target CPL OGR MITAB)
  target_include_directories(${target} PRIVATE ${MITAB_INCLUDE_DIRS})
  if(MSVC)
    target_compile_options(${target} PRIVATE /EHsc /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE)
  endif()
endforeach()
# Original mitab_capi.h explicitly declares the exported API as __stdcall.
# Do NOT globally enable /Gz: this is unnecessary and may affect unrelated functions.
target_link_libraries(MITAB PRIVATE OGR CPL)
if(MSVC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set_property(TARGET MITAB APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:WINDOWS,5.01")
  else()
    set_property(TARGET MITAB APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:WINDOWS,5.02")
  endif()
endif()

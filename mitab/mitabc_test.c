/**********************************************************************
 * $Id: mitabc_test.c,v 1.2 2000-01-14 16:33:24 warmerda Exp $
 *
 * Name:     mitabcrep.c
 * Project:  MapInfo TAB Read/Write library
 * Language: C
 * Purpose:  Report on MITAB file using MITAB C API - test program.
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
 * $Log: mitabc_test.c,v $
 * Revision 1.2  2000-01-14 16:33:24  warmerda
 * initial implementation complete
 *
 * Revision 1.1  2000/01/14 14:53:59  warmerda
 * New
 *
 */

#include <stdio.h>
#include <assert.h>
#include "mitab_capi.h"

#ifndef NULL
#  define NULL 0
#endif

/************************************************************************/
/*                              ReportFile                              */
/************************************************************************/

static void ReportFile( const char * pszFilename )

{
    mitab_handle	dataset;
    int			feature_id;
    
    dataset = mitab_c_open( pszFilename );

    if( dataset == NULL )
    {
        printf( "mitab_c_open(%s) failed.\n", pszFilename );
        exit( 1 );
    }

    for( feature_id = mitab_c_next_feature_id(dataset,-1);
         feature_id != -1;
         feature_id = mitab_c_next_feature_id(dataset,feature_id) )
    {
        mitab_feature	feature;

        feature = mitab_c_read_feature( dataset, feature_id );
        if( feature == NULL )
        {
            printf( "Failed to read feature %d.\n", feature_id );
            exit( 1 );
        }

        printf( "Read feature %d.\n", feature_id );

        mitab_c_destroy_feature( feature );
    }

    mitab_c_close( dataset );
}

/************************************************************************/
/*                              CopyFile()                              */
/************************************************************************/

static void CopyFile( const char * pszSource, const char * pszDest )

{
}

/************************************************************************/
/*                             WriteFile()                              */
/************************************************************************/

static void WriteFile( const char * pszDest, const char * pszMifOrTab )

{
    mitab_handle	dataset;
    mitab_feature	feature;
    double		x[100], y[100];
    int			field_index;
    
    dataset = mitab_c_create( pszDest, pszMifOrTab,
                              "CoordSys Earth Projection 1, 0",
                              90, 0, 180, -180 );

    if( dataset == NULL )
    {
        printf( "Failed to create %s file %s.\n", pszMifOrTab, pszDest );
        return;
    }

/* -------------------------------------------------------------------- */
/*      Add a text, float and integer field.                            */
/* -------------------------------------------------------------------- */
    field_index = mitab_c_add_field( dataset, "TestInt",
                                     TABFT_Integer, 8, 0 );
    assert( field_index == 0 );

    field_index = mitab_c_add_field( dataset, "TestFloat", TABFT_Float,
                                     12, 2 );
    assert( field_index == 1 );

    field_index = mitab_c_add_field( dataset, "TestString", TABFT_Char,
                                     10, 0 );
    assert( field_index == 2 );

/* -------------------------------------------------------------------- */
/*      Write a point.                                                  */
/* -------------------------------------------------------------------- */
    feature = mitab_c_create_feature( dataset, TABFC_Point );

    x[0] = 100;
    y[0] = 50;
    
    mitab_c_set_points( feature, 0, 1, x, y );
    mitab_c_set_symbol( feature, 1, 2, 255*256 );
    mitab_c_set_field( feature, 0, "100" );
    mitab_c_set_field( feature, 1, "100.5" );
    mitab_c_set_field( feature, 2, "12345678901234567890" );
    mitab_c_write_feature( dataset, feature );
    mitab_c_destroy_feature( feature );

/* -------------------------------------------------------------------- */
/*      Write a line.                                                   */
/* -------------------------------------------------------------------- */
    feature = mitab_c_create_feature( dataset, TABFC_Polyline );

    x[0] = 100;
    y[0] = 50;
    x[1] = 101;
    y[1] = 42;
    
    mitab_c_set_points( feature, 0, 2, x, y );
    mitab_c_set_pen( feature, 1, 2, 3, 65535 );
    mitab_c_write_feature( dataset, feature );
    mitab_c_destroy_feature( feature );

/* -------------------------------------------------------------------- */
/*      Write text.                                                     */
/* -------------------------------------------------------------------- */
    feature = mitab_c_create_feature( dataset, TABFC_Text );

    x[0] = 101;
    y[0] = 51;
    
    mitab_c_set_points( feature, 0, 1, x, y );
    mitab_c_set_text( feature, "My text" );
    mitab_c_set_font( feature, "Arial" );
    mitab_c_set_pen( feature, 1, 2, 3, 65535 );
    mitab_c_set_text_display( feature, 0.0, 0.0, 0.0, 255*65536, 0,
                              -1, -1, -1 );
    mitab_c_write_feature( dataset, feature );
    mitab_c_destroy_feature( feature );
    
/* -------------------------------------------------------------------- */
/*      Write region (polygon).                                         */
/* -------------------------------------------------------------------- */
    feature = mitab_c_create_feature( dataset, TABFC_Region );

    x[0] = 101;
    y[0] = 51;
    x[1] = 100;
    y[1] = 51;
    x[2] = 100;
    y[2] = 50;
    x[3] = 101;
    y[3] = 50;
    x[4] = 101;
    y[4] = 51;
    
    mitab_c_set_points( feature, 0, 5, x, y );
    
    x[0] = 100.5;
    y[0] = 50.5;
    x[1] = 100.7;
    y[1] = 50.7;
    x[2] = 100.7;
    y[2] = 50.7;
    x[3] = 100.5;
    y[3] = 50.5;
    
    mitab_c_set_points( feature, 1, 4, x, y );

    mitab_c_set_brush( feature, 255, 0, 1, 0 );
    mitab_c_set_pen( feature, 1, 2, 3, 65535 );
    mitab_c_write_feature( dataset, feature );
    mitab_c_destroy_feature( feature );
    
/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    mitab_c_close( dataset );
    
}

/************************************************************************/
/*                                main()                                */
/************************************************************************/

int main( int nArgc, char ** papszArgv )

{
    if( nArgc < 2 )
    {
        printf( "Usage: mitabc_test src_filename [dst_filename]\n" );
        printf( "    or mitabc_test -w[mif/tab] dst_filename\n" );
        exit( 1 );
    }

    if( nArgc == 2 )
        ReportFile( papszArgv[1] );
    else if( strcmp(papszArgv[1],"-wtab") == 0 )
        WriteFile( papszArgv[2], "tab" );
    else if( strcmp(papszArgv[1],"-wmif") == 0 )
        WriteFile( papszArgv[2], "mif" );
    else
        CopyFile( papszArgv[1], papszArgv[2] );

    exit( 0 );
}








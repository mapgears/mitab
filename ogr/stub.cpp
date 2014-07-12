#include "gdal_priv.h"
#include "ogr_api.h"
#include "gdal_alg_priv.h"

char *OGR_G_ExportToKML( OGRGeometryH, const char* pszAltitudeMode )
{
    return NULL;
}

char   *OGR_G_ExportToJson( OGRGeometryH )
{
    return NULL;
}

CPLErr 
HFAAuxBuildOverviews( const char *pszOvrFilename, GDALDataset *poParentDS,
                      GDALDataset **ppoDS,
                      int nBands, int *panBandList,
                      int nNewOverviews, int *panNewOverviewList, 
                      const char *pszResampling, 
                      GDALProgressFunc pfnProgress, 
                      void *pProgressData )
{
    return CE_Failure;
}

CPLErr 
GTIFFBuildOverviews( const char * pszFilename,
                     int nBands, GDALRasterBand **papoBandList, 
                     int nOverviews, int * panOverviewList,
                     const char * pszResampling, 
                     GDALProgressFunc pfnProgress, void * pProgressData )
{
    return CE_Failure;
}

void *
GDALCreateGCPRefineTransformer( int nGCPCount, const GDAL_GCP *pasGCPList, 
                                int nReqOrder, int bReversed, double tolerance, int minimumGcps)
{
    return NULL;
}

void  *
GDALCreateGCPTransformer( int nGCPCount, const GDAL_GCP *pasGCPList, 
                          int nReqOrder, int bReversed )
{
    return NULL;
}

void  *
GDALCreateTPSTransformer( int nGCPCount, const GDAL_GCP *pasGCPList, 
                          int bReversed )
{
    return NULL;
}

void  *
GDALCreateRPCTransformer( GDALRPCInfo *psRPC, int bReversed, 
                          double dfPixErrThreshold,
                          char **papszOptions )

{
    return NULL;
}

void  *
GDALCreateGeoLocTransformer( GDALDatasetH hBaseDS, 
                             char **papszGeolocationInfo,
                             int bReversed )

{
    return NULL;
}

void* GDALCreateTPSTransformerInt( int nGCPCount, const GDAL_GCP *pasGCPList, 
                                   int bReversed, char** papszOptions )
{
    return NULL;
}

void GDALDestroyGCPTransformer( void *pTransformArg )
{
}

void GDALDestroyTPSTransformer( void *pTransformArg )
{
}

void GDALDestroyRPCTransformer( void *pTransformArg )
{
}

void GDALDestroyGeoLocTransformer( void *pTransformArg )
{
}

int GDALGCPTransform( 
    void *pTransformArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
    return 0;
}

int GDALTPSTransform( 
    void *pTransformArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
    return 0;
}

int GDALRPCTransform( 
    void *pTransformArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
    return 0;
}

int GDALGeoLocTransform( 
    void *pTransformArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
    return 0;
}

CPL_C_START
void *GDALDeserializeGCPTransformer( CPLXMLNode *psTree );
void *GDALDeserializeTPSTransformer( CPLXMLNode *psTree );
void *GDALDeserializeGeoLocTransformer( CPLXMLNode *psTree );
void *GDALDeserializeRPCTransformer( CPLXMLNode *psTree );
CPL_C_END;

void *GDALDeserializeGCPTransformer( CPLXMLNode *psTree )
{
    return NULL;
}

void *GDALDeserializeTPSTransformer( CPLXMLNode *psTree )
{
    return NULL;
}

void *GDALDeserializeGeoLocTransformer( CPLXMLNode *psTree )
{
    return NULL;
}

void *GDALDeserializeRPCTransformer( CPLXMLNode *psTree )
{
    return NULL;
}

void* GDALCloneTPSTransformer( void *pTransformArg )
{
    return NULL;
}


#include "mitab_ogr_driver.h"


OGRTABDataSource::OGRTABDataSource( const char * pszNameIn,
                                        TABFile *poLayerIn )

{
    m_pszName = CPLStrdup( pszNameIn );
    m_poLayer = poLayerIn;
}

OGRTABDataSource::~OGRTABDataSource()

{
    CPLFree( m_pszName ); 
    delete m_poLayer;
}

OGRMIDDataSource::OGRMIDDataSource( const char * pszNameIn,
                                        MIFFile *poLayerIn )

{
    m_pszName = CPLStrdup( pszNameIn );
    m_poLayer = poLayerIn;
}

/************************************************************************/
/*                        ~OGRShapeDataSource()                         */
/************************************************************************/

OGRMIDDataSource::~OGRMIDDataSource()

{
    CPLFree( m_pszName );
    delete m_poLayer;
}

/************************************************************************/
/*                          ~OGRShapeDriver()                           */
/************************************************************************/

OGRTABDriver::~OGRTABDriver()

{
}

/************************************************************************/
/*                              GetName()                               */
/************************************************************************/

const char *OGRTABDriver::GetName()

{
    return "MapInfo TABFile";
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

OGRDataSource *OGRTABDriver::Open( const char * pszFilename,
                                     int bUpdate )

{

    if( bUpdate )
    {
	return NULL;
    }
       
/* -------------------------------------------------------------------- */
/*      Create the layer object.                                        */
/* -------------------------------------------------------------------- */
    TABFile	*poLayer;

    poLayer = new TABFile();
    if (poLayer->Open(pszFilename,"r") ==0)
         return new OGRTABDataSource( pszFilename, poLayer );
    else
    {
	delete poLayer;
	return NULL;
    }
}

/************************************************************************/
/*                          ~OGRMIDDriver()                           */
/************************************************************************/

OGRMIDDriver::~OGRMIDDriver()

{
}

/************************************************************************/
/*                              GetName()                               */
/************************************************************************/

const char *OGRMIDDriver::GetName()

{
    return "MapInfo Mid/Mif File";
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

OGRDataSource *OGRMIDDriver::Open( const char * pszFilename,
                                     int bUpdate )

{

    if( bUpdate )
    {
	return NULL;
    }
       
/* -------------------------------------------------------------------- */
/*      Create the layer object.                                        */
/* -------------------------------------------------------------------- */
    MIFFile	*poLayer;

    poLayer = new MIFFile();
    if (poLayer->Open(pszFilename,"r") == 0)
      return new OGRMIDDataSource( pszFilename, poLayer );
    else
    {
	delete poLayer;
	return NULL;
    }
}

/************************************************************************/
/*              RegisterOGRTAB() and RegisterOGRMID()                   */
/************************************************************************/

extern "C"
{

void RegisterOGRTAB()

{
    OGRSFDriverRegistrar::GetRegistrar()->RegisterDriver( new OGRTABDriver );
}

void RegisterOGRMID()

{
    OGRSFDriverRegistrar::GetRegistrar()->RegisterDriver( new OGRMIDDriver );
}

}

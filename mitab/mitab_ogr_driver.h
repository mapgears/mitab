#include "mitab.h"
#include "ogrsf_frmts.h"



class OGRTABDataSource : public OGRDataSource
{
  private:
    TABFile	        *m_poLayer;
    char		*m_pszName;
    
  public:
    			 OGRTABDataSource( const char * pszName,
                                            TABFile * poLayerIn );
    			~OGRTABDataSource();

    const char	        *GetName() { return m_pszName; }
    int			 GetLayerCount() { return 1; }
    OGRLayer		*GetLayer( int ) { return m_poLayer; }
    int                 TestCapability( const char * ){return 0;}
    OGRLayer    *CreateLayer(const char *, 
                                      OGRSpatialReference * = NULL,
                                      OGRwkbGeometryType = wkbUnknown,
					     char ** = NULL ){return NULL;}
};
 

class OGRMIDDataSource : public OGRDataSource
{
  private:
    MIFFile	        *m_poLayer;
    char		*m_pszName; 
    
  public:
    			 OGRMIDDataSource( const char * pszName,
                                            MIFFile * poLayerIn );
    			~OGRMIDDataSource();

    const char	        *GetName() { return m_pszName; }
    int			 GetLayerCount() { return 1; }
    OGRLayer		*GetLayer( int ) { return m_poLayer; }
    int                 TestCapability( const char * ){return 0;}
    virtual OGRLayer    *CreateLayer( const char *, 
                                      OGRSpatialReference * = NULL,
                                      OGRwkbGeometryType = wkbUnknown,
                                      char ** = NULL ){return NULL;}
};


class OGRTABDriver : public OGRSFDriver
{
public:
              ~OGRTABDriver();

     const char *GetName();
     OGRDataSource *Open ( const char *,int );
     int TestCapability( const char * ){return 0;}
     virtual OGRDataSource *CreateDataSource( const char *pszName,
						 char ** = NULL ){return NULL;}
    
    

};


class OGRMIDDriver : public OGRSFDriver
{
  public: 
                ~OGRMIDDriver();

      const char *GetName();
      OGRDataSource *Open( const char *,int );
      int                 TestCapability( const char * ){return 0;}
      virtual OGRDataSource *CreateDataSource( const char *pszName,
					       char ** = NULL ){return NULL;}
    
    
};











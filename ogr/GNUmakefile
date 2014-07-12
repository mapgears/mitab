#
# CPL (Common Portability Library) makefile
#

OBJ	=	ogr_srsnode.o ogrcurve.o ogrfeature.o ogrfeaturedefn.o \
		ogrgeometry.o ogrgeometrycollection.o ogrgeometryfactory.o \
		ogrlinearring.o ogrlinestring.o ogrmultilinestring.o \
		ogrmultipoint.o ogrmultipolygon.o ogrpoint.o ogrpolygon.o \
		ogrspatialreference.o ogrsurface.o ogrutils.o \
		ogrlayer.o ogrsfdriverregistrar.o ogrfielddefn.o \
		ogr_srs_proj4.o ogrdatasource.o ogrsfdriver.o \
		ogrfeaturestyle.o ogr_fromepsg.o ogrfeaturequery.o swq.o \
		ogrct.o ogr_gensql.o ogr_srs_xml.o ogr_srs_esri.o \
		ogr_api.o gml2ogrgeometry.o ogr2gmlgeometry.o \
		ogr_miattrind.o ogr_attrind.o ogr_srs_dict.o \
		swq_expr_node.o swq_op_general.o swq_op_registrar.o swq_parser.o swq_select.o \
		gdalmajorobject.o gdaldataset.o gdalrasterband.o gdalmultidomainmetadata.o \
		ogrgeomfielddefn.o gdal_misc.o gdaldriver.o gdaldrivermanager.o \
		gdalcolortable.o rasterio.o gdalrasterblock.o gdaldefaultoverviews.o \
		gdaltransformer.o gdalopeninfo.o gdalclientserver.o gdalpamdataset.o gdalpamrasterband.o \
		gdalnodatamaskband.o gdalnodatavaluesmaskband.o gdalpamproxydb.o gdal_rat.o \
		gdalvirtualmem.o gdalallvalidmaskband.o gdalproxypool.o gdalproxydataset.o gdaldefaultasync.o \
		ogrunionlayer.o ogrwarpedlayer.o ogrlayerdecorator.o overview.o ogr_srs_ozi.o \
		stub.o

LIB	=	ogr.a

include ../GNUmake.opt

default:	$(LIB)

clean:
	rm -f *.o $(LIB)

$(LIB): $(OBJ)
	rm -f $(LIB)
	$(AR) rc $(LIB) $(OBJ)


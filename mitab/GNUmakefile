# Makefile for the MapInfo TAB Read/Write library
#

include ../GNUmake.opt

INCLUDE :=	-I../ogr $(INCLUDE)

#
#  
#
TAB_OBJS= mitab_rawbinblock.o mitab_mapheaderblock.o mitab_mapindexblock.o \
	  mitab_mapobjectblock.o mitab_mapcoordblock.o mitab_maptoolblock.o \
	  mitab_feature.o mitab_tooldef.o \
	  mitab_mapfile.o mitab_idfile.o mitab_datfile.o mitab_tabfile.o \
	  mitab_utils.o mitab_spatialref.o mitab_feature_mif.o \
	  mitab_middatafile.o mitab_miffile.o mitab_imapinfofile.o \
	  mitab_coordsys.o mitab_indfile.o mitab_tabview.o \
	  mitab_geometry.o mitab_capi.o mitab_bounds.o mitab_tabseamless.o

TAB_HDRS= mitab.h mitab_priv.h mitab_utils.h 

OGRTAB_OBJS = mitab_ogr_driver.o mitab_ogr_datasource.o


MITABLIB_OBJS= $(TAB_OBJS)
MITAB_LIB=      mitab.a

LIBS =	$(MITAB_LIB) ../ogr/ogr.a ../cpl/cpl.a $(LIB_DBMALLOC) -ldl


default: $(MITAB_LIB) tab2tab ogrinfo mitabc_test


$(TAB_OBJS):	$(TAB_HDRS)

$(OGRTAB_OBJS):	mitab_ogr_driver.h

tab2tab: tab2tab.o $(LIBS) mitab.h mitab_priv.h
	$(CXX) $(LFLAGS) -o tab2tab tab2tab.o $(LIBS) $(LIB_DBMALLOC) -lm 

ogr2ogr: ogr2ogr.o $(OGRTAB_OBJS) $(LIBS) mitab.h mitab_priv.h
	$(CXX) $(LFLAGS) -o ogr2ogr ogr2ogr.o $(OGRTAB_OBJS) $(LIBS) \
		$(LIB_DBMALLOC) -lm

ogrinfo: ogrinfo.o $(OGRTAB_OBJS) $(LIBS) mitab.h mitab_priv.h
	$(CXX) $(LFLAGS) -o ogrinfo ogrinfo.o $(OGRTAB_OBJS) $(LIBS) \
		$(LIB_DBMALLOC) -lm

mitabc_test: mitabc_test.o $(LIBS) mitab.h mitab_priv.h mitab_capi.h
	$(CXX) $(LFLAGS) -o mitabc_test mitabc_test.o $(LIBS) \
		$(LIB_DBMALLOC) -lm

tabindex: tabindex.o $(LIBS) mitab.h mitab_priv.h
	$(CXX) $(LFLAGS) -o tabindex tabindex.o $(LIBS) $(LIB_DBMALLOC) -lm

tabdump: tabdump.o $(LIBS) mitab.h mitab_priv.h
	$(CXX) $(LFLAGS) -o tabdump tabdump.o $(LIBS) $(LIB_DBMALLOC) -lm

../cpl/cpl.a:
	(cd ../cpl; make)

$(MITAB_LIB): $(MITABLIB_OBJS)
	rm -f $(MITAB_LIB)
	$(AR) rc $(MITAB_LIB) $(MITABLIB_OBJS)

clean:
	rm -f $(MITAB_LIB) $(MITABLIB_OBJS) $(OGRTAB_OBJS)
	rm -f tab2tab.o tab2tab
	rm -f ogr2ogr.o ogr2ogr
	rm -f ogrinfo.o ogrinfo
	rm -f mitabc_test mitabc_test.o
	rm -f tabdump.o tabdump
	rm -f tabindex.o tabindex


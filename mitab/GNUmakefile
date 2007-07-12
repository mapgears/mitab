# Makefile for the MapInfo TAB Read/Write library
#
# $Id: GNUmakefile,v 1.7 2007-07-12 12:30:30 dmorissette Exp $
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


MITABLIB_OBJS=   $(TAB_OBJS)
MITAB_LIB=       mitab.a
MITAB_SHARED_LIB_LINKNAME=libmitab.so
MITAB_SHARED_LIB_SONAME  =libmitab.so.1
MITAB_SHARED_LIB_FULLNAME=libmitab.so.1.0.1

# -ldl doesn't work on MacOSX
ifneq "$(shell uname -s)" "Darwin"
  DL_LIB = -ldl
endif

LIBS =	$(MITAB_LIB) ../ogr/ogr.a ../cpl/cpl.a $(LIB_DBMALLOC) $(DL_LIB)


default: $(MITAB_LIB) $(MITAB_SHARED_LIB_FULLNAME) tab2tab ogrinfo mitabc_test tabdump


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

$(MITAB_SHARED_LIB_FULLNAME): $(MITABLIB_OBJS)
	rm -f $(MITAB_SHARED_LIB_FULLNAME) $(MITAB_SHARED_LIB_SONAME)  $(MITAB_SHARED_LIB_LINKNAME)
	$(CXX) $(MITABLIB_OBJS) ../ogr/ogr.a ../cpl/cpl.a -shared -Wl,-soname,$(MITAB_SHARED_LIB_SONAME) -o $(MITAB_SHARED_LIB_FULLNAME) -ldl
	ln -s $(MITAB_SHARED_LIB_FULLNAME) $(MITAB_SHARED_LIB_SONAME)
	ln -s $(MITAB_SHARED_LIB_SONAME) $(MITAB_SHARED_LIB_LINKNAME) 

clean:
	rm -f $(MITAB_LIB) $(MITABLIB_OBJS) $(OGRTAB_OBJS)
	rm -f $(MITAB_SHARED_LIB_FULLNAME) $(MITAB_SHARED_LIB_SONAME)  $(MITAB_SHARED_LIB_LINKNAME)
	rm -f tab2tab.o tab2tab
	rm -f ogr2ogr.o ogr2ogr
	rm -f ogrinfo.o ogrinfo
	rm -f mitabc_test mitabc_test.o
	rm -f tabdump.o tabdump
	rm -f tabindex.o tabindex


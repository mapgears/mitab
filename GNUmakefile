#
# Unix makefile to build the MITAB library and test programs
#


default:	all

all:
	(cd cpl; $(MAKE))
	(cd ogr; $(MAKE))
	(cd mitab; $(MAKE))

clean:
	(cd cpl; $(MAKE) clean)
	(cd ogr; $(MAKE) clean)
	(cd mitab; $(MAKE) clean)


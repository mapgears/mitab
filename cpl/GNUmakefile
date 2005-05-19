#
# CPL (Common Portability Library) makefile
#

OBJ	=	cpl_conv.o cpl_error.o cpl_string.o cpl_vsisimple.o \
		cplgetsymbol.o cpl_dir.o cpl_path.o cpl_csv.o cpl_findfile.o \
		cpl_minixml.o

LIB	=	cpl.a

include ../GNUmake.opt

default:	cpl_config.h $(LIB)

clean:
	rm -f *.o $(LIB)

cpl_config.h:	cpl_config.h.in
	cp cpl_config.h.in cpl_config.h

$(LIB): $(OBJ)
	rm -f $(LIB)
	$(AR) rc $(LIB) $(OBJ)


dist:	clean
	rm -f $(HOME)/cpl.zip
	(cd ..; zip $(HOME)/cpl.zip ./cpl/cpl*.cpp ./cpl/cpl*.h \
			./cpl/README.* ./cpl/cpl_config.h.* )
	echo Built $(HOME)/cpl.zip


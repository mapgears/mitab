VB, Pascal, and MapBasic interfaces contributed by Bo Thomsen:
==============================================================

Subject:   SV: Some modifications to MITAB c-api.
   Date:   Fri, 22 Feb 2002 11:07:17 +0100
   From:   BVT@sns.dk
     To:   morissette@dmsolutions.ca

Hi Daniel - 

My calendar tells me that I should send you a message :) ...

I've revised my original functions according to your wish about backwards
compatibility and attached new versions of the files that's been edited.

mitab_capi.cpp, mitab_capi.h :

6 new functions which _supplements_ the existing functions returning "const
char *" function results. 

The new function is named as the existing functions with the suffix "_vb",
ex:

const char * mitab_c_getlasterrormsg () ==> int mitab_c_getlasterrormsg_vb
(char * errormsg, int l)

All of the new functions have two new arguments : 
1) char * xxx: which will contain the returned string. The char * (string)
variable must be initialized with ex. 255 blanks before being used.    
2) int l: the maximum allowed lenght of the string, i.e the number of
"blanks" in the string.

The function itself returns the actual length of the result string.

nmake.opt:

The line 
OPTFLAGS=       /Zi /nologo 

has been changed to 
OPTFLAGS=       /Zi /nologo /Gz

which makes the compiler use the stdcall calling convention instead of cdecl
as a default. The best method is probably to make som kind of qualifier to
the "nmake" command, so it's possible to switch between stdcall and cdecl.

mitab.pas: Pascal interface unit using stdcall.
mitab_cdecl.pas: Pascal interface unit using cdecl.
mitab.def: include file for MapBasic to mitab using stdcall.
mitab.vb: include file for Visual Basic to Mitab using stdcall

I haven't done any extensive error hunt, but all of the above compiles
cleanly. I don't have access to a unix or linux based pc, so I haven't tried
to compile and link mitab with the GNU c compiler 

Please check the header's in the files. I've tried to make changes with
revision numbers and explanations, but you seem to use some kind of
documentation generator I don't recognize.

Drop me a mail, if you make any changes in the .h and .cpp files, and I'll
edit the .pas .def and .vb files to reflect the changes.

regards 
Bo Thomsen
GeoConsult I/S
Denmark



------------
$Id: README_VB.TXT,v 1.1 2002-02-22 13:43:30 daniel Exp $

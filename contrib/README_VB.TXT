VB, Pascal, and MapBasic interfaces to mitab.dll and mitab_cdecl.dll
=================================================================

The interface files consist of 4 files: 

mitab.pas: Pascal interface unit to mitab.dll.

mitab_cdecl.pas: Pascal interface unit to mitab_cdecl.dll

mitab.def: include file for MapBasic to mitab.dll.

mitab.vb: include file for Visual Basic to Mitab.dll

Delphi can use both mitab.dll, which use the "stdcall" calling convention and 
mitab_cdecl.dll, which use the "cdecl" calling convention. Some text book indicate, that 
stdcall (mitab.dll) should be sligtly faster than cdecl. My personal experience 
is that the difference is insignificant.   

MapBasic and Visual Basic can only use the stdcall calling convention (mitab.dll). 

MB and VB is unable to use dll-functions that returns string as a function result.
So 6 new functions which _supplements_ the existing functions returning "const
char *" function results has been added to the mitab api. 

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


Have fun 

Bo Thomsen


--------
$Id: README_VB.TXT,v 1.2 2002-05-08 20:23:53 daniel Exp $

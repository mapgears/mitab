Microsoft Visual Studio Project Files
-------------------------------------

Contributed by Tony Wells (see bug 1877).

-------- Original Message --------
Subject: RE: MITAB Visual Studio Project
Date: Mon, 21 Apr 2008 11:57:59 -0400
From: Tony Wells <tony.wells@...>
To: Daniel Morissette <dmorissette@...>

Below are instructions that should work.  I tested it quickly and I
think it should work.

Put the vcproj and sln in the route directory of mitab as downloaded and
unzipped (preserving file structure) from the mitab site.

Remove the .vc from cpl_config.h.vc filename

Change in #include "mitab/mitab_priv.h" in ogr_miattrind.cpp to #include
"..\mitab/mitab_priv.h"

That should compile fine from there. There are a lot of warnings
generated when compiled but should be no errors.

The .sln file isn't really needed.  You could just open the vcproj and
of course when including mitab as part of a larger project you would
just include the vcproj anyway.

[...]

Please redistribute as you see fit.  I hope people get some use out of
it and it advances development.

Tony Wells



--------
$Id: README_VCPROJ.TXT,v 1.1 2008-04-21 18:13:50 dmorissette Exp $
--------

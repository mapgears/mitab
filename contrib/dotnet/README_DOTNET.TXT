The .net wrappers include:

	1. MiApi.cs			This imports the c functions and enumerations from version 
						1.3.0 of the MapInfo Tab API.
	
	2. MiWrapper.cs		These classes wrap the functions in MiApi to produce a 
						hierarchy of classes giving readonly access to the feature points. 

						eg. 
						
						Layer layer = Layer.GetByName("c:/test.tab");
						foreach( Feature feature in layer.Features) {
							foreach(Part part in feature.Parts) {
								foreach (Vertrex v in part.Vertices) {
									Console.Out(v.X+”,”v.Y)
								}
							}
						}

These source files can either be dropped directly into your project, or
alternatively you could build it as a seperate assembly.
						
Graham Sims,
Environment Bay of Plenty, Whakatane, New Zealand
http://www.envbop.govt.nz

-------
$Id: README_DOTNET.TXT,v 1.3 2005-03-24 17:05:09 dmorissette Exp $

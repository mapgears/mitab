/* allproj.cpp
 *
 * Create one MIF file for each projection definition in mapinfow.prj
 *
 *
 * Reads mapinfow.prj in current directory and writes all mif files to 
 * same dir.
 *
 * $Id: allproj.cpp,v 1.1 2001-01-23 21:23:41 daniel Exp $
 *
 * $Log: allproj.cpp,v $
 * Revision 1.1  2001-01-23 21:23:41  daniel
 * Added projection bounds lookup table, called from TABFile::SetProjInfo()
 *
 */

#include "cpl_string.h"

int main()
{
    FILE *fpIn, *fpOut;
    const char *pszLine = NULL;
    int iFile = 0;

    if ((fpIn = fopen("mapinfow.prj", "r")) == NULL)
    {
        printf("mapinfow.prj not found.\n");
        exit(1);
    }

    while((pszLine = CPLReadLine(fpIn)) != NULL)
    {
        char **papszParams = CSLTokenizeStringComplex(pszLine, " ,", TRUE,FALSE);
        if (CSLCount(papszParams) >= 3 && 
            !EQUALN(papszParams[0], "---", 3))
        {
            int nProj = 0;
            int nDatum= 0;
            int nUnitsParamIndex = 0;

            if ((fpOut = fopen(CPLSPrintf("tttproj%04.4d.mif", iFile), "w")) == NULL)
            {
                printf("Failed creating MIF output file!\n");
                exit(2);
            }

            nProj = atoi(papszParams[1]);
            nDatum = atoi(papszParams[2]);
            if (nProj >= 1000)
            {
                printf("File tttproj%04.4d.mif uses projection %d ... \n"
                       "this case is not handled properly by this version\n",
                       iFile, nProj);
            }

            if (nProj == 1)
                nUnitsParamIndex = -1;  // No units for geographic proj.
            else if (nProj == 0)
            {
                nUnitsParamIndex = 2;   // NonEarth... units only.
                printf("File tttproj%04.4d.mif is NonEarth\n", iFile); 
            }
            else if (nDatum == 999 || nDatum == 9999)
            {
                nUnitsParamIndex = -1;  // Custom datum defn= 9999,x,x,x,x,...
                                        // Units are in numeric fmt
                                        // No conversion required.
                printf("File tttproj%04.4d.mif has custom datum\n", iFile); 
            }
            else
                nUnitsParamIndex = 3;

            fprintf(fpOut, "Version 300\n");
            fprintf(fpOut, "Charset \"WindowsLatin1\"\n");
            fprintf(fpOut, "Delimiter \",\"\n");
            if (nProj == 0)
                fprintf(fpOut, "CoordSys Nonearth ");
            else
                fprintf(fpOut, "CoordSys Earth Projection %d", nProj);

            for(int i=2; papszParams[i]!=NULL; i++)
            {
                if (i == nUnitsParamIndex)
                {
                    // Units string (except for proj=1: geographic)
                    const char *pszUnits = "???";
                    switch(atoi(papszParams[i]))
                    {
                      case 6:
                        pszUnits = "cm";
                        break;
                      case 31:
                        pszUnits = "ch";
                        break;
                      case 3:
                        pszUnits = "ft";
                        break;
                      case 2:
                        pszUnits = "in";
                        break;
                      case 1:
                        pszUnits = "km";
                        break;
                      case 30:
                        pszUnits = "li";
                        break;
                      case 7:
                        pszUnits = "m";
                        break;
                      case 0:
                        pszUnits = "mi";
                        break;
                      case 5:
                        pszUnits = "mm";
                        break;
                      case 9:
                        pszUnits = "nmi";
                        break;
                      case 32:
                        pszUnits = "rd";
                        break;
                      case 8:
                        pszUnits = "survey ft";
                        break;
                      case 4:
                        pszUnits = "yd";
                        break;
                      default:
                        printf("WARNING: Unsupported units type: %s in\n%s\n", papszParams[i], pszLine);
                        pszUnits = "m";
                        break;
                    }
                    if (nProj == 0)
                        fprintf(fpOut, "Units \"%s\" Bounds (-1,-1)(1,1)", pszUnits);
                    else
                        fprintf(fpOut, ", \"%s\"", pszUnits);
                }
                else
                    fprintf(fpOut, ", %s", papszParams[i]);
            }
            fprintf(fpOut, "\n");

            fprintf(fpOut, "Columns 1\n");
            fprintf(fpOut, "  ttt Char(10)\n");
            fprintf(fpOut, "Data\n");
            fprintf(fpOut, "POINT 0 0\n");

            fclose(fpOut);

            if ((fpOut = fopen(CPLSPrintf("tttproj%04.4d.mid", iFile++), "w")))
            {
                fprintf(fpOut, "ttt\n");
                fclose(fpOut);
            }

        }
        CSLDestroy(papszParams);
    }

}

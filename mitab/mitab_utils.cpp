/**********************************************************************
 * $Id: mitab_utils.cpp,v 1.2 1999-07-12 05:44:59 daniel Exp $
 *
 * Name:     mitab_utils.cpp
 * Project:  MapInfo TAB Read/Write library
 * Language: C++
 * Purpose:  Misc. util. functions for the library
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, Daniel Morissette
 *
 * All rights reserved.  This software may be copied or reproduced, in
 * all or in part, without the prior written consent of its author,
 * Daniel Morissette (danmo@videotron.ca).  However, any material copied
 * or reproduced must bear the original copyright notice (above), this 
 * original paragraph, and the original disclaimer (below).
 * 
 * The entire risk as to the results and performance of the software,
 * supporting text and other information contained in this file
 * (collectively called the "Software") is with the user.  Although 
 * considerable efforts have been used in preparing the Software, the 
 * author does not warrant the accuracy or completeness of the Software.
 * In no event will the author be liable for damages, including loss of
 * profits or consequential damages, arising out of the use of the 
 * Software.
 * 
 **********************************************************************
 *
 * $Log: mitab_utils.cpp,v $
 * Revision 1.2  1999-07-12 05:44:59  daniel
 * Added include math.h for VC++
 *
 * Revision 1.1  1999/07/12 04:18:25  daniel
 * Initial checkin
 *
 **********************************************************************/

#include "mitab_utils.h"

#include <math.h>       /* sin()/cos() */

/**********************************************************************
 *                       TABGenerateArc()
 *
 * Generate the coordinates for an arc and ADD the coordinates to the 
 * geometry object.  If the geometry already contains some points then
 * these won't be lost.
 *
 * poLine can be a OGRLineString or one of its derived classes, such as 
 *        OGRLinearRing
 * numPoints is the number of points to generate.
 * Angles are specified in radians, valid values are in the range [0..2*PI]
 *
 * Arcs are always generated counterclockwise, even if StartAngle > EndAngle
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABGenerateArc(OGRLineString *poLine, int numPoints, 
                   double dCenterX, double dCenterY,
                   double dXRadius, double dYRadius,
                   double dStartAngle, double dEndAngle)
{
    double dX, dY, dAngleStep, dAngle;
    int i;

    // Adjust angles to go counterclockwise
    if (dEndAngle < dStartAngle)
        dEndAngle += 2.0*PI;

    dAngleStep = (dEndAngle-dStartAngle)/(numPoints-1.0);

    for(i=0; i<numPoints; i++)
    {
        dAngle = (dStartAngle + i*dAngleStep);
        dX = dCenterX + dXRadius*cos(dAngle);
        dY = dCenterY + dYRadius*sin(dAngle);
        poLine->addPoint(dX, dY);
    }

    return 0;
}


/**********************************************************************
 *                       TABCloseRing()
 *
 * Check if a ring is closed, and add a point to close it if necessary.
 *
 * Returns 0 on success, -1 on error.
 **********************************************************************/
int TABCloseRing(OGRLineString *poRing)
{
    if ( poRing->getNumPoints() > 0 && !poRing->get_IsClosed() )
    {
        poRing->addPoint(poRing->getX(0), poRing->getY(0));
    }

    return 0;
}



/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2010  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/
 
/* created: 27/02/2008
   updated: 03/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBoundingBox.h"

/**
 * \file SCEBoundingBox.c
 * \copydoc boundingbox
 * 
 * \file SCEBoundingBox.h
 * \copydoc boundingbox
 */

/**
 * \defgroup boundingbox Bounding box managment
 * \ingroup interface
 * \brief Boundig box managment and computing functions
 */

/** @{ */

/**
 * \brief Initializes a bounding box structure
 * \sa SCE_Box_Init()
 */
void SCE_BoundingBox_Init (SCE_SBoundingBox *box)
{
    size_t i;
    SCE_Box_Init (&box->box);
    for (i = 0; i < 6; i++)
        SCE_Plane_Init (&box->planes[i]);
}
/**
 * \brief Set a bounding box from a box
 */
void SCE_BoundingBox_SetFrom (SCE_SBoundingBox *box, SCE_SBox *b)
{
    SCE_Box_Copy (&box->box, b);
}

/**
 * \brief Gets the box of a bounding box
 * \sa SCE_Box_Init(), SCE_SBox
 */
SCE_SBox* SCE_BoundingBox_GetBox (SCE_SBoundingBox *box)
{
    return &box->box;
}

/**
 * \brief Gets the points of the box of a bounding box
 * \sa SCE_Box_GetPoints()
 */
float* SCE_BoundingBox_GetPoints (SCE_SBoundingBox *box)
{
    return SCE_Box_GetPoints (&box->box);
}

/**
 * \brief Makes the planes of a bounding box (needed by the collision module)
 * \sa SCE_Box_MakePlanes()
 */
void SCE_BoundingBox_MakePlanes (SCE_SBoundingBox *box)
{
    SCE_Box_MakePlanes (&box->box, box->planes);
}
/**
 * \brief Gets the planes of a bounding box
 * \sa SCE_BoundingBox_MakePlanes()
 */
SCE_SPlane* SCE_BoundingBox_GetPlanes (SCE_SBoundingBox *box)
{
    return box->planes;
}
void SCE_BoundingBox_GetPlanesv (SCE_SBoundingBox *box, SCE_SPlane *planes)
{
    unsigned int i;
    for (i = 0; i < 6; i++)
        planes[i] = box->planes[i];
}

/**
 * \brief Applies a matrix to a bounding box and saves the old data
 * \param box a bounding box
 * \param m the matrix to apply
 * \param old writes out the old box here
 * \sa SCE_BoundingBox_Pop(), SCE_Box_Copy()
 */
void SCE_BoundingBox_Push (SCE_SBoundingBox *box, SCE_TMatrix4x3 m,
                           SCE_SBox *old)
{
    SCE_Box_Copy (old, &box->box);
    SCE_Box_ApplyMatrix4x3 (&box->box, m);
}
/**
 * \brief Restores the data of a bounding box saved by SCE_BoundingBox_Push()
 * \param box a bounding box
 * \param old data to restore
 * \sa SCE_BoundingBox_Push(), SCE_Box_Copy()
 */
void SCE_BoundingBox_Pop (SCE_SBoundingBox *box, SCE_SBox *old)
{
    SCE_Box_Copy (&box->box, old);
}

/** @} */

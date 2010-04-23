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

/* created: 03/08/2009
   updated: 24/08/2009 */

#include <string.h>             /* memcpy */
#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBox.h"

/**
 * \brief Initialize a box
 * \param box the box to initialize
 */
void SCE_Box_Init (SCE_SBox *box)
{
    int i;
    for (i = 0; i < 8; i++)
        SCE_Vector3_Operator1 (box->p[i], =, 0.);
}
/**
 * \brief Copies \p src into \p dst
 */
void SCE_Box_Copy (SCE_SBox *dst, const SCE_SBox *src)
{
    memcpy (dst->p, src->p, 8 * sizeof (SCE_TVector3));
}

/**
 * \brief Sets size and origin of a box
 * \param box a box
 * \param o origin vector of the box
 * \param w,h,d new box's width, height and depth
 * 
 * \see SCE_Box_SetFromCenter(), SCE_Box_Setv()
 */
void SCE_Box_Set (SCE_SBox *box, const SCE_TVector3 o,
                  float w, float h, float d)
{
    SCE_Vector3_Set (box->p[0], o[0],   o[1],   o[2]);
    SCE_Vector3_Set (box->p[1], o[0]+w, o[1],   o[2]);
    SCE_Vector3_Set (box->p[2], o[0]+w, o[1]+h, o[2]);
    SCE_Vector3_Set (box->p[3], o[0],   o[1]+h, o[2]);
    SCE_Vector3_Set (box->p[4], o[0],   o[1]+h, o[2]+d);
    SCE_Vector3_Set (box->p[5], o[0]+w, o[1]+h, o[2]+d);
    SCE_Vector3_Set (box->p[6], o[0]+w, o[1],   o[2]+d);
    SCE_Vector3_Set (box->p[7], o[0],   o[1],   o[2]+d);
}
/**
 * \brief Vectorial version of SCE_Box_Set()
 * \param box a box
 * \param o origin vector of the box
 * \param d dimension vector of the bouding box
 * \see SCE_Box_Set()
 */
void SCE_Box_Setv (SCE_SBox *box, const SCE_TVector3 o, const SCE_TVector3 d)
{
    SCE_Box_Set (box, o, d[0], d[1], d[2]);
}
/*
 *  4_____________5
 *  |\           |\
 *  | \          | \
 *  |  \         |  \
 *  |   \        |   \
 *  |    \3___________\2
 *  |    |            |         z   y
 *  |7__ | ______|6   |          \ |
 *   \   |        \   |           \|__ x
 *    \  |         \  |
 *     \ |          \ |
 *      \|___________\|
 *       0            1
 */

/**
 * \brief Sets position and size of a box
 * \param box a box
 * \param c the coordinates of the new box's center
 * \param w,h,d new box's width, height and depth
 */
void SCE_Box_SetFromCenter (SCE_SBox *box, const SCE_TVector3 c, float w,
                            float h, float d)
{
    SCE_TVector3 origin;
    origin[0] = c[0] - w/2.0f;
    origin[1] = c[1] - h/2.0f;
    origin[2] = c[2] - d/2.0f;
    SCE_Box_Set (box, origin, w, h, d);
}

/**
 * \brief Constructs a box from the farest extremity points
 */
void SCE_Box_SetFromMinMax (SCE_SBox *box, const SCE_TVector3 min,
                            const SCE_TVector3 max)
{
    SCE_TVector3 maxp;
    SCE_Vector3_Operator2v (maxp, =, max, -, min);
    SCE_Box_Set (box, min, maxp[0], maxp[1], maxp[2]);
}

/**
 * \brief Sets the size of a bouding box
 * \param box a box
 * \param w,h,d new box's width, height and depth
 */
void SCE_Box_SetSize (SCE_SBox *box, float w, float h, float d)
{
    SCE_TVector3 center;
    SCE_Box_GetCenterv (box, center);
    SCE_Box_SetFromCenter (box, center, w, h, d);
}

/**
 * \brief Sets the center of a box
 * \param box a box
 * \param x,y,z new center coordinates
 * 
 * This function updates center coordinates of a box (move it).
 * \see SCE_Box_SetCenterv()
 */
void SCE_Box_SetCenter (SCE_SBox *box, float x, float y, float z)
{
    SCE_TVector3 center;
    center[0] = x;
    center[1] = y;
    center[2] = z;
    SCE_Box_SetCenterv (box, center);
}
/**
 * \brief Vectorial version of SCE_Box_SetCenter()
 * \see SCE_Box_SetCenter()
 */
void SCE_Box_SetCenterv (SCE_SBox *box, const SCE_TVector3 c)
{
    int i;
    SCE_TVector3 dir, center;
    SCE_Box_GetCenterv (box, center);
    SCE_Vector3_Operator2v (dir, =, c, -, center);
    /* translate all points */
    for (i = 0; i < 8; i++)
        SCE_Vector3_Operator1v (box->p[i], +=, dir);
}
/**
 * \brief Gets center of a box
 * \param box a box
 * \param center a vector where store box's center coordinates
 */
void SCE_Box_GetCenterv (SCE_SBox *box, SCE_TVector3 center)
{
    center[0] = (box->p[1][0] - box->p[0][0]) / 2.0f + box->p[0][0];
    center[1] = (box->p[2][1] - box->p[1][1]) / 2.0f + box->p[1][1];
    center[2] = (box->p[7][2] - box->p[0][2]) / 2.0f + box->p[0][2];
}

float* SCE_Box_GetOrigin (SCE_SBox *box)
{
    /* as box->p is a float[8][3], it is the same to return box->p or box->p[0],
     * (we always return the address of box->p[0][0] that is the first of all)
     * but GCC complains when we returns box->p, then make him happy by
     * returning box->p[0]. */
    return box->p[0];
}

void SCE_Box_GetOriginv (SCE_SBox *box, SCE_TVector3 o)
{
    SCE_Vector3_Copy (o, box->p);
}


/**
 * \brief Gets the eigth corners of a box
 * \param box a box
 * \returns an array containing the eight vector coordinates of the eight
 *          box's corners
 * \see SCE_Box_GetPointsv()
 */
float* SCE_Box_GetPoints (SCE_SBox *box)
{
    /* see comment in SCE_Box_GetOrigin() */
    return box->p[0];
}
/**
 * \brief Gets the eigth corners of a box
 * \param box a box
 * \param p an array of eight SCE_TVector3 to fill with the eight corner
 *          coordinates of the box
 * 
 * This function is the vectorial version of SCE_Box_GetPoints().
 * \see SCE_Box_GetPoints()
 */
void SCE_Box_GetPointsv (SCE_SBox *box, SCE_TVector3 p[6])
{
    int i;
    for (i = 0; i < 8; i++)
        SCE_Vector3_Copy (p[i], box->p[i]);
}

/**
 * \brief Builds the planes of a box
 * \param planes write out the planes here
 */
void SCE_Box_MakePlanes (SCE_SBox *box, SCE_SPlane planes[6])
{
    SCE_TVector3 n;

    /* near */
    SCE_Vector3_Operator2v (n, =, box->p[1], -, box->p[6]);
    SCE_Plane_SetFromPointv (&planes[0], n, box->p[1]);
    /* far */
    SCE_Vector3_Operator1 (n, *=, -1.0f);
    SCE_Plane_SetFromPointv (&planes[1], n, box->p[6]);

    /* left */
    SCE_Vector3_Operator2v (n, =, box->p[1], -, box->p[0]);
    SCE_Plane_SetFromPointv (&planes[2], n, box->p[1]);
    /* right */
    SCE_Vector3_Operator1 (n, *=, -1.0f);
    SCE_Plane_SetFromPointv (&planes[3], n, box->p[0]);

    /* up */
    SCE_Vector3_Operator2v (n, =, box->p[1], -, box->p[2]);
    SCE_Plane_SetFromPointv (&planes[4], n, box->p[1]);
    /* down */
    SCE_Vector3_Operator1 (n, *=, -1.0f);
    SCE_Plane_SetFromPointv (&planes[5], n, box->p[2]);
}

/**
 * \brief Get the width of a box
 * \returns width of the given box
 * \sa SCE_Box_GetDimensionsv()
 */
float SCE_Box_GetWidth (SCE_SBox *box)
{
    return box->p[1][0] - box->p[0][0];
}
/**
 * \brief Get the height of a box
 * \returns height of the given box
 * \sa SCE_Box_GetDimensionsv()
 */
float SCE_Box_GetHeight (SCE_SBox *box)
{
    return box->p[2][1] - box->p[1][1];
}
/**
 * \brief Get the depth of a box
 * \returns depth of the given box
 * \sa SCE_Box_GetDimensionsv()
 */
float SCE_Box_GetDepth (SCE_SBox *box)
{
    return box->p[7][2] - box->p[0][2];
}
/**
 * \brief Writes the dimensions of a box into the given pointers
 * \param w,h,d will writes the dimensions here, some pointers can be NULL
 * \sa SCE_Box_GetWidth(), SCE_Box_GetHeight(), SCE_Box_GetDepth()
 */
void SCE_Box_GetDimensionsv (SCE_SBox *box, float *w, float *h, float *d)
{
    if (w)
        *w = SCE_Box_GetWidth (box);
    if (h)
        *h = SCE_Box_GetHeight (box);
    if (d)
        *d = SCE_Box_GetDepth (box);
}

/**
 * \brief Applies a 4x4 matrix to a box
 * \sa SCE_Matrix4_MulV3(), SCE_Box_ApplyMatrix4x3()
 */
void SCE_Box_ApplyMatrix4 (SCE_SBox *box, SCE_TMatrix4 m)
{
    size_t i;
    for (i = 0; i < 8; i++)
        SCE_Matrix4_MulV3Copy (m, box->p[i]);
}
/**
 * \brief Applies a 3x3 matrix to a box
 * \sa SCE_Matrix3_MulV3(), SCE_Box_ApplyMatrix4x3()
 */
void SCE_Box_ApplyMatrix3 (SCE_SBox *box, SCE_TMatrix3 m)
{
    size_t i;
    for (i = 0; i < 8; i++)
        SCE_Matrix3_MulV3Copy (m, box->p[i]);
}
/**
 * \brief Applies a 4x3 matrix to a box
 * \sa SCE_Matrix4x3_MulV3(), SCE_Box_ApplyMatrix4()
 */
void SCE_Box_ApplyMatrix4x3 (SCE_SBox *box, SCE_TMatrix4x3 m)
{
    size_t i;
    for (i = 0; i < 8; i++)
        SCE_Matrix4x3_MulV3Copy (m, box->p[i]);
}

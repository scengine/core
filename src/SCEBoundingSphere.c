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
 
/* created: 06/01/2009
   updated: 04/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBox.h"
#include "SCE/core/SCEBoundingSphere.h"

/**
 * \file SCEBoundingSphere.c
 * \copydoc boundingsphere
 * 
 * \file SCEBoundingSphere.h
 * \copydoc boundingsphere
 */

/**
 * \defgroup boundingsphere Bounding sphere managment
 * \ingroup interface
 * \brief Boundig sphere managment and computing functions
 */

/** @{ */

/**
 * \brief Initialize a bounding sphere
 * \param sphere the bounding sphere to initialize
 */
void SCE_BoundingSphere_Init (SCE_SBoundingSphere *sphere)
{
    SCE_BoundingSphere_Set (sphere, 0.0f, 0.0f, 0.0f, 1.0f);
}
/**
 * \brief Set a bounding sphere from a sphere
 * \param sphere the bounding sphere to set
 * \param s the source sphere
 */
void SCE_BoundingSphere_SetFrom (SCE_SBoundingSphere *sphere, SCE_SSphere *s)
{
    SCE_Sphere_Copy (&sphere->sphere, s);
}

/**
 * \brief Sets a bounding sphere
 * \param sphere the bounding sphere to set
 * \param x, y, z coordinates of the bouding sphere's center
 * \param radius radius of the bouding sphere
 */
void SCE_BoundingSphere_Set (SCE_SBoundingSphere *sphere,
                             float x, float y, float z, float radius)
{
    SCE_Vector3_Set (sphere->sphere.center, x, y, z);
    sphere->sphere.radius = radius;
}
/**
 * \brief Sets a bounding sphere from a vector
 * \param sphere the bounding sphere to set
 * \param center a 3D vector representing the coordinates of the bouding
 *               sphere's center
 * \param radius radius of the bouding sphere
 */
void SCE_BoundingSphere_Setv (SCE_SBoundingSphere *sphere,
                              SCE_TVector3 center, float radius)
{
    SCE_Vector3_Copy (sphere->sphere.center, center);
    sphere->sphere.radius = radius;
}

/**
 * \brief Gets the sphere defining a bouding sphere
 * \param sphere a bouding sphere
 * \returns the bouding sphere's sphere
 */
SCE_SSphere* SCE_BoundingSphere_GetSphere (SCE_SBoundingSphere *sphere)
{
    return &sphere->sphere;
}
/**
 * \brief Gets the center coordinates of a bouding sphere
 * \param sphere a bouding sphere
 * \returns the 3D vector representing the center coordinates of \p sphere
 */
float* SCE_BoundingSphere_GetCenter (SCE_SBoundingSphere *sphere)
{
    return sphere->sphere.center;
}
/**
 * \brief Gets the radius of a bouding sphere
 * \param sphere a bouding sphere
 * \returns the radius of \p sphere
 */
float SCE_BoundingSphere_GetRadius (SCE_SBoundingSphere *sphere)
{
    return sphere->sphere.radius;
}

static void SCE_BoundingSphere_MakeBoxFrom (SCE_SSphere *sphere, SCE_SBox *box)
{
    SCE_Box_SetFromCenter (box, sphere->center, sphere->radius,
                           sphere->radius, sphere->radius);
}
static void SCE_BoundingSphere_ApplyMatrix (SCE_SSphere *sphere,
                                            SCE_TMatrix4x3 m)
{
    float highest, h, d;
    SCE_SBox box;

    /* use box to determine highest radius after transformation */
    SCE_BoundingSphere_MakeBoxFrom (sphere, &box);
    SCE_Box_ApplyMatrix4x3 (&box, m);
    highest = SCE_Box_GetWidth (&box);
    h = SCE_Box_GetHeight (&box);
    d = SCE_Box_GetDepth (&box);
    highest = MAX (highest, h);
    highest = MAX (highest, d);
    /* apply the matrix to the center vector */
    SCE_Matrix4x3_MulV3Copy (m, sphere->center);
    sphere->radius = highest;
}

/**
 * \brief Pushes a matrix on a bouding sphere
 * \param sphere a bouding sphere
 * \param m the matrix to push
 * \param old a sphere to fill with the pre-push state of the bouding sphere,
 *        which can be used to pop the transformation later
 * \sa SCE_BoundingSphere_Pop()
 */
void SCE_BoundingSphere_Push (SCE_SBoundingSphere *sphere, SCE_TMatrix4x3 m,
                              SCE_SSphere *old)
{
    SCE_Sphere_Copy (old, &sphere->sphere);
    SCE_BoundingSphere_ApplyMatrix (&sphere->sphere, m);
}
/**
 * \brief Pops a matrix transformation
 * \param sphere a bouding sphere
 * \param old the state of \p sphere saved by a previous call to
 *        SCE_BoundingSphere_Push()
 * \sa SCE_BoundingSphere_Push()
 */
void SCE_BoundingSphere_Pop (SCE_SBoundingSphere *sphere, SCE_SSphere *old)
{
    SCE_Sphere_Copy (&sphere->sphere, old);
}

/** @} */

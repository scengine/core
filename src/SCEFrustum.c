/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2011  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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
 
/* created: 28/02/2008
   updated: 01/11/2011 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCECollide.h"
#include "SCE/core/SCEGeometry.h" /* compute bounding sphere */
#include "SCE/core/SCEFrustum.h"

void SCE_Frustum_Init (SCE_SFrustum *f)
{
    unsigned int i;
    for (i = 0; i < 6; i++)
        SCE_Plane_Init (&f->planes[i]);
}

void SCE_Frustum_MakeFromMatrices (SCE_SFrustum *f, SCE_TMatrix4 view,
                                   SCE_TMatrix4 proj)
{
    unsigned int i;
    SCE_TMatrix4 clip; /* combined matrices */

    SCE_Matrix4_Mul (proj, view, clip);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_RIGHT],
                   clip[12]-clip[0], clip[13]-clip[1],
                   clip[14]-clip[2], clip[15]-clip[3]);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_LEFT],
                   clip[12]+clip[0], clip[13]+clip[1],
                   clip[14]+clip[2], clip[15]+clip[3]);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_BOTTOM],
                   clip[12]+clip[4], clip[13]+clip[5],
                   clip[14]+clip[6], clip[15]+clip[7]);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_TOP],
                   clip[12]-clip[4], clip[13]-clip[5],
                   clip[14]-clip[6], clip[15]-clip[7]);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_FAR],
                   clip[12]-clip[8], clip[13]-clip[9],
                   clip[14]-clip[10], clip[15]-clip[11]);
    SCE_Plane_Set (&f->planes[SCE_FRUSTUM_NEAR],
                   clip[12]+clip[8], clip[13]+clip[9],
                   clip[14]+clip[10], clip[15]+clip[11]);
    /* normalisation */
    for (i = 0; i < 6; i++)
        SCE_Plane_Normalize (&f->planes[i], SCE_TRUE);
}

#define N_PL 6
int SCE_Frustum_BoundingBoxIn (SCE_SFrustum *f, SCE_SBoundingBox *b)
{
    return SCE_Collide_PlanesWithBB (f->planes, N_PL, b);
}
int SCE_Frustum_BoundingSphereIn (SCE_SFrustum *f, SCE_SBoundingSphere *s)
{
    return SCE_Collide_PlanesWithBS (f->planes, N_PL, s);
}

int SCE_Frustum_BoundingBoxInBool (SCE_SFrustum *f, SCE_SBoundingBox *b)
{
    return SCE_Collide_PlanesWithBBBool (f->planes, N_PL, b);
}
int SCE_Frustum_BoundingSphereInBool (SCE_SFrustum *f, SCE_SBoundingSphere *s)
{
    return SCE_Collide_PlanesWithBSBool (f->planes, N_PL, s);
}

/*
 * How points are indexed:
 *
 *              far
 *        4_____________5
 *        |\           |\
 *        | \          | \
 *        |  \         |  \
 *        |   \        |   \
 *        |    \3___________\2
 *        |    |            |         z   y
 *   left |7__ | ______|6   | right    \ |
 *         \   |        \   |           \|__ x
 *          \  |         \  |
 *           \ |          \ |
 *            \|___________\|
 *             0            1
 *                  near
 */
static void SCE_Frustum_GetPoints (const SCE_SPlane p[6], SCE_TVector3 v[8])
{
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_NEAR], &p[SCE_FRUSTUM_LEFT],
                             &p[SCE_FRUSTUM_BOTTOM], v[0]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_NEAR], &p[SCE_FRUSTUM_RIGHT],
                             &p[SCE_FRUSTUM_BOTTOM], v[1]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_NEAR], &p[SCE_FRUSTUM_RIGHT],
                             &p[SCE_FRUSTUM_TOP], v[2]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_NEAR], &p[SCE_FRUSTUM_LEFT],
                             &p[SCE_FRUSTUM_TOP], v[3]);

    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_FAR], &p[SCE_FRUSTUM_LEFT],
                             &p[SCE_FRUSTUM_BOTTOM], v[7]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_FAR], &p[SCE_FRUSTUM_RIGHT],
                             &p[SCE_FRUSTUM_BOTTOM], v[6]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_FAR], &p[SCE_FRUSTUM_RIGHT],
                             &p[SCE_FRUSTUM_TOP], v[5]);
    SCE_Plane_Intersection3 (&p[SCE_FRUSTUM_FAR], &p[SCE_FRUSTUM_LEFT],
                             &p[SCE_FRUSTUM_TOP], v[4]);
}

/**
 * \brief Extracts corners of a frustum
 * \param f a frustum
 * \param near change near plane, negative values hold it unchanged
 * \param far change far plane, negative values hold it unchanged
 * \param p 8 corner points
 * \sa SCE_Box_MakePlanes(), SCE_Frustum_ExtractBoundingSphere()
 */
void SCE_Frustum_ExtractCorners (const SCE_SFrustum *f, float near, float far,
                                 SCE_TVector3 p[8])
{
    if (near < 0.0f || far < 0.0f)
        SCE_Frustum_GetPoints (f->planes, p);
    else {
        int i;
        SCE_SPlane planes[6];

        for (i = 0; i < 6; i++)
            SCE_Plane_Copy (&planes[i], &f->planes[i]);

        /* far plane hax */
        SCE_Plane_Copy (&planes[SCE_FRUSTUM_FAR], &f->planes[SCE_FRUSTUM_NEAR]);

        planes[SCE_FRUSTUM_FAR].d -= far;
        planes[SCE_FRUSTUM_NEAR].d -= near;

        SCE_Frustum_GetPoints (planes, p);
    }
}

/**
 * \brief Computes a bounding sphere around the corners of the given frustum
 * \param f a frustum
 * \param near change near plane
 * \param far change far plane
 * \param sphere bounding sphere
 * \sa SCE_Frustum_ExtractCorners()
 */
void SCE_Frustum_ExtractBoundingSphere (const SCE_SFrustum *f, float near,
                                        float far, SCE_SSphere *sphere)
{
    SCE_SBox box;
    SCE_TVector3 p[8];
    float d, r;
    SCE_TVector3 middle;

    SCE_Box_Init (&box);
    SCE_Frustum_ExtractCorners (f, near, far, p);

    SCE_Vector3_Operator2v (middle, =, p[4], +, p[6]);
    SCE_Vector3_Operator1 (middle, *=, 0.5);
    d = 0.5 * SCE_Vector3_Distance (p[4], p[6]);
    r = SCE_Vector3_Distance (middle, p[0]);

    if (r > d) {
        SCE_Sphere_SetRadius (sphere, r);
        SCE_Sphere_SetCenterv (sphere, middle);
    } else {
        SCE_TVector3 middle2, dir;
        float x, a, b;

        r = d;
        SCE_Sphere_SetRadius (sphere, r);

        SCE_Vector3_Operator2v (middle2, =, p[0], +, p[2]);
        SCE_Vector3_Operator1 (middle2, *=, 0.5);
        SCE_Vector3_Operator2v (dir, =, middle, -, middle2);
        SCE_Vector3_Normalize (dir);
        a = SCE_Vector3_Distance (middle, middle2);
        d = (SCE_Vector3_Distance (p[0], p[1]) * 0.5);
        x = cos (asin (d / r)) - a / r;
        x *= r;

        SCE_Vector3_Operator2 (middle, +=, dir, *, x);
        SCE_Sphere_SetCenterv (sphere, middle);
    }
}


/**
 * \brief Extracts the base of a cube along a given axis
 * \param dir a direction vector that will be the z axis of the base
 * \param base resulting base
 *
 * Some decisions have to be made regarding the direction of the two
 * other axis of the base; this function tries to maintain these axis
 * consistent.
 * \todo I'd like to move it move it, I'd like to move it move it §
 */
static void SCE_Frustum_SliceBase (const SCE_TVector3 dir, SCE_TMatrix3 base)
{
    SCE_SPlane p;
    /* axis are in world space */
    SCE_TVector3 X, Z; /* {X, dir, Z} shall be an ORTHOGONAL BASE WESH BRO */
    SCE_TVector3 Y;

    SCE_Plane_SetFromPoint (&p, dir, 0.0, 0.0, 0.0);

    SCE_Vector3_Copy (Z, dir);
    /* check for special cases */
    if (SCE_Math_IsZero (dir[0]) && SCE_Math_IsZero (dir[1])) {
        /* colinear with Z axis */
        Z[1] += 42.0f;
    } else {
        Z[2] += 42.0f;
    }

    SCE_Plane_Project (&p, Z);
    SCE_Vector3_Normalize (Z);
    SCE_Vector3_Operator1v (Y, = -, dir);
    SCE_Vector3_Cross (X, Z, Y);
    SCE_Vector3_Normalize (X);

    /* axis are in camera space */
    SCE_Matrix3_Base (base, X, Z, Y);
}

/**
 * \brief
 * \param f a frustum
 * \param near distance between origin and the near clip plane of the slice
 * \param far distance between origin and the far clip plane of the slice
 * \param dir light direction, must be normalized
 * \param dist distance between the center of the slice and the light source
 * \param proj
 * \param cam
 */
void SCE_Frustum_Slice (const SCE_SFrustum *f, float near, float far,
                        SCE_TVector3 dir, float dist, SCE_TMatrix4 proj,
                        SCE_TMatrix4 cam)
{
    SCE_TMatrix3 base;
    SCE_SSphere sphere;
    SCE_TVector3 center;
    float radius;

    SCE_Frustum_ExtractBoundingSphere (f, near, far, &sphere);

    SCE_Sphere_GetCenterv (&sphere, center);
    radius = SCE_Sphere_GetRadius (&sphere);

    SCE_Frustum_SliceBase (dir, base);
    SCE_Matrix4_Identity (cam);
    SCE_Matrix4_CopyM3 (cam, base);

    SCE_Matrix4_SetTranslation (cam, center);
    SCE_Matrix4_MulTranslate (cam, 0.0, 0.0, dist);

    SCE_Matrix4_Identity (proj);
    SCE_Matrix4_Ortho (proj, 2.0 * radius, 2.0 * radius, 0.0, dist + radius);
}

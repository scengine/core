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
 
/* created: 08/01/2009
   updated: 29/10/2010 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCECollide.h"

/**
 * \file SCECollide.c
 * \copydoc collide
 * 
 * \file SCECollide.h
 * \copydoc collide
 */

/**
 * \defgroup collide Collider
 * \ingroup interface
 * \brief Collider module that manages collisions between some simple shapes
 */

/** @{ */

/**
 * \brief Checks if a point is inside a convex shape represented by a set
 * of planes.
 * \param planes a convex shape
 * \param n number of planes
 * \param x,y,z a point
 *
 * The normals of the planes must be oriented inside the convex shape.
 * \return SCE_TRUE if the point is inside the shape, SCE_FALSE otherwise
 * \sa SCE_Collide_PlanesWithPointv()
 */
int SCE_Collide_PlanesWithPoint (const SCE_SPlane *planes, size_t n,
                                 float x, float y, float z)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (SCE_Plane_DistanceToPoint (&planes[i], x, y, z) < 0.0f)
            return SCE_FALSE;
    }
    return SCE_TRUE;
}
/**
 * \brief Vectorial version of SCE_Collide_PlanesWithPoint()
 * \param planes a convex shape
 * \param n number of planes
 * \param p a point
 * \return SCE_TRUE if the point is inside the shape, SCE_FALSE otherwise
 * \sa SCE_Collide_PlanesWithPoint()
 */
int SCE_Collide_PlanesWithPointv (const SCE_SPlane *planes, size_t n,
                                  const SCE_TVector3 p)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (SCE_Plane_DistanceToPointv (&planes[i], p) < 0.0f)
            return SCE_FALSE;
    }
    return SCE_TRUE;
}

int SCE_Collide_PlanesWithBB (const SCE_SPlane *planes, size_t n,
                              const SCE_SBoundingBox *box)
{
    size_t i;
    unsigned int total = 0;
    float *points = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)box);
    for (i = 0; i < n; i++) {
        size_t j;
        unsigned int k = 0;
        for (j = 0; j < 8; j++) {
            if (SCE_Plane_DistanceToPointv (&planes[i], &points[j*3]) > 0.0f)
                k++;
        }
        if (k == 8)
            total++;
        else if (k == 0)
            return SCE_COLLIDE_OUT;
    }
    return (total == n ? SCE_COLLIDE_IN : SCE_COLLIDE_PARTIALLY);
}
int SCE_Collide_PlanesWithBBBool (const SCE_SPlane *planes, size_t n,
                                  const SCE_SBoundingBox *box)
{
    size_t i, j;
    float *points = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)box);
    for (i = 0; i < n; i++) {
        for (j = 0; j < 8; j++) {
            if (SCE_Plane_DistanceToPointv (&planes[i], &points[j*3]) > 0.0f)
                goto end;
        }
        return SCE_FALSE;
    end:
        continue;
    }
    return SCE_TRUE;
}
int SCE_Collide_PlanesWithBS (const SCE_SPlane *planes, size_t n,
                              const SCE_SBoundingSphere *sphere)
{
    float d;
    size_t i;
    unsigned int passed = 0;
    float *c, r;

    c = SCE_BoundingSphere_GetCenter ((SCE_SBoundingSphere*)sphere);
    r = SCE_BoundingSphere_GetRadius (sphere);

    /* works only with convex meshs */
    for (i = 0; i < n; i++) {
        d = SCE_Plane_DistanceToPointv (&planes[i], c);
        if (d >= r)
            passed++;
        else if (d < -r)        /* TODO: requires a normalized plane */
            return SCE_COLLIDE_OUT;
        else
            passed--;     /* the sphere can't be totally in (maybe partially) */
    }
    return (passed == n ? SCE_COLLIDE_IN : SCE_COLLIDE_PARTIALLY);
}
/**
 * \brief Works only on convex meshes
 */
int SCE_Collide_PlanesWithBSBool (const SCE_SPlane *planes, size_t n,
                                  const SCE_SBoundingSphere *sphere)
{
    size_t i;
    float *c, r;

    c = SCE_BoundingSphere_GetCenter ((SCE_SBoundingSphere*)sphere);
    r = SCE_BoundingSphere_GetRadius (sphere);

    /* works only with convex meshs */
    for (i = 0; i < n; i++) {
        if (SCE_Plane_DistanceToPointv (&planes[i], c) < -r)
            return SCE_FALSE;
    }
    return SCE_TRUE;
}

#if 0
int SCE_Collide_RectWithBS (SCE_SFloatRect *rect, SCE_SBoundingSphere *sphere)
{
    SCE_SFloatRect rect2;
    float r = SCE_BoundingSphere_GetRadius (sphere);
    rect2 = *rect;
    /* intrusion! */
    rect2.p1[0] -= r;
    rect2.p1[1] -= r;
    rect2.p2[0] += r;
    rect2.p2[1] += r;
    if (SCE_Rectangle_IsInfv (&rect2, SCE_BoundingSphere_GetCenter (sphere))) {
        if (SCE_Rectangle_IsInfv (&rect, SCE_BoundingSphere_GetCenter (sphere)))
            return SCE_COLLIDE_IN;
        else
            return SCE_COLLIDE_PARTIALLY;
    }
    return SCE_COLLIDE_OUT;
}
#endif

int SCE_Collide_AABBWithPoint (const SCE_SBoundingBox *box, float x, float y,
                               float z)
{
    float *p = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)box);
    if (x >= p[0] && x <= p[3] &&
        y >= p[1] && y <= p[10] &&
        z >= p[2] && z <= p[23])
        return SCE_COLLIDE_IN;
    return SCE_COLLIDE_OUT;
}
int SCE_Collide_AABBWithPointv (const SCE_SBoundingBox *box,
                                const SCE_TVector3 p)
{
    return SCE_Collide_AABBWithPoint (box, p[0], p[1], p[2]);
}
int SCE_Collide_AABBWithLine (const SCE_SBoundingBox *box, const SCE_SLine3 *l)
{
    return SCE_Collide_BBWithLine (box, l);
}
int SCE_Collide_AABBWithBS (const SCE_SBoundingBox *b,
                            const SCE_SBoundingSphere *s)
{
    float r, *c, *p;
    SCE_SSphere *sphere = SCE_BoundingSphere_GetSphere((SCE_SBoundingSphere*)s);
    r = sphere->radius;
    c = sphere->center;
    p = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)b);

    if (c[0] >= p[0]-r && c[0] <= p[3]+r &&
        c[1] >= p[1]-r && c[1] <= p[10]+r &&
        c[2] >= p[2]-r && c[2] <= p[23]+r) {
        if (c[0] >= p[0]+r && c[0] <= p[3]-r &&
            c[1] >= p[1]+r && c[1] <= p[10]-r &&
            c[2] >= p[2]+r && c[2] <= p[23]-r)
            return SCE_COLLIDE_IN;
        else
            return SCE_COLLIDE_PARTIALLY;
    }
    return SCE_COLLIDE_OUT;
}
int SCE_Collide_AABBWithBSBool (const SCE_SBoundingBox *b,
                                const SCE_SBoundingSphere *s)
{
    float r, *c, *p;
    SCE_SSphere *sphere = SCE_BoundingSphere_GetSphere((SCE_SBoundingSphere*)s);
    r = sphere->radius;
    c = sphere->center;
    p = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)b);

    if (c[0] >= p[0]-r && c[0] <= p[3]+r &&
        c[1] >= p[1]-r && c[1] <= p[10]+r &&
        c[2] >= p[2]-r && c[2] <= p[23]+r)
        return SCE_TRUE;
    else
        return SCE_FALSE;
}

int SCE_Collide_BBWithPoint (const SCE_SBoundingBox *box, float x, float y,
                             float z)
{
    size_t i;
    SCE_SPlane *planes = SCE_BoundingBox_GetPlanes ((SCE_SBoundingBox*)box);
    for (i = 0; i < 6; i++) {
        if (SCE_Plane_DistanceToPoint (&planes[i], x, y, z) > 0.0f)
            return SCE_FALSE;
    }
    return SCE_TRUE;
}
int SCE_Collide_BBWithPointv (const SCE_SBoundingBox *box, const SCE_TVector3 p)
{
    size_t i;
    SCE_SPlane *planes = SCE_BoundingBox_GetPlanes ((SCE_SBoundingBox*)box);
    for (i = 0; i < 6; i++) {
        if (SCE_Plane_DistanceToPointv (&planes[i], p) > 0.0f)
            return SCE_FALSE;
    }
    return SCE_TRUE;
}
int SCE_Collide_BBWithLine (const SCE_SBoundingBox *box, const SCE_SLine3 *l)
{
    size_t i, j, k[6] = {1, 2, 3, 4, 5, 6};
    SCE_TVector3 p;
    SCE_SPlane *planes = SCE_BoundingBox_GetPlanes ((SCE_SBoundingBox*)box);

    for (i = 0; i < 6; i++) {
        if (SCE_Plane_LineIntersection (&planes[i], l, p)) {
            int foo = SCE_TRUE;
            for (j = 0; j < 5; j++) {
                float a = SCE_Plane_DistanceToPointv (&planes[k[j]], p);
                if (a > 0.0f) {
                    foo = SCE_FALSE;
                    break;
                }
            }
            if (foo)
                return SCE_TRUE;
        }
        /* h4x, we need to skip the test with the current plane */
        k[i] = i;
    }
    return SCE_FALSE;
}
int SCE_Collide_BBWithBS (const SCE_SBoundingBox *box,
                          const SCE_SBoundingSphere *sphere)
{
    return SCE_Collide_PlanesWithBS (SCE_BoundingBox_GetPlanes(
                                         (SCE_SBoundingBox*)box), 6, sphere);
}
int SCE_Collide_BBWithBB (const SCE_SBoundingBox *box,
                          const SCE_SBoundingBox *box2)
{
    return SCE_Collide_PlanesWithBB (SCE_BoundingBox_GetPlanes(
                                         (SCE_SBoundingBox*)box), 6, box2);
}
int SCE_Collide_BBWithBBBool (const SCE_SBoundingBox *box,
                              const SCE_SBoundingBox *box2)
{
    return SCE_Collide_PlanesWithBBBool (SCE_BoundingBox_GetPlanes(
                                             (SCE_SBoundingBox*)box), 6, box2);
}

int SCE_Collide_BSWithPoint (const SCE_SBoundingSphere *s,
                             float x, float y, float z)
{
    SCE_TVector3 p;
    SCE_Vector3_Set (p, x, y, z);
    return SCE_Collide_BSWithPointv (s, p);
}
int SCE_Collide_BSWithPointv (const SCE_SBoundingSphere *s, const SCE_TVector3 p)
{
    float r;
    SCE_TVector3 diff;
    SCE_SSphere *sphere = SCE_BoundingSphere_GetSphere ((SCE_SBoundingSphere*)s);
    r = sphere->radius;
    SCE_Vector3_Operator2v (diff, =, sphere->center, -, p);
    return (SCE_Math_Fabsf (SCE_Vector3_Dot (diff, diff)) < r * r);
}
int SCE_Collide_BSWithBB (const SCE_SBoundingSphere *s,
                          const SCE_SBoundingBox *box)
{
    size_t i;
    unsigned int n = 0;
    float *points = SCE_BoundingBox_GetPoints ((SCE_SBoundingBox*)box);
    for (i = 0; i < 8; i++) {
        if (SCE_Collide_BSWithPointv (s, &points[i * 3]))
            n++;
    }
    if (n == 0) {
        return (SCE_Collide_BBWithBS (box, s) ?
                SCE_COLLIDE_PARTIALLY : SCE_COLLIDE_OUT);
    } else
        return (n == 8 ? SCE_COLLIDE_IN : SCE_COLLIDE_PARTIALLY);
}
int SCE_Collide_BSWithBS (const SCE_SBoundingSphere *s,
                          const SCE_SBoundingSphere *s2)
{
    float d, *c, *c2, r, r2;
    SCE_SSphere *sphere = SCE_BoundingSphere_GetSphere((SCE_SBoundingSphere*)s);
    SCE_SSphere *sphere2=SCE_BoundingSphere_GetSphere((SCE_SBoundingSphere*)s2);
    c = sphere->center;
    r = sphere->radius;
    c2 = sphere2->center;
    r2 = sphere2->radius;
    d = SCE_Math_Fabsf (SCE_Vector3_Dot (c, c2));
    r  = r - r2; r  *= r;
    r2 = r + r2; r2 *= r2;
    if (d <= r - r2)
        return SCE_COLLIDE_IN;
    else if (d < r + r2)
        return SCE_COLLIDE_PARTIALLY;
    else
        return SCE_COLLIDE_OUT;
}

int SCE_Collide_BCWithPoint (const SCE_SCone *c, float x, float y, float z)
{
    SCE_TVector3 p;
    SCE_Vector3_Set (p, x, y, z);
    return SCE_Collide_BCWithPointv (c, p);
}
int SCE_Collide_BCWithPointv (const SCE_SCone *c, const SCE_TVector3 p)
{
    SCE_SPlane plane;
    SCE_TVector3 pos, dir;
    float dist;
    SCE_Cone_GetPositionv (c, pos);
    SCE_Cone_GetOrientationv (c, dir);
    SCE_Plane_SetFromPointv (&plane, dir, pos);
    dist = SCE_Plane_DistanceToPointv (&plane, p);
    if (0.0 < dist && dist < SCE_Cone_GetHeight (c)) {
        float angle = SCE_Cone_GetAngle (c);
        SCE_Vector3_Operator2v (pos, =, p, -, pos);
        SCE_Vector3_Normalize (pos);
        return acos (SCE_Vector3_Dot (pos, dir)) < angle;
    }
    return SCE_FALSE;
}

/** @} */

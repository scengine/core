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
 
/* created: 28/02/2008
   updated: 16/06/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCECollide.h"
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
    /* right plane */
    SCE_Plane_Set (&f->planes[0],
                   clip[12]-clip[0], clip[13]-clip[1],
                   clip[14]-clip[2], clip[15]-clip[3]);
    /* left plane */
    SCE_Plane_Set (&f->planes[1],
                   clip[12]+clip[0], clip[13]+clip[1],
                   clip[14]+clip[2], clip[15]+clip[3]);
    /* bottom plane */
    SCE_Plane_Set (&f->planes[2],
                   clip[12]+clip[4], clip[13]+clip[5],
                   clip[14]+clip[6], clip[15]+clip[7]);
    /* top plane */
    SCE_Plane_Set (&f->planes[3],
                   clip[12]-clip[4], clip[13]-clip[5],
                   clip[14]-clip[6], clip[15]-clip[7]);
    /* far plane */
    SCE_Plane_Set (&f->planes[4],
                   clip[12]-clip[8], clip[13]-clip[9],
                   clip[14]-clip[10], clip[15]-clip[11]);
    /* near plane */
    SCE_Plane_Set (&f->planes[5],
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

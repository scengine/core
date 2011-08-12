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

/* created: 11/08/2011
   updated: 12/08/2011 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCECone.h"
#include "SCE/core/SCEConeGeometry.h"


int SCE_ConeGeom_Generate (const SCE_SCone *cone, SCEuint segments,
                           SCE_SGeometry *geom)
{
    int i;
    SCEvertices *vertices = NULL;
    SCEindices *indices = NULL, *index = NULL;
    float height, offset, radius;
    size_t n_vertices, n_indices;

    n_vertices = segments + 2;
    n_indices = 3 * segments * 2;

    if (!(vertices = SCE_malloc (n_vertices * 3 * sizeof *vertices)))
        goto fail;
    if (!(indices = SCE_malloc (n_indices * sizeof *indices)))
        goto fail;

    /* top */
    SCE_Cone_GetPositionv (cone, vertices);

    /* bottom */
    height = SCE_Cone_GetHeight (cone);
#if 0
    SCE_Vertor3_Operator2v (b, =, vertices, - height *, dir);
    SCE_Vector3_Copy (&vertices[(segments + 1) * 3], b);
#else
    SCE_Cone_GetPositionv (cone, &vertices[(n_vertices - 1) * 3]);
    vertices[(n_vertices - 1) * 3 + 2] = -height;
#endif

    /* ring */
    offset = 2.0 * M_PI / segments;
    radius = SCE_Cone_GetRadius (cone);
    for (i = 0; i < segments; i++) {
        float angle = offset * i;
        float x = SCE_Math_Cosf (angle) * radius;
        float y = SCE_Math_Sinf (angle) * radius;
        SCE_Vector3_Set (&vertices[(i + 1) * 3], x, y, -height);
    }

    /* indices */
    index = indices;
    for (i = 0; i < segments; i++) {
        index[0] = 0;
        index[1] = index[4] = i + 1;
        index[2] = index[3] = i + 2;
        index[5] = n_vertices - 1;
        index = &index[6];
    }

    index = &index[-6];
    index[2] = index[3] = 1;

    if (SCE_Geometry_SetData (geom, vertices, NULL, NULL, indices,
                              n_vertices, n_indices) < 0)
        goto fail;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

SCE_SGeometry* SCE_ConeGeom_Create (const SCE_SCone *cone, SCEuint segments)
{
    SCE_SGeometry *geom = NULL;

    if (!(geom = SCE_Geometry_Create ()))
        goto fail;
    SCE_Geometry_SetPrimitiveType (geom, SCE_TRIANGLES);
    if (SCE_ConeGeom_Generate (cone, segments, geom) < 0)
        goto fail;

    return geom;
fail:
    SCE_Geometry_Delete (geom);
    SCEE_LogSrc ();
    return NULL;
}

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

/* created: 02/06/2009
   updated: 03/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCESphereGeometry.h"

/**
 * \defgroup spheregeometry Sphere Geometry
 * \ingroup core
 * \internal
 * \brief 
 */

/** @{ */

/**
 * \internal
 * \brief Generates vertex positions and indices of a UV sphere
 * \param center,radius,segments,rings parameters that defines the sphere
 * to create. \p center can be NULL
 * \param pout vertices positions output (will allocate memory)
 * \param n_pos number of positions in \p pout
 * \param iout indices output (will allocate memory)
 * \sa SCE_SphereGeom_CreateUV(), SCE_SphereGeom_CreateDefaultUV()
 */
int
SCE_SphereGeom_GetUV (SCE_SSphere *sphere, SCEuint segments,
                      SCEuint rings, SCEvertices **pout, SCEuint *n_pos,
                      SCEindices **iout)
{
    unsigned int i, j;
    SCE_TVector3 c = {0.0f, 0.0f, 0.0f};
    int n_indices;
    SCEvertices *p = NULL;
    SCEindices *indices = NULL, *index = NULL;
    float *center;
    float radius;
    float seg_offset;
    float rin_offset = M_PI / (float)rings;
    seg_offset = 2.0 * M_PI / (float)segments;
    *n_pos = segments * rings + 2;
    center = sphere->center;
    radius = sphere->radius;
    if (!(p = SCE_malloc (*n_pos * 3 * sizeof *p)))
        goto failure;
    n_indices = segments * rings * 6;
    if (!(indices = SCE_malloc (n_indices * sizeof *indices)))
        goto failure;

    if (center)
        SCE_Vector3_Copy (c, center);

    SCE_Vector3_Set (&p[segments * rings * 3], 0.0f, 0.0f, radius);
    SCE_Vector3_Set (&p[segments * rings * 3 + 3], 0.0f, 0.0f, -radius);

    for (i = 0; i < rings; i++) {
        SCE_TVector3 v1;
        float angle1 = rin_offset * i;
        v1[2] = SCE_Math_Cosf (angle1) * radius;
        v1[0] = v1[1] = SCE_Math_Sinf (angle1) * radius;
        for (j = 0; j < segments; j++) {
            SCE_TVector3 v2;
            float angle2 = seg_offset * j;
            v2[0] = SCE_Math_Cosf (angle2);
            v2[1] = SCE_Math_Sinf (angle2);
            v2[2] = 1.0f;
            SCE_Vector3_Operator1v (v2, *=, v1);
            SCE_Vector3_Copy (&p[(i * segments + j) * 3], v2);
        }
    }

    index = indices;
    /* middle */
    for (i = 1; i < rings; i++) {
        for (j = 0; j < segments; j++) {
            unsigned int j1 = (j == 0 ? segments - 1 : j - 1);
            /* make two triangles */
            index[0] = i * segments + j;
            index[1] = (i - 1) * segments + j1;
            index[2] = i * segments + j1;

            index[3] = i * segments + j;
            index[4] = (i - 1) * segments + j;
            index[5] = (i - 1) * segments + j1;
            index = &index[6];
        }
    }
    /* top/middle */
    for (i = 0; i < segments; i++) {
        unsigned int i1 = (i == 0 ? segments - 1 : i - 1);
        /* upper triangle */
        index[0] = segments * rings;
        index[2] = i;
        index[1] = i1;
        /* lower triangle */
        index[3] = segments * rings + 1;
        index[4] = segments * (rings - 1) + i;
        index[5] = segments * (rings - 1) + i1;
        index = &index[6];
    }

    *pout = p;
    *iout = indices;    
    return n_indices;
failure:
    SCE_free (p);
    SCE_free (indices);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Generates a geometry containing a sphere defined as requested
 * via \p center, \p radius, \p segments and \p rings
 * \sa SCE_SphereGeom_CreateUV()
 */
int SCE_SphereGeom_GenerateUV (SCE_SSphere *sphere, SCEuint segments,
                               SCEuint rings, SCE_SGeometry *geom)
{
    SCEvertices *pos = NULL;
    SCEindices *indices = NULL;
    int n_indices = 0;
    SCEuint n_vertices = 0;

    if ((n_indices = SCE_SphereGeom_GetUV (sphere, segments, rings,
                                           &pos, &n_vertices, &indices)) < 0)
        goto fail;
    if (SCE_Geometry_SetData (geom, pos, NULL, NULL, indices, n_vertices,
                              n_indices) < 0)
        goto fail;
    return SCE_OK;
fail:
    SCE_free (pos);
    SCE_free (indices);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Creates an UV sphere geometry
 * \param center,radius,segments,rings parameters that defines the sphere
 * to create. \p center can be NULL
 * \sa SCE_SphereGeom_CreateDefaultUV(), SCE_SphereGeom_GenerateUV()
 */
SCE_SGeometry* SCE_SphereGeom_CreateUV (SCE_SSphere *sphere,
                                        SCEuint segments, SCEuint rings)
{
    SCE_SGeometry *geom = NULL;
    if (!(geom = SCE_Geometry_Create ()))
        goto fail;
    SCE_Geometry_SetPrimitiveType (geom, SCE_TRIANGLES);
    if (SCE_SphereGeom_GenerateUV (sphere, segments, rings, geom) < 0)
        goto fail;
    return geom;
fail:
    SCE_Geometry_Delete (geom);
    SCEE_LogSrc ();
    return NULL;
}

/**
 * \brief Creates an UV sphere geometry
 * \param center,radius defines the sphere to create, \p center can be NULL
 *
 * The segments and the rings of the created sphere are
 * SCE_SPHEREGEOMETRY_SEGMENTS and SCE_SPHEREGEOMETRY_RINGS
 * \sa SCE_SphereGeom_CreateUV(), SCE_SphereGeom_GenerateUV()
 */
SCE_SGeometry* SCE_SphereGeom_CreateDefaultUV (SCE_SSphere *sphere)
{
    return SCE_SphereGeom_CreateUV (sphere, SCE_SPHEREGEOMETRY_SEGMENTS,
                                    SCE_SPHEREGEOMETRY_RINGS);
}

/** @} */

/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2013  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 07/08/2009
   updated: 06/04/2013 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCEBoxGeometry.h"

static int is_init = SCE_FALSE;

#define p0 -1.0f, -1.0f, -1.0f
#define p1  1.0f, -1.0f, -1.0f
#define p2  1.0f,  1.0f, -1.0f
#define p3 -1.0f,  1.0f, -1.0f
#define p4 -1.0f,  1.0f,  1.0f
#define p5  1.0f,  1.0f,  1.0f
#define p6  1.0f, -1.0f,  1.0f
#define p7 -1.0f, -1.0f,  1.0f

#define n_px  1.0f,  0.0f,  0.0f
#define n_nx -1.0f,  0.0f,  0.0f
#define n_py  0.0f,  1.0f,  0.0f
#define n_ny  0.0f, -1.0f,  0.0f
#define n_pz  0.0f,  0.0f,  1.0f
#define n_nz  0.0f,  0.0f, -1.0f

/* ccw triangles */
static SCEvertices pos_indiv_triangle[] = {
    /* front Z */
    p0, p3, p2, p2, p1, p0,
    /* back Z */
    p4, p7, p6, p6, p5, p4,
    /* front X */
    p0, p7, p4, p4, p3, p0,
    /* back X */
    p1, p2, p5, p5, p6, p1,
    /* front Y */
    p0, p1, p6, p6, p7, p0,
    /* back Y */
    p2, p3, p4, p4, p5, p2
};

static SCEvertices nor_indiv_triangle[] = {
    /* front Z */
    n_nz, n_nz, n_nz, n_nz, n_nz, n_nz,
    /* back Z */
    n_pz, n_pz, n_pz, n_pz, n_pz, n_pz,
    /* front X */
    n_nx, n_nx, n_nx, n_nx, n_nx, n_nx,
    /* back X */
    n_px, n_px, n_px, n_px, n_px, n_px,
    /* front Y */
    n_ny, n_ny, n_ny, n_ny, n_ny, n_ny,
    /* back Y */
    n_py, n_py, n_py, n_py, n_py, n_py
};

static SCEvertices texcoord_interior_triangle[] = {
    /* front Z */
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    /* back Z */
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    /* front X */
    1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    /* back X */
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    /* front Y */
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    /* back Y */
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

static SCEvertices texcoord_exterior_triangle[] = {
    /* front Z */
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    /* back Z */
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    /* front X */
    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    /* back X */
    1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    /* front Y */
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    /* back Y */
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
};

static SCEvertices texcoord_cubemap[] = {
    -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f
};

static SCEindices indices_lines[] = {
    0, 1,  1, 2,  2, 3,  3, 0,
    3, 4,  4, 7,  7, 6,  6, 5,
    5, 4,  5, 2,  6, 1,  0, 7
};

/* cw triangles */
static SCEindices indices_triangles[] = {
    1, 0, 2,  2, 0, 3,
    3, 0, 4,  4, 0, 7,
    7, 0, 6,  6, 0, 1,
    1, 2, 6,  2, 5, 6,
    5, 2, 3,  3, 4, 5,
    4, 7, 5,  7, 6, 5
};


int SCE_Init_BoxGeom (void)
{
    size_t i;
    if (is_init)
        return SCE_OK;
    for (i = 0; i < 8; i++)
        SCE_Vector3_Normalize (&texcoord_cubemap[i * 3]);
    is_init = SCE_TRUE;
    return SCE_OK;
}
void SCE_Quit_BoxGeom (void)
{
    is_init = SCE_FALSE;
}


static int SCE_BoxGeom_GenPoints (SCE_SBox *box, SCE_EBoxGeomTexCoordMode mode,
                                  SCE_SGeometry *geom)
{
    SCE_SGeometryArray array;

    SCE_Geometry_SetNumVertices (geom, 8);
    /* no texturing, except in case of cubemap */
    if (mode == SCE_BOX_CUBEMAP_TEXCOORD) {
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayData (&array, SCE_TEXCOORD0, SCE_VERTICES_TYPE,
                                   0, 3, texcoord_cubemap, SCE_FALSE);
        if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
            goto fail;
    }
    SCE_Geometry_InitArray (&array);
    SCE_Geometry_SetArrayData (&array, SCE_POSITION, SCE_VERTICES_TYPE, 0, 3,
                               SCE_Box_GetPoints (box), SCE_FALSE);
    if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
        goto fail;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

static int SCE_BoxGeom_GenLines (SCE_SBox *box, SCE_EBoxGeomTexCoordMode mode,
                                 SCE_SGeometry *geom)
{
    SCE_SGeometryArray array;
    /* TODO: texturing not supported yet */
    SCE_Geometry_SetNumVertices (geom, 8);
    SCE_Geometry_SetNumIndices (geom, 24);
#if 0
    switch (mode) {
    case 
        }
#endif
    /* ... just cubemap */
    if (mode == SCE_BOX_CUBEMAP_TEXCOORD) {
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayData (&array, SCE_TEXCOORD0, SCE_VERTICES_TYPE, 0,
                                   3, texcoord_cubemap, SCE_FALSE);
        if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
            goto fail;
    }
    SCE_Geometry_InitArray (&array);
    SCE_Geometry_SetArrayData (&array, SCE_POSITION, SCE_VERTICES_TYPE, 0,
                               3, SCE_Box_GetPoints (box), SCE_FALSE);
    if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
        goto fail;
    SCE_Geometry_InitArray (&array);
    SCE_Geometry_SetArrayIndices (&array, SCE_INDICES_TYPE,
                                  indices_lines, SCE_FALSE);
    if (!SCE_Geometry_SetIndexArrayDupDup (geom, &array, SCE_FALSE))
        goto fail;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


static void SCE_BoxGeom_MulIndiv (SCEvertices *v, SCE_SBox *box)
{
    SCE_TVector3 coeff, center;
    size_t i;
    coeff[0] = SCE_Box_GetWidth (box) * 0.5f;
    coeff[1] = SCE_Box_GetHeight (box) * 0.5f;
    coeff[2] = SCE_Box_GetDepth (box) * 0.5f;
    SCE_Box_GetCenterv (box, center);
    for (i = 0; i < 24; i++)
        SCE_Vector3_Operator2v (&v[i * 3], *=, coeff, +, center);
}
static int SCE_BoxGeom_GenTriangles (SCE_SBox *box,
                                     SCE_EBoxGeomTexCoordMode mode,
                                     SCE_EBoxGeomNormalMode nmode,
                                     SCE_SGeometry *geom)
{
    SCEvertices *v = NULL, *t = NULL, *n = NULL;
    SCEindices *i = NULL;
    int indiv = SCE_TRUE;
    int t_size = 2;

    SCE_Geometry_SetNumVertices (geom, 36);
    v = pos_indiv_triangle;

    switch (mode) {
    case SCE_BOX_EXTERIOR_TEXCOORD:
        t = texcoord_exterior_triangle;
        break;
    case SCE_BOX_INTERIOR_TEXCOORD:
        t = texcoord_interior_triangle;
        break;
    case SCE_BOX_CUBEMAP_TEXCOORD:
        t = texcoord_cubemap;
    default:;
    }

    switch (nmode) {
    case SCE_BOX_SMOOTH_NORMALS:
        /* TODO: not supported yet */
        n = NULL; /*nor_smoothed_triangle;*/
        break;
    case SCE_BOX_SHARP_NORMALS:
        n = nor_indiv_triangle;
        break;
    default:;
    }


    if (mode == SCE_BOX_NONE_TEXCOORD && nmode != SCE_BOX_SHARP_NORMALS) {
        SCE_Geometry_SetNumVertices (geom, 8);
        SCE_Geometry_SetNumIndices (geom, 36);
        v = SCE_Box_GetPoints (box);
        if (nmode != SCE_BOX_NONE_NORMALS)
            n = texcoord_cubemap;
        i = indices_triangles;
        indiv = SCE_FALSE;
        t_size = 3;
    }


    {
        SCE_SGeometryArray array, *ap = NULL;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayData (&array, SCE_POSITION, SCE_VERTICES_TYPE,
                                   0, 3, v, SCE_FALSE);
        ap = SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE);
        if (!ap)
            goto fail;
        if (indiv)
            SCE_BoxGeom_MulIndiv (SCE_Geometry_GetData (ap), box);
        if (t) {
            SCE_Geometry_InitArray (&array);
            SCE_Geometry_SetArrayData (&array, SCE_TEXCOORD0, SCE_VERTICES_TYPE,
                                       0, t_size, t, SCE_FALSE);
            if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
                goto fail;
        }
        if (n) {
            SCE_Geometry_InitArray (&array);
            SCE_Geometry_SetArrayData (&array, SCE_NORMAL, SCE_VERTICES_TYPE,
                                       0, 3, n, SCE_FALSE);
            if (!SCE_Geometry_AddArrayDupDup (geom, &array, SCE_FALSE))
                goto fail;
        }
        if (i) {
            SCE_Geometry_InitArray (&array);
            SCE_Geometry_SetArrayIndices (&array, SCE_INDICES_TYPE,
                                          i, SCE_FALSE);
            if (!SCE_Geometry_SetIndexArrayDupDup (geom, &array, SCE_FALSE))
                goto fail;
        }
    }
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Generate a box into the given geometry
 * \param box the box model
 * \param prim the primitive type of the generated box
 * \param mode the generation mode (texture coordinate usage)
 * \param geom where store the geometry
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \sa SCE_BoxGeom_Create()
 */
int SCE_BoxGeom_Generate (SCE_SBox *box, SCE_EPrimitiveType prim,
                          SCE_EBoxGeomTexCoordMode mode,
                          SCE_EBoxGeomNormalMode nmode, SCE_SGeometry *geom)
{
    switch (prim) {
    case SCE_POINTS:
        SCE_Geometry_SetPrimitiveType (geom, SCE_POINTS);
        if (SCE_BoxGeom_GenPoints (box, mode, geom) < 0)
            goto fail;
        break;
    case SCE_LINES:
        SCE_Geometry_SetPrimitiveType (geom, SCE_LINES);
        if (SCE_BoxGeom_GenLines (box, mode, geom) < 0)
            goto fail;
        break;
    case SCE_TRIANGLES:
        SCE_Geometry_SetPrimitiveType (geom, SCE_TRIANGLES);
        if (SCE_BoxGeom_GenTriangles (box, mode, nmode, geom) < 0)
            goto fail;
    }
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Creates a geometry containing a box
 * \param box the box model
 * \param prim the primitive type of the generated box
 * \param mode the generation mode (texture coordinate usage)
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \sa SCE_BoxGeom_Generate()
 */
SCE_SGeometry* SCE_BoxGeom_Create (SCE_SBox *box, SCE_EPrimitiveType prim,
                                   SCE_EBoxGeomTexCoordMode mode,
                                   SCE_EBoxGeomNormalMode nmode)
{
    SCE_SGeometry * geom = NULL;
    if (!(geom = SCE_Geometry_Create ()))
        SCEE_LogSrc ();
    else {
        if (SCE_BoxGeom_Generate (box, prim, mode, nmode, geom) < 0) {
            SCE_Geometry_Delete (geom), geom = NULL;
            SCEE_LogSrc ();
        }
    }
    return geom;
}

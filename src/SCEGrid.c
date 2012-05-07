/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2012  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 21/01/2012
   updated: 17/03/2012 */

#include <string.h>             /* memcpy() */
#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCETextureData.h"
#include "SCE/core/SCEGrid.h"

void SCE_Grid_Init (SCE_SGrid *grid)
{
    grid->data = NULL;
    grid->width = grid->height = grid->depth = 0;
    grid->wrap_x = grid->wrap_y = grid->wrap_z = 0;
    grid->size = 1;
    grid->built = SCE_FALSE;
    grid->udata = NULL;
}
void SCE_Grid_Clear (SCE_SGrid *grid)
{
    SCE_free (grid->data);
}
SCE_SGrid* SCE_Grid_Create (void)
{
    SCE_SGrid *grid = NULL;
    if (!(grid = SCE_malloc (sizeof *grid)))
        SCEE_LogSrc ();
    else
        SCE_Grid_Init (grid);
    return grid;
}
void SCE_Grid_Delete (SCE_SGrid *grid)
{
    if (grid) {
        SCE_Grid_Clear (grid);
        SCE_free (grid);
    }
}


/**
 * \brief Copies the data from one grid to another,
 * assumes dimensions are identical
 * \param dst destination grid
 * \param src source grid
 */
void SCE_Grid_CopyData (SCE_SGrid *dst, const SCE_SGrid *src)
{
    memcpy (dst->data, src->data, SCE_Grid_GetSize (dst));
}


void SCE_Grid_SetPointSize (SCE_SGrid *grid, size_t size)
{
    grid->size = size;
}
void SCE_Grid_SetDimensions (SCE_SGrid *grid, int width, int height, int depth)
{
    grid->width = width;
    grid->height = height;
    grid->depth = depth;
}
void SCE_Grid_SetWidth (SCE_SGrid *grid, int width)
{
    grid->width = width;
}
void SCE_Grid_SetHeight (SCE_SGrid *grid, int height)
{
    grid->height = height;
}
void SCE_Grid_SetDepth (SCE_SGrid *grid, int depth)
{
    grid->depth = depth;
}

size_t SCE_Grid_GetPointSize (const SCE_SGrid *grid)
{
    return grid->size;
}
void* SCE_Grid_GetRaw (SCE_SGrid *grid)
{
    return grid->data;
}
int SCE_Grid_GetWidth (const SCE_SGrid *grid)
{
    return grid->width;
}
int SCE_Grid_GetHeight (const SCE_SGrid *grid)
{
    return grid->height;
}
int SCE_Grid_GetDepth (const SCE_SGrid *grid)
{
    return grid->depth;
}
int SCE_Grid_GetNumPoints (const SCE_SGrid *grid)
{
    return grid->width * grid->height * grid->depth;
}
size_t SCE_Grid_GetSize (const SCE_SGrid *grid)
{
    return grid->size * SCE_Grid_GetNumPoints (grid);
}

void SCE_Grid_SetData (SCE_SGrid *grid, void *udata)
{
    grid->udata = udata;
}
void* SCE_Grid_GetData (SCE_SGrid *grid)
{
    return grid->udata;
}


static void SCE_Grid_FillupZeros (SCE_SGrid *grid)
{
    unsigned char buf[256] = {0};
    int x, y, z;

    for (z = 0; z < grid->depth; z++) {
        for (y = 0; y < grid->height; y++) {
            for (x = 0; x < grid->width; x++)
                SCE_Grid_SetPoint (grid, x, y, z, buf);
        }
    }
}

int SCE_Grid_Build (SCE_SGrid *grid)
{
    size_t size;

    if (grid->built)
        return SCE_OK;

    size = SCE_Grid_GetSize (grid);
    if (!(grid->data = SCE_malloc (size)))
        goto fail;

    SCE_Grid_FillupZeros (grid);

    grid->built = SCE_TRUE;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


size_t SCE_Grid_GetOffset (const SCE_SGrid *grid, int x, int y, int z)
{
    size_t offset;
    x = SCE_Math_Ring (x + grid->wrap_x, grid->width);
    y = SCE_Math_Ring (y + grid->wrap_y, grid->height);
    z = SCE_Math_Ring (z + grid->wrap_z, grid->depth);
    offset = grid->width * (grid->height * z + y) + x;
    return offset;
}

void SCE_Grid_GetPoint (const SCE_SGrid *grid, int x, int y, int z, void *p)
{
    unsigned char *src = NULL;
    size_t offset = SCE_Grid_GetOffset (grid, x, y, z);
    offset *= grid->size;
    src = grid->data;
    memcpy (p, &src[offset], grid->size);
}
void SCE_Grid_SetPoint (SCE_SGrid *grid, int x, int y, int z, void *p)
{
    unsigned char *dst = NULL;
    size_t offset = SCE_Grid_GetOffset (grid, x, y, z);
    offset *= grid->size;
    dst = grid->data;
    memcpy (&dst[offset], p, grid->size);
}


int SCE_Grid_ToGeometry (const SCE_SGrid *grid, SCE_SGeometry *geom)
{
    return SCE_Grid_ToGeometryDiv (grid, geom, grid->width,
                                   grid->height, grid->depth);
}
int SCE_Grid_ToGeometryDiv (const SCE_SGrid *grid, SCE_SGeometry *geom,
                            int w, int h, int d)
{
    SCEvertices *vertices = NULL;
    size_t n_points;
    int x, y, z;
    SCE_SGrid g;
    const int size = 3;

    g = *grid;
    g.wrap_x = g.wrap_y = g.wrap_z = 0;
    n_points = SCE_Grid_GetNumPoints (&g);

    if (!(vertices = SCE_malloc (size * n_points * sizeof *vertices)))
        goto fail;

    for (z = 0; z < g.depth; z++) {
        for (y = 0; y < g.height; y++) {
            for (x = 0; x < g.width; x++) {
                size_t offset = SCE_Grid_GetOffset (&g, x, y, z);
                offset *= size;
                vertices[offset + 0] = (float)x / w;
                vertices[offset + 1] = (float)y / h;
                vertices[offset + 2] = (float)z / d;
            }
        }
    }

    if (!SCE_Geometry_AddNewArray (geom, SCE_POSITION, SCE_VERTICES_TYPE,
                                   0, size, vertices, SCE_TRUE))
        goto fail;
    SCE_Geometry_SetNumVertices (geom, n_points);
    SCE_Geometry_SetPrimitiveType (geom, SCE_POINTS);

    return SCE_OK;
fail:
    SCE_free (vertices);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

SCE_SGeometry* SCE_Grid_CreateGeometryFrom (const SCE_SGrid *grid)
{
    return SCE_Grid_CreateGeometryFromDiv (grid, grid->width, grid->height,
                                           grid->depth);
}
SCE_SGeometry* SCE_Grid_CreateGeometryFromDiv (const SCE_SGrid *grid,
                                               int w, int h, int d)
{
    SCE_SGeometry *geom = NULL;
    if (!(geom = SCE_Geometry_Create ()))
        goto fail;
    if (SCE_Grid_ToGeometryDiv (grid, geom, w, h, d) < 0)
        goto fail;
    return geom;
fail:
    SCE_Geometry_Delete (geom);
    SCEE_LogSrc ();
    return NULL;
}


void SCE_Grid_ToTexture (const SCE_SGrid *grid, SCE_STexData *tex,
                         SCE_EPixelFormat pxf, SCE_EType type)
{
    SCE_TexData_SetDataType (tex, type);
    SCE_TexData_SetDimensions (tex, grid->width, grid->height, grid->depth);
    SCE_TexData_SetPixelFormat (tex, pxf);
    SCE_TexData_SetDataFormat (tex, SCE_IMAGE_RED);
    SCE_TexData_SetType (tex, SCE_IMAGE_3D);
    SCE_TexData_SetData (tex, grid->data, SCE_FALSE);
}


/* TODO: update this, use a more generic design like in VStore */
void SCE_Grid_UpdateFace (SCE_SGrid *grid, SCE_EBoxFace f, const void *data)
{
    int x, y, z;
    unsigned char buffer[64] = {0};   /* let's hope 64 is enough */

    SCE_SGrid g;

    SCE_Grid_Init (&g);
    SCE_Grid_SetDimensions (&g, grid->width, grid->height, grid->depth);
    g.size = grid->size;
    g.data = data;

    x = y = z = 0;

    switch (f) {
    case SCE_BOX_NEGX:
        x = grid->width - 1;
    case SCE_BOX_POSX:
        g.width = 1;
        for (z = 0; z < grid->depth; z++) {
            for (y = 0; y < grid->height; y++) {
                /* TODO: use of temporary buffer isn't optimized */
                SCE_Grid_GetPoint (&g, 0, y, z, buffer);
                SCE_Grid_SetPoint (grid, x, y, z, buffer);
            }
        }
        break;

    case SCE_BOX_NEGY:
        y = grid->height - 1;
    case SCE_BOX_POSY:
        g.height = 1;
        for (z = 0; z < grid->depth; z++) {
            for (x = 0; x < grid->width; x++) {
                SCE_Grid_GetPoint (&g, x, 0, z, buffer);
                SCE_Grid_SetPoint (grid, x, y, z, buffer);
            }
        }
        break;

    case SCE_BOX_NEGZ:
        z = grid->depth - 1;
    case SCE_BOX_POSZ:
        g.depth = 1;
        for (y = 0; y < grid->height; y++) {
            for (x = 0; x < grid->width; x++) {
                SCE_Grid_GetPoint (&g, x, y, 0, buffer);
                SCE_Grid_SetPoint (grid, x, y, z, buffer);
            }
        }
        break;
    }

    switch (f) {
    case SCE_BOX_NEGX: grid->wrap_x--; break;
    case SCE_BOX_POSX: grid->wrap_x++; break;
    case SCE_BOX_NEGY: grid->wrap_y--; break;
    case SCE_BOX_POSY: grid->wrap_y++; break;
    case SCE_BOX_NEGZ: grid->wrap_z--; break;
    case SCE_BOX_POSZ: grid->wrap_z++; break;
    }
}

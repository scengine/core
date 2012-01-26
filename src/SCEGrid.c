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
   updated: 24/01/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCETextureData.h"
#include "SCE/core/SCEGrid.h"

void SCE_Grid_Init (SCE_SGrid *grid)
{
    grid->data = NULL;
    grid->width = grid->height = grid->depth = 0;
    grid->wrap_x = grid->wrap_y = grid->wrap_z = 0;
    grid->type = SCE_BYTE;
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

void SCE_Grid_SetType (SCE_SGrid *grid, SCE_EType type)
{
    grid->type = type;
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

SCE_EType SCE_Grid_GetType (const SCE_SGrid *grid)
{
    return grid->type;
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
    return SCE_Type_Sizeof (grid->type) * SCE_Grid_GetNumPoints (grid);
}

void SCE_Grid_SetData (SCE_SGrid *grid, void *udata)
{
    grid->udata = udata;
}
void* SCE_Grid_GetData (SCE_SGrid *grid)
{
    return grid->udata;
}


int SCE_Grid_Build (SCE_SGrid *grid)
{
    size_t size;

    if (grid->built)
        return SCE_OK;

    size = SCE_Grid_GetSize (grid);
    if (!(grid->data = SCE_malloc (size)))
        goto fail;

    /* TODO: fillup the grid with something */

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


#define ring(x, w) ((((x) % (w)) + (w)) % (w))

size_t SCE_Grid_GetOffset (const SCE_SGrid *grid, int x, int y, int z)
{
    size_t offset;
    x = ring (x + grid->wrap_x, grid->width);
    y = ring (y + grid->wrap_y, grid->height);
    z = ring (z + grid->wrap_z, grid->depth);
    offset = grid->width * (grid->height * z + y) + x;
    return offset;
}

void SCE_Grid_GetPoint (const SCE_SGrid *grid, int x, int y, int z, void *p)
{
    unsigned char *src = NULL;
    size_t offset = SCE_Grid_GetOffset (grid, x, y, z);
    offset *= SCE_Type_Sizeof (grid->type);
    src = grid->data;
    memcpy (p, &src[offset], SCE_Type_Sizeof (grid->type));
}
void SCE_Grid_SetPoint (SCE_SGrid *grid, int x, int y, int z, void *p)
{
    unsigned char *dst = NULL;
    size_t offset = SCE_Grid_GetOffset (grid, x, y, z);
    offset *= SCE_Type_Sizeof (grid->type);
    dst = grid->data;
    memcpy (&dst[offset], p, SCE_Type_Sizeof (grid->type));
}


int SCE_Grid_ToGeometry (const SCE_SGrid *grid, SCE_SGeometry *geom)
{
    SCEvertices *vertices = NULL;
    size_t n_points;
    int x, y, z;
    SCE_SGrid g;
    const int size = 3;

    g = *grid;
    g.width--; g.height--; g.depth--;
    g.wrap_x = g.wrap_y = g.wrap_z = 0;
    n_points = SCE_Grid_GetNumPoints (&g);

    if (!(vertices = SCE_malloc (size * n_points * sizeof *vertices)))
        goto fail;

    for (z = 0; z < g.depth; z++) {
        for (y = 0; y < g.height; y++) {
            for (x = 0; x < g.width; x++) {
                size_t offset = SCE_Grid_GetOffset (&g, x, y, z);
                offset *= size;
                vertices[offset + 0] = (float)x / grid->width;
                vertices[offset + 1] = (float)y / grid->height;
                vertices[offset + 2] = (float)z / grid->depth;
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
    SCE_SGeometry *geom = NULL;
    if (!(geom = SCE_Geometry_Create ()))
        goto fail;
    if (SCE_Grid_ToGeometry (grid, geom) < 0)
        goto fail;
    return geom;
fail:
    SCE_Geometry_Delete (geom);
    SCEE_LogSrc ();
    return NULL;
}


void SCE_Grid_ToTexture (const SCE_SGrid *grid, SCE_STexData *tex,
                         SCE_EPixelFormat pxf)
{
    SCE_TexData_SetDataType (tex, grid->type);
    SCE_TexData_SetDimensions (tex, grid->width, grid->height, grid->depth);
    SCE_TexData_SetPixelFormat (tex, pxf);
    SCE_TexData_SetDataFormat (tex, SCE_IMAGE_RED);
    SCE_TexData_SetType (tex, SCE_IMAGE_3D);
    SCE_TexData_SetData (tex, grid->data, SCE_FALSE);
}


void SCE_Grid_UpdateFace (SCE_SGrid *grid, SCE_EBoxFace f, void *data)
{
    int x, y, z;
    unsigned char buffer[64] = {0};   /* let's hope 64 is enough */

    SCE_SGrid g;

    SCE_Grid_Init (&g);
    SCE_Grid_SetDimensions (&g, grid->width, grid->height, grid->depth);
    g.type = grid->type;
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

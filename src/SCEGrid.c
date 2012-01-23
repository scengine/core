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
   updated: 22/01/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEGrid.h"

void SCE_Grid_Init (SCE_SGrid *grid)
{
    grid->data = NULL;
    grid->width = grid->height = grid->depth = 0;
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


size_t SCE_Grid_GetOffset (const SCE_SGrid *grid, int x, int y, int z)
{
    size_t offset;
    offset = grid->height * (grid->width * z + y) + x;
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
void SCE_Grid_SetPoint (const SCE_SGrid *grid, int x, int y, int z, void *p)
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
    const int size = 3;

    n_points = (grid->width - 1) * (grid->height - 1) * (grid->depth - 1);

    if (!(vertices = SCE_malloc (size * n_points * sizeof *vertices)))
        goto fail;

    for (z = 0; z < grid->depth - 1; z++) {
        for (y = 0; y < grid->height - 1; y++) {
            for (x = 0; x < grid->width - 1; x++) {
                char buffer[16] = {0};
                size_t offset = SCE_Grid_GetOffset (grid, x, y, z);
                offset *= size;
                SCE_Grid_GetPoint (grid, x, y, z, buffer);
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

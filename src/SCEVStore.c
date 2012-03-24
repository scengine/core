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

/* created: 18/03/2012
   updated: 24/03/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVStore.h"

static void SCE_VStore_InitLevel (SCE_SVoxelStorageLevel *sl)
{
    sl->data = NULL;
    sl->width = sl->height = sl->depth = 0;
}
static void SCE_VStore_ClearLevel (SCE_SVoxelStorageLevel *sl)
{
    SCE_free (sl->data);
}

void SCE_VStore_Init (SCE_SVoxelStorage *vs)
{
    int i;
    for (i = 0; i < SCE_MAX_VOXEL_STORAGE_LOD; i++)
        SCE_VStore_InitLevel (&vs->levels[i]);
    vs->n_lod = 1;
    vs->data_size = 0;
    vs->width = vs->height = vs->depth = 0;
    vs->vacuum = NULL;
    for (i = 0; i < SCE_MAX_UPDATE_ZONES; i++)
        SCE_Rectangle3_Init (&vs->zones[i]);
    vs->last = vs->first = 0;
}
void SCE_VStore_Clear (SCE_SVoxelStorage *vs)
{
    int i;
    for (i = 0; i < SCE_MAX_VOXEL_STORAGE_LOD; i++)
        SCE_VStore_ClearLevel (&vs->levels[i]);
    SCE_free (vs->vacuum);
}
SCE_SVoxelStorage* SCE_VStore_Create (void)
{
    SCE_SVoxelStorage *vs = NULL;
    if (!(vs = SCE_malloc (sizeof *vs)))
        SCEE_LogSrc ();
    else
        SCE_VStore_Init (vs);
    return vs;
}
void SCE_VStore_Delete (SCE_SVoxelStorage *vs)
{
    if (vs) {
        SCE_VStore_Clear (vs);
        SCE_free (vs);
    }
}


void SCE_VStore_SetDataSize (SCE_SVoxelStorage *vs, size_t size)
{
    vs->data_size = size;
}
void SCE_VStore_SetDimensions (SCE_SVoxelStorage *vs, SCEuint w, SCEuint h,
                               SCEuint d)
{
    SCEuint i;

    for (i = 0; i < SCE_MAX_VOXEL_STORAGE_LOD; i++) {
        vs->levels[i].width = w;
        vs->levels[i].height = h;
        vs->levels[i].depth = d;
        w = w / 2 + 1;
        h = h / 2 + 1;
        d = d / 2 + 1;
    }
}
void SCE_VStore_SetNumLevels (SCE_SVoxelStorage *vs, SCEuint n)
{
    vs->n_lod = MIN (n, SCE_MAX_VOXEL_STORAGE_LOD);
}

SCEuint SCE_VStore_GetWidth (const SCE_SVoxelStorage *vs)
{
    return vs->levels[0].width;
}
SCEuint SCE_VStore_GetHeight (const SCE_SVoxelStorage *vs)
{
    return vs->levels[0].height;
}
SCEuint SCE_VStore_GetDepth (const SCE_SVoxelStorage *vs)
{
    return vs->levels[0].depth;
}

SCEuint SCE_VStore_GetWidthLevel (const SCE_SVoxelStorage *vs, SCEuint level)
{
    return vs->levels[level].width;
}
SCEuint SCE_VStore_GetHeightLevel (const SCE_SVoxelStorage *vs, SCEuint level)
{
    return vs->levels[level].height;
}
SCEuint SCE_VStore_GetDepthLevel (const SCE_SVoxelStorage *vs, SCEuint level)
{
    return vs->levels[level].depth;
}
void SCE_VStore_GetDimensionsLevel (const SCE_SVoxelStorage *vs, SCEuint level,
                                    SCEuint *w, SCEuint *h, SCEuint *d)
{
    *w = vs->levels[level].width;
    *h = vs->levels[level].height;
    *d = vs->levels[level].depth;
}

SCEuint SCE_VStore_GetNumPoints (const SCE_SVoxelStorage *vs, SCEuint level)
{
    return vs->levels[level].width *
           vs->levels[level].height * vs->levels[level].depth;
}
size_t SCE_VStore_GetSize (const SCE_SVoxelStorage *vs, SCEuint level)
{
    return vs->data_size * SCE_VStore_GetNumPoints (vs, level);
}


static size_t getoffset (const SCE_SVoxelStorage *vs, SCEuint level,
                         int x, int y, int z)
{
    int w, h;
    w = vs->levels[level].width;
    h = vs->levels[level].height;
    return vs->data_size * (w * (h * z + y) + x);
}


static int SCE_VStore_BuildLevel (SCE_SVoxelStorage *vs, SCEuint level)
{
    SCEuint i;
    size_t size, offset, num;
    SCE_SVoxelStorageLevel *sl = NULL;

    sl = &vs->levels[level];

    size = SCE_VStore_GetSize (vs, level);
    if (!(sl->data = SCE_malloc (size))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    /* fill voxels with the given vaccum data */
    num = SCE_VStore_GetNumPoints (vs, level);
    size = vs->data_size;

    for (i = 0; i < num; i++)
        memcpy (&sl->data[i * size], vs->vacuum, size);

    return SCE_OK;
}

/* memory leak if this function is called multiple times */
/* also vacuum in void, dog. */
int SCE_VStore_Build (SCE_SVoxelStorage *vs, const void *vacuum)
{
    SCEuint i;

    if (!(vs->vacuum = SCE_malloc (vs->data_size)))
        goto fail;
    memcpy (vs->vacuum, vacuum, vs->data_size);

    for (i = 0; i < vs->n_lod; i++) {
        if (SCE_VStore_BuildLevel (vs, i) < 0)
            goto fail;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


static void SCE_VStore_PushZone (SCE_SVoxelStorage *vs,
                                 const SCE_SIntRect3 *r)
{
    vs->zones[vs->last] = *r;
    vs->last = SCE_Math_Ring (vs->last + 1, SCE_MAX_UPDATE_ZONES);
}
static int SCE_VStore_PopZone (SCE_SVoxelStorage *vs, SCE_SIntRect3 *r)
{
    if (vs->first == vs->last)
        return SCE_FALSE;
    else {
        *r = vs->zones[vs->first];
        vs->first = SCE_Math_Ring (vs->first + 1, SCE_MAX_UPDATE_ZONES);
        return SCE_TRUE;
    }
}

void SCE_VStore_SetPoint (SCE_SVoxelStorage *vs, SCEuint x, SCEuint y,
                          SCEuint z, const void *data)
{
    SCE_SIntRect3 r;
    size_t offset;

    if (x >= vs->levels[0].width ||
        y >= vs->levels[0].height ||
        z >= vs->levels[0].depth)
        return;

    offset = getoffset (vs, 0, x, y, z);
    memcpy (&vs->levels[0].data[offset], data, vs->data_size);
    SCE_Rectangle3_Set (&r, x, y, z, x+1, y+1, z+1);
    SCE_VStore_PushZone (vs, &r);
}
void SCE_VStore_GetPoint (const SCE_SVoxelStorage *vs, SCEuint level,
                          SCEuint x, SCEuint y, SCEuint z, void *data)
{
    size_t offset = getoffset (vs, level, x, y, z);
    if (x >= vs->levels[0].width ||
        y >= vs->levels[0].height ||
        z >= vs->levels[0].depth) {
        /* just copy a 'vacuum' voxel */
        memcpy (data, vs->vacuum, vs->data_size);
    } else {
        memcpy (data, &vs->levels[level].data[offset], vs->data_size);
    }
}


void SCE_VStore_SetRegion (SCE_SVoxelStorage *vs, const SCE_SIntRect3 *area_,
                           const void *data_)
{
    SCEuint y, z, y2, z2;
    int p1[3], p2[3], start[3], bleup[3];
    int w, h, d, w2;
    SCE_SIntRect3 rect, area;
    const unsigned char *data = data_;

    SCE_VStore_GetDimensionsLevel (vs, 0, &w, &h, &d);
    SCE_Rectangle3_Set (&rect, 0, 0, 0, w, h, d);

    w = SCE_Rectangle3_GetWidth (area_);
    h = SCE_Rectangle3_GetHeight (area_);

    area = *area_;
    if (!SCE_Rectangle3_IsInside (&rect, &area))
        SCE_Rectangle3_Intersection (&rect, area_, &area);

    w2 = SCE_Rectangle3_GetWidth (&area);
    SCE_Rectangle3_GetPointsv (&area, p1, p2);
    SCE_Rectangle3_GetPointsv (area_, start, bleup);
    start[0] = p1[0] - start[0];
    start[1] = p1[1] - start[1];
    start[2] = p1[2] - start[2];

    for (z = p1[2], z2 = start[2]; z < p2[2]; z++, z2++) {
        for (y = p1[1], y2 = start[1]; y < p2[1]; y++, y2++) {
            size_t offset = getoffset (vs, 0, p1[0], y, z);
            size_t offset2 = w * (h * z2 + y2) + start[0];
            memcpy (&vs->levels[0].data[offset], &data[offset2],
                    vs->data_size * w2);
        }
    }

    SCE_VStore_PushZone (vs, &area);
}

void SCE_VStore_GetRegion (const SCE_SVoxelStorage *vs, SCEuint level,
                           const SCE_SIntRect3 *area_, void *data_)
{
    SCEuint y, z, y2, z2;
    int p1[3], p2[3], start[3], bleup[3];
    int w, h, d, w2;
    SCE_SIntRect3 rect, area;
    SCE_SVoxelStorageLevel *sl = NULL;
    unsigned char *data = data_;

    sl = &vs->levels[level];
    SCE_VStore_GetDimensionsLevel (vs, level, &w, &h, &d);
    SCE_Rectangle3_Set (&rect, 0, 0, 0, w, h, d);

    w = SCE_Rectangle3_GetWidth (area_);
    h = SCE_Rectangle3_GetHeight (area_);

    area = *area_;
    if (!SCE_Rectangle3_IsInside (&rect, &area)) {
        /* fillup user buffer with vacuum */
        size_t i, num = SCE_Rectangle3_GetArea (area_);
        for (i = 0; i < num; i++)
            memcpy (&data[i * vs->data_size], vs->vacuum, vs->data_size);
        SCE_Rectangle3_Intersection (&rect, area_, &area);
    }

    w2 = SCE_Rectangle3_GetWidth (&area);
    SCE_Rectangle3_GetPointsv (&area, p1, p2);
    SCE_Rectangle3_GetPointsv (area_, start, bleup);
    start[0] = p1[0] - start[0];
    start[1] = p1[1] - start[1];
    start[2] = p1[2] - start[2];

    if (w2 > 0 && p2[0] > 0 && p2[1] > 0 && p2[2] > 0) {
        for (z = p1[2], z2 = start[2]; z < p2[2]; z++, z2++) {
            for (y = p1[1], y2 = start[1]; y < p2[1]; y++, y2++) {
                size_t offset = getoffset (vs, level, p1[0], y, z);
                size_t offset2 = w * (h * z2 + y2) + start[0];
                memcpy (&data[offset2], &sl->data[offset], vs->data_size * w2);
            }
        }
    }
}

void SCE_VStore_GetGridRegion (const SCE_SVoxelStorage *vs, SCEuint level,
                               const SCE_SIntRect3 *area_,
                               SCE_SGrid *grid, int ox, int oy, int oz)
{
    SCEuint x, y, z, x2, y2, z2;
    SCEuint w, h, d;
    int p1[3], p2[3], start[3], bleup[3];
    size_t offset1, offset2;
    SCE_SIntRect3 rect, area;
    SCE_SVoxelStorageLevel *sl = NULL;
    unsigned char *data = SCE_Grid_GetRaw (grid);

    sl = &vs->levels[level];
    SCE_VStore_GetDimensionsLevel (vs, level, &w, &h, &d);
    SCE_Rectangle3_Set (&rect, 0, 0, 0, w, h, d);

    area = *area_;
    if (!SCE_Rectangle3_IsInside (&rect, &area)) {
        /* fillup user buffer with vacuum */
        w = SCE_Rectangle3_GetWidth (area_);
        h = SCE_Rectangle3_GetHeight (area_);
        d = SCE_Rectangle3_GetDepth (area_);
        for (z = oz; z < d + oz; z++) {
            for (y = oy; y < h + oy; y++) {
                for (x = ox; x < w + ox; x++) {
                    offset1 = SCE_Grid_GetOffset (grid, x, y, z);
                    memcpy (&data[offset1], vs->vacuum, vs->data_size);
                }
            }
        }

        SCE_Rectangle3_Intersection (&rect, area_, &area);
    }

    SCE_Rectangle3_GetPointsv (&area, p1, p2);
    SCE_Rectangle3_GetPointsv (area_, start, bleup);
    ox += p1[0] - start[0];
    oy += p1[1] - start[1];
    oz += p1[2] - start[2];

    w = SCE_Grid_GetWidth (grid);
    h = SCE_Grid_GetHeight (grid);
    d = SCE_Grid_GetDepth (grid);

    for (z2 = p1[2], z = oz; z2 < p2[2] && z < d; z2++, z++) {
        for (y2 = p1[1], y = oy; y2 < p2[1] && y < h; y2++, y++) {
            for (x2 = p1[0], x = ox; x2 < p2[0] && x < w; x2++, x++) {
                offset1 = SCE_Grid_GetOffset (grid, x, y, z);
                offset2 = getoffset (vs, level, x2, y2, z2);
                memcpy (&data[offset1], &sl->data[offset2], vs->data_size);
            }
        }
    }
}


void SCE_VStore_ForceUpdate (SCE_SVoxelStorage *vs, const SCE_SIntRect3 *r)
{
    SCE_VStore_PushZone (vs, r);
}
int SCE_VStore_GetNextUpdatedZone (SCE_SVoxelStorage *vs, SCE_SIntRect3 *zone)
{
    return SCE_VStore_PopZone (vs, zone);
}

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
   updated: 18/03/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVStore.h"

void SCE_VStore_Init (SCE_SVoxelStorage *vs)
{
    int i;
    vs->data = NULL;
    vs->data_size = 0;
    vs->width = vs->height = vs->depth = 0;
    for (i = 0; i < SCE_MAX_UPDATE_ZONES; i++)
        SCE_Rectangle3_Init (&vs->zones[i]);
    vs->last = vs->first = 0;
}
void SCE_VStore_Clear (SCE_SVoxelStorage *vs)
{
    SCE_free (vs->data);
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
    vs->width = w; vs->height = h; vs->depth = d;
}

SCEuint SCE_VStore_GetWidth (const SCE_SVoxelStorage *vs)
{
    return vs->width;
}
SCEuint SCE_VStore_GetHeight (const SCE_SVoxelStorage *vs)
{
    return vs->height;
}
SCEuint SCE_VStore_GetDepth (const SCE_SVoxelStorage *vs)
{
    return vs->depth;
}

SCEuint SCE_VStore_GetNumPoints (const SCE_SVoxelStorage *vs)
{
    return vs->width * vs->height * vs->depth;
}
size_t SCE_VStore_GetSize (const SCE_SVoxelStorage *vs)
{
    return vs->data_size * SCE_VStore_GetNumPoints (vs);
}


int SCE_VStore_Build (SCE_SVoxelStorage *vs)
{
    size_t size;

    SCE_free (vs->data);
    size = SCE_VStore_GetSize (vs);
    if (!(vs->data = SCE_malloc (size))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    /* by default, the data are filled with crap :) */
    return SCE_OK;
}


static size_t getoffset (const SCE_SVoxelStorage *vs, SCEuint x, SCEuint y,
                         SCEuint z)
{
    return vs->data_size * (vs->width * (vs->height * z + y) + x);
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
    size_t offset = getoffset (vs, x, y, z);
    memcpy (&(((unsigned char*)vs->data)[offset]), data, vs->data_size);
    SCE_Rectangle3_Set (&r, x, y, z, x+1, y+1, z+1);
    SCE_VStore_PushZone (vs, &r);
}
void SCE_VStore_GetPoint (const SCE_SVoxelStorage *vs, SCEuint x, SCEuint y,
                          SCEuint z, void *data)
{
    size_t offset = getoffset (vs, x, y, z);
    memcpy (data, &(((unsigned char*)vs->data)[offset]), vs->data_size);
}


void SCE_VStore_SetRegion (SCE_SVoxelStorage *vs,
                           const SCE_SIntRect3 *area, const void *data)
{
    SCEuint y, z, y2, z2;
    int p1[3], p2[3];
    int w, h;

    SCE_Rectangle3_GetPointsv (area, p1, p2);
    w = SCE_Rectangle3_GetWidth (area);
    h = SCE_Rectangle3_GetHeight (area);

    for (z = p1[2], z2 = 0; z < p2[2]; z++, z2++) {
        for (y = p1[1], y2 = 0; y < p2[1]; y++, y2++) {
            size_t offset = getoffset (vs, p1[0], y, z);
            size_t offset2 = w * (h * z2 + y2);
            memcpy (&(((unsigned char*)vs->data)[offset]),
                    &(((unsigned char*)data)[offset2]),
                    vs->data_size * w);
        }
    }

    SCE_VStore_PushZone (vs, area);
}
void SCE_VStore_GetRegion (const SCE_SVoxelStorage *vs,
                           const SCE_SIntRect3 *area, void *data)
{
    SCEuint y, z, y2, z2;
    int p1[3], p2[3];
    int w, h;

    SCE_Rectangle3_GetPointsv (area, p1, p2);
    w = SCE_Rectangle3_GetWidth (area);
    h = SCE_Rectangle3_GetHeight (area);

    for (z = p1[2], z2 = 0; z < p2[2]; z++, z2++) {
        for (y = p1[1], y2 = 0; y < p2[1]; y++, y2++) {
            size_t offset = getoffset (vs, p1[0], y, z);
            size_t offset2 = w * (h * z2 + y2);
            memcpy (&(((unsigned char*)data)[offset2]),
                    &(((unsigned char*)vs->data)[offset]),
                    vs->data_size * w);
        }
    }
}
void SCE_VStore_GetGridRegion (const SCE_SVoxelStorage *vs,
                               const SCE_SIntRect3 *area,
                               SCE_SGrid *grid, int ox, int oy, int oz)
{
    SCEuint x, y, z, x2, y2, z2;
    SCEuint w, h, d;
    int p1[3], p2[3];
    /* let's hope vs->data_size doesnt exceed 256 */
    unsigned char buf[256] = {0};
    size_t offset1, offset2;
    void *data = SCE_Grid_GetRaw (grid);

    SCE_Rectangle3_GetPointsv (area, p1, p2);

    w = SCE_Grid_GetWidth (grid);
    h = SCE_Grid_GetHeight (grid);
    d = SCE_Grid_GetDepth (grid);

    for (z2 = p1[2], z = oz; z2 < p2[2] && z < d; z2++, z++) {
        for (y2 = p1[1], y = oy; y2 < p2[1] && y < h; y2++, y++) {
            for (x2 = p1[0], x = ox; x2 < p2[0] && x < w; x2++, x++) {
                offset1 = SCE_Grid_GetOffset (grid, x, y, z);
                offset2 = getoffset (vs, x2, y2, z2);
                memcpy (&(((unsigned char*)data)[offset1]),
                        &(((unsigned char*)vs->data)[offset2]),
                        vs->data_size);
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

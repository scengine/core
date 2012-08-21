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

/* created: 27/04/2012
   updated: 17/08/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVoxelGrid.h"

void SCE_VGrid_Init (SCE_SVoxelGrid *vg)
{
    vg->w = vg->h = vg->d = 0;
    vg->n_cmp = 1;
    vg->data = NULL;

    vg->full_checked = vg->empty_checked = SCE_FALSE;
    vg->full = vg->empty = SCE_FALSE;
}
void SCE_VGrid_Clear (SCE_SVoxelGrid *vg)
{
    SCE_free (vg->data);
}
SCE_SVoxelGrid* SCE_VGrid_Create (void)
{
    SCE_SVoxelGrid *vg = NULL;
    if (!(vg = SCE_malloc (sizeof *vg)))
        SCEE_LogSrc ();
    else
        SCE_VGrid_Init (vg);
    return vg;
}
void SCE_VGrid_Delete (SCE_SVoxelGrid *vg)
{
    if (vg) {
        SCE_VGrid_Clear (vg);
        SCE_free (vg);
    }
}

void SCE_VGrid_SetDimensions (SCE_SVoxelGrid *vg, SCEulong w, SCEulong h,
                              SCEulong d)
{
    vg->w = w;
    vg->h = h;
    vg->d = d;
}
SCEulong SCE_VGrid_GetWidth (SCE_SVoxelGrid *vg)
{
    return vg->w;
}
SCEulong SCE_VGrid_GetHeight (SCE_SVoxelGrid *vg)
{
    return vg->h;
}
SCEulong SCE_VGrid_GetDepth (SCE_SVoxelGrid *vg)
{
    return vg->d;
}
SCEulong SCE_VGrid_GetNumVoxels (const SCE_SVoxelGrid *vg)
{
    return vg->w * vg->h * vg->d;
}
size_t SCE_VGrid_GetSize (const SCE_SVoxelGrid *vg)
{
    return SCE_VGrid_GetNumVoxels (vg) * vg->n_cmp;
}
void* SCE_VGrid_GetRaw (SCE_SVoxelGrid *vg)
{
    return vg->data;
}
void SCE_VGrid_SetRaw (SCE_SVoxelGrid *vg, void *data)
{
    SCE_free (vg->data);
    vg->data = data;
}

void SCE_VGrid_SetNumComponents (SCE_SVoxelGrid *vg, size_t n)
{
    vg->n_cmp = n;
}

int SCE_VGrid_Build (SCE_SVoxelGrid *vg)
{
    size_t size;

    size = vg->n_cmp * SCE_VGrid_GetNumVoxels (vg);
    if (!(vg->data = SCE_malloc (size))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    /* NOTE: fill the voxels with an empty pattern... ? */
    return SCE_OK;
}

#define GOFFSET(g, x, y, z) ((g)->w * ((g)->h * (z) + (y)) + (x))
#define VOFFSET(g, x, y, z) ((g)->n_cmp * GOFFSET (g, x, y, z))

void SCE_VGrid_Fill (SCE_SVoxelGrid *vg, const SCE_SLongRect3 *region,
                     const SCEubyte *pattern)
{
    long p1[3], p2[3];
    long x, y, z;
    SCE_SLongRect3 default_region;

    SCE_Rectangle3_Setl (&default_region, 0, 0, 0, vg->w, vg->h, vg->d);
    if (!region) region = &default_region;
    SCE_Rectangle3_GetPointslv (region, p1, p2);

    for (z = p1[2]; z < p2[2]; z++) {
        for (y = p1[1]; y < p2[1]; y++) {
            for (x = p1[0]; x < p2[0]; x++)
                memcpy (&vg->data[VOFFSET (vg, x, y, z)], pattern, vg->n_cmp);
        }
    }
}

void SCE_VGrid_Copy (const SCE_SLongRect3 *dst_region, SCE_SVoxelGrid *dst,
                     const SCE_SLongRect3 *src_region,
                     const SCE_SVoxelGrid *src)
{
    long dst_p1[3], dst_p2[3];
    long src_p1[3], src_p2[3];
    long x1, y1, z1, x2, y2, z2;
    SCE_SLongRect3 default_dst_region, default_src_region;
    size_t n_cmp;

    SCE_Rectangle3_Setl (&default_dst_region, 0, 0, 0, dst->w, dst->h, dst->d);
    SCE_Rectangle3_Setl (&default_src_region, 0, 0, 0, src->w, src->h, src->d);
    if (!dst_region) dst_region = &default_dst_region;
    if (!src_region) src_region = &default_src_region;

    n_cmp = MIN (src->n_cmp, dst->n_cmp);

    SCE_Rectangle3_GetPointslv (dst_region, dst_p1, dst_p2);
    SCE_Rectangle3_GetPointslv (src_region, src_p1, src_p2);

    for (z1 = dst_p1[2], z2 = src_p1[2]; z1 < dst_p2[2]; z1++, z2++) {
        for (y1 = dst_p1[1], y2 = src_p1[1]; y1 < dst_p2[1]; y1++, y2++) {
            for (x1 = dst_p1[0], x2 = src_p1[0]; x1 < dst_p2[0]; x1++, x2++) {
                memcpy (&dst->data[VOFFSET (dst, x1, y1, z1)],
                        &src->data[VOFFSET (src, x2, y2, z2)], n_cmp);
            }
        }
    }
}

SCEubyte* SCE_VGrid_Offset (SCE_SVoxelGrid *vg, SCEulong x, SCEulong y,
                            SCEulong z)
{
    return &vg->data[VOFFSET (vg, x, y, z)];
}


int SCE_VGrid_IsEmpty (SCE_SVoxelGrid *vg, const SCE_SLongRect3 *region)
{
    if (vg->empty_checked && vg->empty)
        return SCE_TRUE;
    else {
        long p1[3], p2[3];
        long x, y, z;

        if (!vg->empty_checked) {
            vg->empty_checked = SCE_TRUE;
            vg->empty = SCE_VGrid_IsEmpty (vg, NULL);
        }

        if (region)
            SCE_Rectangle3_GetPointslv (region, p1, p2);
        else {
            p1[0] =        p1[1] =        p1[2] = 0;
            p2[0] = vg->w; p2[1] = vg->h; p2[2] = vg->d;
        }

        for (z = p1[2]; z < p2[2]; z++) {
            for (y = p1[1]; y < p2[1]; y++) {
                for (x = p1[0]; x < p2[0]; x++) {
                    /* assuming the first byte is the density */
                    SCEubyte density = vg->data[VOFFSET (vg, x, y, z)];
                    if (density > 127)
                        return SCE_FALSE;
                }
            }
        }
        return SCE_TRUE;
    }
}

int SCE_VGrid_IsFull (SCE_SVoxelGrid *vg, const SCE_SLongRect3 *region)
{
    if (vg->full_checked && vg->full)
        return SCE_TRUE;
    else {
        long p1[3], p2[3];
        long x, y, z;

        if (!vg->full_checked) {
            vg->full_checked = SCE_TRUE;
            vg->full = SCE_VGrid_IsFull (vg, NULL);
        }

        if (region)
            SCE_Rectangle3_GetPointslv (region, p1, p2);
        else {
            p1[0] =        p1[1] =        p1[2] = 0;
            p2[0] = vg->w; p2[1] = vg->h; p2[2] = vg->d;
        }

        for (z = p1[2]; z < p2[2]; z++) {
            for (y = p1[1]; y < p2[1]; y++) {
                for (x = p1[0]; x < p2[0]; x++) {
                    /* assuming the first byte is the density */
                    SCEubyte density = vg->data[VOFFSET (vg, x, y, z)];
                    if (density < 128)
                        return SCE_FALSE;
                }
            }
        }
        return SCE_TRUE;
    }
}

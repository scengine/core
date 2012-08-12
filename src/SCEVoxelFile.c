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
   updated: 11/08/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVoxelGrid.h"
#include "SCE/core/SCEVoxelFile.h"


static void SCE_VFile_InitStats (SCE_SVoxelFileStats *s)
{
    s->in_volume = 0;
}

void SCE_VFile_Init (SCE_SVoxelFile *vf)
{
    SCE_File_Init (&vf->fp);
    vf->is_open = SCE_FALSE;
    vf->w = vf->h = vf->d = 0;
    vf->n_cmp = 0;
    SCE_VFile_InitStats (&vf->stats);
}
void SCE_VFile_Clear (SCE_SVoxelFile *vf)
{
    if (vf->is_open)
        SCE_File_Close (&vf->fp);
}


void SCE_VFile_SetDimensions (SCE_SVoxelFile *vf, SCEulong w, SCEulong h,
                              SCEulong d)
{
    vf->w = w;
    vf->h = h;
    vf->d = d;
}

void SCE_VFile_SetNumComponents (SCE_SVoxelFile *vf, size_t n)
{
    vf->n_cmp = n;
}

int SCE_VFile_Open (SCE_SVoxelFile *vf, SCE_SFileSystem *fs, const char *fname)
{
    if (SCE_File_Open (&vf->fp, fs, fname, SCE_FILE_READ |
                       SCE_FILE_WRITE | SCE_FILE_CREATE) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    vf->is_open = SCE_TRUE;
    return SCE_OK;
}

void SCE_VFile_Close (SCE_SVoxelFile *vf)
{
    if (vf->is_open) {
        SCE_File_Close (&vf->fp);
        vf->is_open = SCE_FALSE;
    }
}

int SCE_VFile_IsOpen (SCE_SVoxelFile *vf)
{
    return vf->is_open;
}


void SCE_VFile_Fill (SCE_SVoxelFile *vf, const SCEubyte *pattern)
{
    size_t x, y, z;

    for (z = 0; z < vf->d; z++) {
        for (y = 0; y < vf->h; y++) {
            for (x = 0; x < vf->w; x++)
                SCE_File_Write (pattern, vf->n_cmp, 1, &vf->fp);
        }
    }
    SCE_File_Rewind (&vf->fp);
}

void SCE_VFile_GetRegion (SCE_SVoxelFile *src, const SCE_SLongRect3 *src_region,
                          SCE_SVoxelGrid *dst, const SCE_SLongRect3 *dst_region)
{
    long dst_p1[3], dst_p2[3];
    long src_p1[3], src_p2[3];
    long y1, z1, y2, z2;
    SCEulong w;
    SCE_SLongRect3 default_dst_region, default_src_region;
    size_t n_cmp;

    SCE_Rectangle3_Setl (&default_dst_region, 0, 0, 0, dst->w, dst->h, dst->d);
    SCE_Rectangle3_Setl (&default_src_region, 0, 0, 0, src->w, src->h, src->d);
    if (!dst_region) dst_region = &default_dst_region;
    if (!src_region) src_region = &default_src_region;

    n_cmp = MIN (src->n_cmp, dst->n_cmp);
    w = MIN (SCE_Rectangle3_GetWidthl (dst_region),
             SCE_Rectangle3_GetWidthl (src_region));

    SCE_Rectangle3_GetPointslv (dst_region, dst_p1, dst_p2);
    SCE_Rectangle3_GetPointslv (src_region, src_p1, src_p2);

    for (z1 = dst_p1[2], z2 = src_p1[2]; z1 < dst_p2[2]; z1++, z2++) {
        for (y1 = dst_p1[1], y2 = src_p1[1]; y1 < dst_p2[1]; y1++, y2++) {
            SCE_File_Seek (&src->fp, src->w * (src->h * z2 + y2) + src_p1[0],
                           SEEK_SET);
            SCE_File_Read (SCE_VGrid_Offset (dst, dst_p1[0], y1, z1),
                           w * n_cmp, 1, &src->fp);
        }
    }
}
void SCE_VFile_SetRegion (SCE_SVoxelFile *dst, const SCE_SLongRect3 *dst_region,
                          const SCE_SVoxelGrid *src,
                          const SCE_SLongRect3 *src_region)
{
    long dst_p1[3], dst_p2[3];
    long src_p1[3], src_p2[3];
    long x1, y1, z1, x2, y2, z2;
    SCEulong w;
    SCE_SLongRect3 default_dst_region, default_src_region;

    SCE_Rectangle3_Setl (&default_dst_region, 0, 0, 0, dst->w, dst->h, dst->d);
    SCE_Rectangle3_Setl (&default_src_region, 0, 0, 0, src->w, src->h, src->d);
    if (!dst_region) dst_region = &default_dst_region;
    if (!src_region) src_region = &default_src_region;

    w = MIN (SCE_Rectangle3_GetWidthl (dst_region),
             SCE_Rectangle3_GetWidthl (src_region));

    SCE_Rectangle3_GetPointslv (dst_region, dst_p1, dst_p2);
    SCE_Rectangle3_GetPointslv (src_region, src_p1, src_p2);

    for (z1 = src_p1[2], z2 = dst_p1[2]; z1 < src_p2[2]; z1++, z2++) {
        for (y1 = src_p1[1], y2 = dst_p1[1]; y1 < src_p2[1]; y1++, y2++) {
            long o = (dst->w * (dst->h * z2 + y2) + dst_p1[0]);
            SCE_File_Seek (&dst->fp, dst->n_cmp * o, SEEK_SET);

            for (x1 = src_p1[0], x2 = dst_p1[0]; x1 < src_p2[0]; x1++, x2++) {
                /* NOTE: we hope that 42 > n_cmp */
                SCEubyte buffer[42] = {0};
                SCEubyte *grid = SCE_VGrid_Offset (src, x1, y1, z1);

                SCE_File_Read (buffer, dst->n_cmp, 1, &dst->fp);
                if (buffer[0] <= 127 && grid[0] > 127)
                    dst->stats.in_volume++;
                else if (buffer[0] > 127 && grid[0] <= 127)
                    dst->stats.in_volume--;
                /* TODO: check material changes */
                SCE_File_Seek (&dst->fp, -dst->n_cmp, SEEK_CUR);
                SCE_File_Write (grid, dst->n_cmp, 1, &dst->fp);
            }
        }
    }
}


void SCE_VFile_GetStatsv (SCE_SVoxelFile *vf, SCE_SVoxelFileStats *s)
{
    *s = vf->stats;
    SCE_VFile_InitStats (&vf->stats);
}

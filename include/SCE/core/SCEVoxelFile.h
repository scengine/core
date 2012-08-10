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
   updated: 10/08/2012 */

#ifndef SCEVOXELFILE_H
#define SCEVOXELFILE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEVoxelGrid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_svoxelfilestats SCE_SVoxelFileStats;
struct sce_svoxelfilestats {
    long in_volume;
};

typedef struct sce_svoxelfile SCE_SVoxelFile;
struct sce_svoxelfile {
    SCE_SFile fp;
    int is_open;
    SCEulong w, h, d;
    size_t n_cmp;
    SCE_SVoxelFileStats stats;
};

void SCE_VFile_Init (SCE_SVoxelFile*);
void SCE_VFile_Clear (SCE_SVoxelFile*);

void SCE_VFile_SetDimensions (SCE_SVoxelFile*, SCEulong, SCEulong, SCEulong);
void SCE_VFile_SetNumComponents (SCE_SVoxelFile*, size_t);

int SCE_VFile_Open (SCE_SVoxelFile*, SCE_SFileSystem*, const char*);
void SCE_VFile_Close (SCE_SVoxelFile*);
int SCE_VFile_IsOpen (SCE_SVoxelFile*);

void SCE_VFile_Fill (SCE_SVoxelFile*, const SCEubyte*);

void SCE_VFile_GetRegion (SCE_SVoxelFile*, const SCE_SLongRect3*,
                          SCE_SVoxelGrid*, const SCE_SLongRect3*);
void SCE_VFile_SetRegion (SCE_SVoxelFile*, const SCE_SLongRect3*,
                          const SCE_SVoxelGrid*, const SCE_SLongRect3*);

void SCE_VFile_GetStatsv (SCE_SVoxelFile*, SCE_SVoxelFileStats*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

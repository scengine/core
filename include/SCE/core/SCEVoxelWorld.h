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

/* created: 07/05/2012
   updated: 07/05/2012 */

#ifndef SCEVOXELWORLD_H
#define SCEVOXELWORLD_H

#include "SCE/core/SCEVoxelOctree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_svoxelworldtree SCE_SVoxelWorldTree;
struct sce_svoxelworldtree {
    SCE_SVoxelOctree vo;
    SCE_SListIterator it;
};

#define SCE_MAX_VWORLD_UPDATE_ZONES 128

typedef struct sce_svoxelworld SCE_SVoxelWorld;
struct sce_svoxelworld {
    SCE_SList trees;            /* list of VoxelWorldTree */

    SCEulong w, h, d;
    SCEuint n_lod;
    char prefix[128];           /* root directory */

    SCE_SLongRect3 zones[SCE_MAX_VWORLD_UPDATE_ZONES];
    int zones_level[SCE_MAX_VWORLD_UPDATE_ZONES];
    int last, first;

    /* some memory pre-allocated for LOD computation */
    SCEubyte *buffer1, *buffer2;
    size_t size1, size2;
};

void SCE_VWorld_Init (SCE_SVoxelWorld*);
void SCE_VWorld_Clear (SCE_SVoxelWorld*);
SCE_SVoxelWorld* SCE_VWorld_Create (void);
void SCE_VWorld_Delete (SCE_SVoxelWorld*);

void SCE_VWorld_SetDimensions (SCE_SVoxelWorld*, SCEulong, SCEulong, SCEulong);
SCEulong SCE_VWorld_GetWidth (const SCE_SVoxelWorld*);
SCEulong SCE_VWorld_GetHeight (const SCE_SVoxelWorld*);
SCEulong SCE_VWorld_GetDepth (const SCE_SVoxelWorld*);
SCEulong SCE_VWorld_GetTotalWidth (const SCE_SVoxelWorld*);
SCEulong SCE_VWorld_GetTotalHeight (const SCE_SVoxelWorld*);
SCEulong SCE_VWorld_GetTotalDepth (const SCE_SVoxelWorld*);

void SCE_VWorld_SetNumLevels (SCE_SVoxelWorld*, SCEuint);
SCEuint SCE_VWorld_GetNumLevels (SCE_SVoxelWorld*);
void SCE_VWorld_SetPrefix (SCE_SVoxelWorld*, const char*);

SCE_SVoxelWorldTree* SCE_VWorld_AddNewTree (SCE_SVoxelWorld*, long, long, long);

int SCE_VWorld_Build (SCE_SVoxelWorld*);

int SCE_VWorld_Load (SCE_SVoxelWorld*, const char*);
int SCE_VWorld_Save (const SCE_SVoxelWorld*, const char*);

int SCE_VWorld_GetRegion (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                          SCEubyte*);
int SCE_VWorld_SetRegion (SCE_SVoxelWorld*, const SCE_SLongRect3*,
                          const SCEubyte*);

int SCE_VWorld_GetNextUpdatedRegion (SCE_SVoxelWorld*, SCE_SLongRect3*);

int SCE_VWorld_GenerateLOD (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                            SCE_SLongRect3*);
int SCE_VWorld_GenerateAllLOD (SCE_SVoxelWorld*, SCEuint,
                               const SCE_SLongRect3*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

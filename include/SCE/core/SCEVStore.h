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
   updated: 25/03/2012 */

#ifndef SCEVSTORE_H
#define SCEVSTORE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGrid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_svoxelstoragezone SCE_SVoxelStorageZone;
struct sce_svoxelstoragezone {
    SCE_SIntRect3 zone;
    SCE_SListIterator it;
};

typedef struct sce_svoxelstoragelevel SCE_SVoxelStorageLevel;
struct sce_svoxelstoragelevel {
    unsigned char *data;
    SCEuint width, height, depth;
};

#define SCE_MAX_VOXEL_STORAGE_LOD 16
#define SCE_MAX_UPDATE_ZONES 16

typedef struct sce_svoxelstorage SCE_SVoxelStorage;
struct sce_svoxelstorage {
    SCE_SVoxelStorageLevel levels[SCE_MAX_VOXEL_STORAGE_LOD];
    SCEuint n_lod;
    size_t data_size;
    SCEuint width, height, depth;
    unsigned char *vacuum;      /**< Void voxel */

    SCE_SIntRect3 zones[SCE_MAX_UPDATE_ZONES];
    int zones_level[SCE_MAX_UPDATE_ZONES];
    int last, first;
};

void SCE_VStore_Init (SCE_SVoxelStorage*);
void SCE_VStore_Clear (SCE_SVoxelStorage*);
SCE_SVoxelStorage* SCE_VStore_Create (void);
void SCE_VStore_Delete (SCE_SVoxelStorage*);

void SCE_VStore_SetDataSize (SCE_SVoxelStorage*, size_t);
void SCE_VStore_SetDimensions (SCE_SVoxelStorage*, SCEuint, SCEuint, SCEuint);
void SCE_VStore_SetNumLevels (SCE_SVoxelStorage*, SCEuint);

SCEuint SCE_VStore_GetWidth (const SCE_SVoxelStorage*);
SCEuint SCE_VStore_GetHeight (const SCE_SVoxelStorage*);
SCEuint SCE_VStore_GetDepth (const SCE_SVoxelStorage*);
SCEuint SCE_VStore_GetWidthLevel (const SCE_SVoxelStorage*, SCEuint);
SCEuint SCE_VStore_GetHeightLevel (const SCE_SVoxelStorage*, SCEuint);
SCEuint SCE_VStore_GetDepthLevel (const SCE_SVoxelStorage*, SCEuint);
void SCE_VStore_GetDimensionsLevel (const SCE_SVoxelStorage*, SCEuint,
                                    SCEuint*, SCEuint*, SCEuint*);
SCEuint SCE_VStore_GetNumPoints (const SCE_SVoxelStorage*, SCEuint);
size_t SCE_VStore_GetSize (const SCE_SVoxelStorage*, SCEuint);

int SCE_VStore_Build (SCE_SVoxelStorage*, const void*);

void SCE_VStore_SetPoint (SCE_SVoxelStorage*, SCEuint, SCEuint, SCEuint,
                          const void*);
void SCE_VStore_GetPoint (const SCE_SVoxelStorage*, SCEuint, SCEuint,
                          SCEuint, SCEuint, void*);

void SCE_VStore_SetRegion (SCE_SVoxelStorage*, const SCE_SIntRect3*,
                           const void*);
void SCE_VStore_GetRegion (const SCE_SVoxelStorage*, SCEuint,
                           const SCE_SIntRect3*, void*);
void SCE_VStore_GetGridRegion (const SCE_SVoxelStorage*, SCEuint,
                               const SCE_SIntRect3*, SCE_SGrid*, int, int, int);

void SCE_VStore_ForceUpdate (SCE_SVoxelStorage*, const SCE_SIntRect3*, int);
int SCE_VStore_GetNextUpdatedZone (SCE_SVoxelStorage*, SCE_SIntRect3*);

void SCE_VStore_GenerateLOD (SCE_SVoxelStorage*, SCEuint, const SCE_SIntRect3*,
                             SCE_SIntRect3*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

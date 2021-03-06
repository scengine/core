/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2013  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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
   updated: 04/07/2013 */

#ifndef SCEVOXELWORLD_H
#define SCEVOXELWORLD_H

#include <pthread.h>
#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEVoxelOctree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*SCE_FVoxelWorldMkdirFunc)(const char*);

typedef struct sce_svoxelworldtree SCE_SVoxelWorldTree;
struct sce_svoxelworldtree {
    pthread_rwlock_t rwlock;
    SCE_SVoxelOctree vo;
    void *udata;
    SCE_SListIterator it, it2;
};

#define SCE_MAX_VWORLD_UPDATE_ZONES 128

typedef struct sce_svoxelworldbuffer SCE_SVoxelWorldBuffer;
struct sce_svoxelworldbuffer {
    pthread_mutex_t mutex;
    SCEubyte *buffer1, *buffer2;
};

typedef struct sce_svoxelworld SCE_SVoxelWorld;
struct sce_svoxelworld {
    SCE_SList trees;            /* list of VoxelWorldTree */
    SCE_SArray2D trees_grid;
    int trees_initialized;

    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock, rwlock2;

    SCEulong w, h, d;
    SCEuint n_lod;
    SCE_EVoxelOctreeUsage usage;
    int create_trees;
    char prefix[128];           /* root directory */
    SCE_FVoxelWorldMkdirFunc fmkdir;
    SCE_SFileSystem *fs;
    SCE_SFileCache *fcache;
    SCEulong max_cached_nodes;

    SCE_SLongRect3 zones[SCE_MAX_VWORLD_UPDATE_ZONES];
    int zones_level[SCE_MAX_VWORLD_UPDATE_ZONES];
    int last, first;
    int record_updates;

    /* some memory pre-allocated for LOD computation */
    SCE_SVoxelWorldBuffer *buffers;
    size_t n_buffers;
    size_t size1, size2;
};

void SCE_VWorld_InitTree (SCE_SVoxelWorldTree*);
void SCE_VWorld_ClearTree (SCE_SVoxelWorldTree*);
SCE_SVoxelWorldTree* SCE_VWorld_CreateTree (void);
void SCE_VWorld_DeleteTree (SCE_SVoxelWorldTree*);

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
void SCE_VWorld_SetUsage (SCE_SVoxelWorld*, SCE_EVoxelOctreeUsage);
void SCE_VWorld_SetCreateTrees (SCE_SVoxelWorld*, int);
void SCE_VWorld_SetMkdirFunc (SCE_SVoxelWorld*, SCE_FVoxelWorldMkdirFunc);

void SCE_VWorld_SetFileSystem (SCE_SVoxelWorld*, SCE_SFileSystem*);
void SCE_VWorld_SetFileCache (SCE_SVoxelWorld*, SCE_SFileCache*);
void SCE_VWorld_SetMaxCachedNodes (SCE_SVoxelWorld*, SCEulong);
void SCE_VWorld_SetNumBuffers (SCE_SVoxelWorld*, size_t);

SCE_SVoxelWorldTree* SCE_VWorld_NewTree (SCE_SVoxelWorld*, long, long, long);
int SCE_VWorld_AddTree (SCE_SVoxelWorld*, SCE_SVoxelWorldTree*);
SCE_SVoxelWorldTree* SCE_VWorld_RemoveTree (SCE_SVoxelWorld*, long, long, long);
SCE_SVoxelWorldTree* SCE_VWorld_AddNewTree (SCE_SVoxelWorld*, long, long, long);
SCE_SVoxelWorldTree* SCE_VWorld_GetTree (SCE_SVoxelWorld*, long, long, long);
void SCE_VWorld_GetTreeOriginv (const SCE_SVoxelWorldTree*, long*, long*,long*);

SCE_SVoxelOctree* SCE_VWorld_GetOctree (SCE_SVoxelWorldTree*);
void SCE_VWorld_SetData (SCE_SVoxelWorldTree*, void*);
void* SCE_VWorld_GetData (SCE_SVoxelWorldTree*);

int SCE_VWorld_Build (SCE_SVoxelWorld*);

int SCE_VWorld_Load (SCE_SVoxelWorld*, const char*);
int SCE_VWorld_Save (const SCE_SVoxelWorld*, const char*);

int SCE_VWorld_LoadTree (SCE_SVoxelWorld*, long, long, long);
int SCE_VWorld_LoadAllTrees (SCE_SVoxelWorld*);
int SCE_VWorld_SaveTreev (SCE_SVoxelWorld*, SCE_SVoxelWorldTree*);
int SCE_VWorld_SaveTree (SCE_SVoxelWorld*, long, long, long);
int SCE_VWorld_SaveAllTrees (SCE_SVoxelWorld*);

void SCE_VWorld_TreeRegion (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                            long[3], long[3]);

/* int SCE_VWorld_UnloadTree (SCE_SVoxelWorld*, long, long, long); */

int SCE_VWorld_GetRegion (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                          SCEubyte*);
int SCE_VWorld_SetRegion (SCE_SVoxelWorld*, const SCE_SLongRect3*,
                          const SCEubyte*);
int SCE_VWorld_FillRegion (SCE_SVoxelWorld*, const SCE_SLongRect3*, SCEubyte);

void SCE_VWorld_AddUpdatedRegion (SCE_SVoxelWorld*, SCEuint,
                                  const SCE_SLongRect3*);
int SCE_VWorld_GetNextUpdatedRegion (SCE_SVoxelWorld*, SCE_SLongRect3*);
void SCE_VWorld_EnableUpdateRecording (SCE_SVoxelWorld*);
void SCE_VWorld_DisableUpdateRecording (SCE_SVoxelWorld*);

int SCE_VWorld_GenerateLOD (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                            SCE_SLongRect3*);
int SCE_VWorld_GenerateAllLOD (SCE_SVoxelWorld*, SCEuint,
                               const SCE_SLongRect3*);

int SCE_VWorld_FetchTrees (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                           SCE_SList*);
SCE_SVoxelWorldTree* SCE_VWorld_FetchTree (SCE_SVoxelWorld*, SCEuint,
                                           long, long, long);

int SCE_VWorld_FetchNodes (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*,
                           SCE_SList*);
SCE_SVoxelOctreeNode* SCE_VWorld_FetchNode (SCE_SVoxelWorld*, SCEuint,
                                            long, long, long);
int SCE_VWorld_FetchAllNodes (SCE_SVoxelWorld*, SCEuint, SCE_SList*);
int SCE_VWorld_FetchAllTreeNodes (SCE_SVoxelWorld*, long, long, long,
                                  SCEuint, SCE_SList*);

SCE_EVoxelOctreeStatus
SCE_VWorld_GetRegionStatus (SCE_SVoxelWorld*, SCEuint, const SCE_SLongRect3*);

int SCE_VWorld_UpdateCache (SCE_SVoxelWorld*);
int SCE_VWorld_SyncCache (SCE_SVoxelWorld*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

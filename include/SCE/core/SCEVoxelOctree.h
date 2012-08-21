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

/* created: 24/04/2012
   updated: 21/08/2012 */

#ifndef SCEVOXELOCTREE_H
#define SCEVOXELOCTREE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEVoxelGrid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_VOCTREE_VOXEL_ELEMENTS 1

#define SCE_VOCTREE_NODE_FNAME_LENGTH 64

typedef enum {
    SCE_VOCTREE_NODE_EMPTY = 0,
    SCE_VOCTREE_NODE_FULL,
    SCE_VOCTREE_NODE_LEAF,
    SCE_VOCTREE_NODE_NODE
} SCE_EVoxelOctreeStatus;

typedef struct sce_svoxeloctreenode SCE_SVoxelOctreeNode;
struct sce_svoxeloctreenode {
    SCE_EVoxelOctreeStatus status;
    SCE_SVoxelOctreeNode *children[8];
    SCEuint level;
    char fname[SCE_VOCTREE_NODE_FNAME_LENGTH];
    SCE_SVoxelGrid grid;
    SCE_SFile file;
    int is_open;           /* is \c file open? */
    int is_sync;           /* are \c grid and \c file synchronized? */
    int cached;            /* is \c grid on memory or does it need reloading
                              from \c file? */
    long in_volume;        /* number of voxels in the volume */
    int material;          /* material ID if full */
    SCE_SListIterator it;
};


typedef struct sce_svoxeloctree SCE_SVoxelOctree;
struct sce_svoxeloctree {
    SCE_SVoxelOctreeNode root;
    size_t max_depth;

    long x, y, z;
    SCEulong w, h, d;

    char prefix[128];

    SCE_SFileSystem *fs;
    SCE_SFileCache *fcache;

    SCEulong n_cached;
    SCEulong max_cached;
    SCE_SList cached;
};


void SCE_VOctree_Init (SCE_SVoxelOctree*);
void SCE_VOctree_Clear (SCE_SVoxelOctree*);
SCE_SVoxelOctree* SCE_VOctree_Create (void);
void SCE_VOctree_Delete (SCE_SVoxelOctree*);
void SCE_VOctree_DeleteChildren (SCE_SVoxelOctreeNode*);

void SCE_VOctree_SetOrigin (SCE_SVoxelOctree*, long, long, long);
void SCE_VOctree_SetDimensions (SCE_SVoxelOctree*, SCEulong, SCEulong,SCEulong);
void SCE_VOctree_SetMaxDepth (SCE_SVoxelOctree*, size_t);

void SCE_VOctree_SetPrefix (SCE_SVoxelOctree*, const char*);

void SCE_VOctree_SetFileSystem (SCE_SVoxelOctree*, SCE_SFileSystem*);
void SCE_VOctree_SetFileCache (SCE_SVoxelOctree*, SCE_SFileCache*);
void SCE_VOctree_SetMaxCachedNodes (SCE_SVoxelOctree*, SCEulong);

void SCE_VOctree_GetOriginv (const SCE_SVoxelOctree*, long*, long*, long*);
SCEulong SCE_VOctree_GetWidth (const SCE_SVoxelOctree*);
SCEulong SCE_VOctree_GetHeight (const SCE_SVoxelOctree*);
SCEulong SCE_VOctree_GetDepth (const SCE_SVoxelOctree*);
SCEulong SCE_VOctree_GetTotalWidth (const SCE_SVoxelOctree*);
SCEulong SCE_VOctree_GetTotalHeight (const SCE_SVoxelOctree*);
SCEulong SCE_VOctree_GetTotalDepth (const SCE_SVoxelOctree*);

int SCE_VOctree_Load (SCE_SVoxelOctree*, const char*);
int SCE_VOctree_Save (SCE_SVoxelOctree*, const char*);

int SCE_VOctree_GetRegion (SCE_SVoxelOctree*, SCEuint, const SCE_SLongRect3*,
                           SCEubyte*);
int SCE_VOctree_SetRegion (SCE_SVoxelOctree*, SCEuint, const SCE_SLongRect3*,
                           const SCEubyte*);

int SCE_VOctree_GetNode (SCE_SVoxelOctree*, SCEuint, long, long, long, char*);

void SCE_VOctree_UpdateCache (SCE_SVoxelOctree*);
int SCE_VOctree_SyncCache (SCE_SVoxelOctree*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

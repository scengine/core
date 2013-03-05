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

#include <time.h>
#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVoxelOctree.h"

static void SCE_VOctree_InitNode (SCE_SVoxelOctreeNode *node)
{
    size_t i;
    node->status = SCE_VOCTREE_NODE_EMPTY;
    for (i = 0; i < 8; i++)
        node->children[i] = NULL;
    node->level = 0;
    node->x = node->y = node->z = 0;
    memset (node->fname, 0, SCE_VOCTREE_NODE_FNAME_LENGTH);
    SCE_VGrid_Init (&node->grid);
    SCE_File_Init (&node->file);
    node->is_open = SCE_FALSE;
    node->is_sync = SCE_FALSE;
    node->cached = SCE_FALSE;
    node->in_volume = 0;
    node->material = 0;
    node->udata = NULL;
    SCE_List_InitIt (&node->it);
    SCE_List_SetData (&node->it, node);
}
static void SCE_VOctree_DeleteNode (SCE_SVoxelOctreeNode*);
static int SCE_VOctree_SyncNode (SCE_SVoxelOctree*, SCE_SVoxelOctreeNode*);
static void SCE_VOctree_ClearNode (SCE_SVoxelOctreeNode *node)
{
    size_t i;
    for (i = 0; i < 8; i++)
        SCE_VOctree_DeleteNode (node->children[i]);
    if (node->is_open)
        SCE_File_Close (&node->file);
    SCE_VGrid_Clear (&node->grid);
    SCE_List_Remove (&node->it);
}
static SCE_SVoxelOctreeNode* SCE_VOctree_CreateNode (void)
{
    SCE_SVoxelOctreeNode *node = NULL;
    if (!(node = SCE_malloc (sizeof *node)))
        SCEE_LogSrc ();
    else
        SCE_VOctree_InitNode (node);
    return node;
}
static void SCE_VOctree_DeleteNode (SCE_SVoxelOctreeNode *node)
{
    if (node) {
        SCE_VOctree_ClearNode (node);
        SCE_free (node);
    }
}

#if 0
static void
SCE_VOctree_SetFilename (SCE_SVoxelOctreeNode *node, const char *fname)
{
    strncpy (node->fname, fname, SCE_VOCTREE_NODE_FNAME_LENGTH - 1);
}
#endif

void SCE_VOctree_Init (SCE_SVoxelOctree *vo)
{
    SCE_VOctree_InitNode (&vo->root);
    vo->max_depth = 0;
    vo->x = vo->y = vo->z = 0;
    vo->w = vo->h = vo->d = 0;
    memset (vo->prefix, 0, sizeof vo->prefix);
    vo->fs = NULL;
    vo->fcache = NULL;

    vo->n_cached = 0;
    vo->max_cached = 16;        /* seems legit. */
    SCE_List_Init (&vo->cached);

    vo->udata = NULL;
}
void SCE_VOctree_Clear (SCE_SVoxelOctree *vo)
{
    SCE_VOctree_ClearNode (&vo->root);
    SCE_List_Clear (&vo->cached);
}
SCE_SVoxelOctree* SCE_VOctree_Create (void)
{
    SCE_SVoxelOctree *vo = NULL;
    if (!(vo = SCE_malloc (sizeof *vo)))
        SCEE_LogSrc ();
    else
        SCE_VOctree_Init (vo);
    return vo;
}
void SCE_VOctree_Delete (SCE_SVoxelOctree *vo)
{
    if (vo) {
        SCE_VOctree_Clear (vo);
        SCE_free (vo);
    }
}

void SCE_VOctree_DeleteChildren (SCE_SVoxelOctreeNode *node)
{
    size_t i;
    for (i = 0; i < 8; i++) {
        SCE_VOctree_DeleteNode (node->children[i]);
        node->children[i] = NULL;
    }
}

void SCE_VOctree_SetOrigin (SCE_SVoxelOctree *vo, long x, long y, long z)
{
    vo->x = x;
    vo->y = y;
    vo->z = z;
}
void SCE_VOctree_SetDimensions (SCE_SVoxelOctree *vo, SCEulong w,
                                SCEulong h, SCEulong d)
{
    vo->w = w;
    vo->h = h;
    vo->d = d;
}
void SCE_VOctree_SetMaxDepth (SCE_SVoxelOctree *vo, size_t depth)
{
    vo->max_depth = depth;
}


void SCE_VOctree_SetPrefix (SCE_SVoxelOctree *vo, const char *prefix)
{
    strncpy (vo->prefix, prefix, sizeof vo->prefix - 1);
}

void SCE_VOctree_SetFileSystem (SCE_SVoxelOctree *vo, SCE_SFileSystem *fs)
{
    vo->fs = fs;
}
void SCE_VOctree_SetFileCache (SCE_SVoxelOctree *vo, SCE_SFileCache *cache)
{
    vo->fcache = cache;
}
void SCE_VOctree_SetMaxCachedNodes (SCE_SVoxelOctree *vo, SCEulong max_cached)
{
    vo->max_cached = max_cached;
}


void SCE_VOctree_SetData (SCE_SVoxelOctree *vo, void *data)
{
    vo->udata = data;
}
void* SCE_VOctree_GetData (SCE_SVoxelOctree *vo)
{
    return vo->udata;
}
void SCE_VOctree_SetNodeData (SCE_SVoxelOctreeNode *node, void *data)
{
    node->udata = data;
}
void* SCE_VOctree_GetNodeData (SCE_SVoxelOctreeNode *node)
{
    return node->udata;
}
const char* SCE_VOctree_GetNodeFilename (const SCE_SVoxelOctreeNode *node)
{
    return node->fname;
}
static void SCE_VOctree_SetNodeOrigin (SCE_SVoxelOctreeNode *node,
                                       long x, long y, long z)
{
    node->x = x; node->y = y; node->z = z;
}
SCE_EVoxelOctreeStatus
SCE_VOctree_GetNodeStatus (const SCE_SVoxelOctreeNode *node)
{
    return node->status;
}
SCEuint SCE_VOctree_GetNodeLevel (const SCE_SVoxelOctreeNode *node)
{
    return node->level;
}
/**
 * \brief Get a node origin position in absolute coordinates
 * (in node's level space)
 * \param node a node
 * \param x,y,z coordinates of the origin of \p node
 */
void SCE_VOctree_GetNodeOriginv (const SCE_SVoxelOctreeNode *node,
                                long *x, long *y, long *z)
{
    *x = node->x; *y = node->y; *z = node->z;
}


void SCE_VOctree_GetOriginv (const SCE_SVoxelOctree *vo, long *x, long *y,
                             long *z)
{
    *x = vo->x; *y = vo->y; *z = vo->z;
}
void SCE_VOctree_GetDimensionsv (const SCE_SVoxelOctree *vo,
                                 long *w, long *h, long *d)
{
    *w = vo->w; *h = vo->h; *d = vo->d;
}
SCEulong SCE_VOctree_GetWidth (const SCE_SVoxelOctree *vo)
{
    return vo->w;
}
SCEulong SCE_VOctree_GetHeight (const SCE_SVoxelOctree *vo)
{
    return vo->h;
}
SCEulong SCE_VOctree_GetDepth (const SCE_SVoxelOctree *vo)
{
    return vo->d;
}
SCEulong SCE_VOctree_GetTotalWidth (const SCE_SVoxelOctree *vo)
{
    return vo->w * (1 << vo->max_depth);
}
SCEulong SCE_VOctree_GetTotalHeight (const SCE_SVoxelOctree *vo)
{
    return vo->h * (1 << vo->max_depth);
}
SCEulong SCE_VOctree_GetTotalDepth (const SCE_SVoxelOctree *vo)
{
    return vo->d * (1 << vo->max_depth);
}
static SCEulong SCE_VOctree_Getnnodes (const SCE_SVoxelOctreeNode *node)
{
    int i;
    SCEulong n = 0;

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
    case SCE_VOCTREE_NODE_FULL:
    case SCE_VOCTREE_NODE_LEAF:
        return 1;
    case SCE_VOCTREE_NODE_NODE:
        for (i = 0; i < 8; i++)
            n += SCE_VOctree_Getnnodes (node->children[i]);
        return 1 + n;
    }
    return 0;                   /* : d */
}
SCEulong SCE_VOctree_GetNumNodes (const SCE_SVoxelOctree *vo)
{
    return SCE_VOctree_Getnnodes (&vo->root);
}

static void SCE_VOctree_SetNodeGrid (SCE_SVoxelOctreeNode *node, SCEulong w,
                                     SCEulong h, SCEulong d, size_t n_cmp)
{
    SCE_VGrid_SetDimensions (&node->grid, w, h, d);
    SCE_VGrid_SetNumComponents (&node->grid, n_cmp);
}

static void
SCE_VOctree_MakeNodeFilename (const SCE_SVoxelOctree *vo,
                              const SCE_SLongRect3 *node_rect, SCEuint level,
                              char *fname)
{
    long p1[3], p2[3];
    /* TODO: file names use absolute coordinates, which sucks */
    SCE_Rectangle3_GetPointslv (node_rect, p1, p2);
    sprintf (fname, "%s/lod%u/%ld_%ld_%ld", vo->prefix, level,
             p1[0], p1[1], p1[2]);
}

static void SCE_VOctree_ConstructRect (const SCE_SLongRect3 *parent,
                                       SCEuint id, SCE_SLongRect3 *child)
{
    long w, h, d;

    w = SCE_Rectangle3_GetWidthl (parent) / 2;
    h = SCE_Rectangle3_GetHeightl (parent) / 2;
    d = SCE_Rectangle3_GetDepthl (parent) / 2;

    *child = *parent;
    SCE_Rectangle3_Resizel (child, w, h, d);
    SCE_Rectangle3_Movel (child, (1 & id) * w,
                          (1 & (id >> 1)) * h,
                          (1 & (id >> 2)) * d);
}


static int
SCE_VOctree_LoadNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                      SCE_SFile *fp, const SCE_SLongRect3 *node_rect, int level)
{
    size_t i;
    SCE_SLongRect3 rect;
    long x, y, z;

    if (level < 0) {
        SCEE_Log (29);
        SCEE_LogMsg ("invalid voctree format: corrupted max_depth");
        return SCE_ERROR;
    }

    rect = *node_rect;
    SCE_Rectangle3_Pow2l (&rect, -level);
    SCE_Rectangle3_GetOriginlv (&rect, &x, &y, &z);
    SCE_VOctree_MakeNodeFilename (vo, &rect, level, node->fname);
    SCE_VOctree_SetNodeGrid (node, vo->w, vo->h, vo->d, 1);
    SCE_VOctree_SetNodeOrigin (node, x, y, z);

    /* TODO: node->level = ??? */
    node->level = level;
    node->status = SCE_Decode_StreamLong (fp);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        break;
    case SCE_VOCTREE_NODE_FULL:
        node->in_volume = vo->w * vo->h * vo->d;
        node->material = SCE_Decode_StreamLong (fp);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        node->in_volume = SCE_Decode_StreamLong (fp);
        node->material = SCE_Decode_StreamLong (fp);
        break;
    case SCE_VOCTREE_NODE_NODE:
        node->in_volume = SCE_Decode_StreamLong (fp);
        node->material = SCE_Decode_StreamLong (fp);
        for (i = 0; i < 8; i++) {
            if (!(node->children[i] = SCE_VOctree_CreateNode ()))
                goto fail;
            SCE_VOctree_ConstructRect (node_rect, i, &rect);
            SCE_VOctree_LoadNode (vo, node->children[i], fp, &rect, level - 1);
        }
        break;
    default:
        SCEE_Log (29);
        SCEE_LogMsg ("invalid voctree format: corrupted node status");
        return SCE_ERROR;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

int SCE_VOctree_LoadFile (SCE_SVoxelOctree *vo, SCE_SFile *fp)
{
    SCE_SLongRect3 rect;

    vo->max_depth = SCE_Decode_StreamLong (fp);
    vo->x = SCE_Decode_StreamLong (fp);
    vo->y = SCE_Decode_StreamLong (fp);
    vo->z = SCE_Decode_StreamLong (fp);
    vo->w = SCE_Decode_StreamLong (fp);
    vo->h = SCE_Decode_StreamLong (fp);
    vo->d = SCE_Decode_StreamLong (fp);

    SCE_Rectangle3_SetFromOriginl (&rect, vo->x, vo->y, vo->z,
                                   vo->w, vo->h, vo->d);
    SCE_Rectangle3_Pow2l (&rect, vo->max_depth);

    if (SCE_VOctree_LoadNode (vo, &vo->root, fp, &rect, vo->max_depth) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    return SCE_OK;
}

int SCE_VOctree_Load (SCE_SVoxelOctree *vo, const char *fname)
{
    SCE_SFile fp;

    SCE_File_Init (&fp);
    if (SCE_File_Open (&fp, NULL, fname, SCE_FILE_READ) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    if (SCE_VOctree_LoadFile (vo, &fp) < 0) {
        SCEE_LogSrc ();
        SCE_File_Close (&fp);
        return SCE_ERROR;
    }

    SCE_File_Close (&fp);
    return SCE_OK;
}

static void SCE_VOctree_SaveNode (SCE_SVoxelOctreeNode *node, SCE_SFile *fp)
{
    size_t i;

    SCE_Encode_StreamLong (node->status, fp);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        break;
    case SCE_VOCTREE_NODE_FULL:
        SCE_Encode_StreamLong (node->material, fp);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        SCE_Encode_StreamLong (node->in_volume, fp);
        SCE_Encode_StreamLong (node->material, fp);
        break;
    case SCE_VOCTREE_NODE_NODE:
        SCE_Encode_StreamLong (node->in_volume, fp);
        SCE_Encode_StreamLong (node->material, fp);
        for (i = 0; i < 8; i++)
            SCE_VOctree_SaveNode (node->children[i], fp);
        break;
    }
}

void SCE_VOctree_SaveFile (SCE_SVoxelOctree *vo, SCE_SFile *fp)
{
    SCE_Encode_StreamLong (vo->max_depth, fp);
    SCE_Encode_StreamLong (vo->x, fp);
    SCE_Encode_StreamLong (vo->y, fp);
    SCE_Encode_StreamLong (vo->z, fp);
    SCE_Encode_StreamLong (vo->w, fp);
    SCE_Encode_StreamLong (vo->h, fp);
    SCE_Encode_StreamLong (vo->d, fp);

    SCE_VOctree_SaveNode (&vo->root, fp);
}

int SCE_VOctree_Save (SCE_SVoxelOctree *vo, const char *fname)
{
    SCE_SFile fp;

    SCE_File_Init (&fp);
    if (SCE_File_Open (&fp, NULL, fname, SCE_FILE_WRITE |SCE_FILE_CREATE) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    SCE_VOctree_SaveFile (vo, &fp);

    SCE_File_Close (&fp);
    return SCE_OK;
}


static int SCE_VOctree_DecompressNode (SCE_SVoxelOctreeNode *node)
{
    void *grid = NULL;
    size_t size;

    if (!(grid = SCE_malloc (SCE_VGrid_GetSize (&node->grid))))
        goto fail;
    SCE_VGrid_SetRaw (&node->grid, grid);

    size = SCE_File_Length (&node->file);
    /* TODO: dont do dat, go see CacheGrid() */
    if (size > 0) {
        SCE_SArray data;
        void *filedata = NULL;
        SCE_Array_Init (&data);
        /* TODO: use 'grid' pointer for 'data' and dont forget the
                 memcpy() below */
        if (!(filedata = SCE_FileCache_GetRaw (&node->file)))
            goto fail;
        /* we must be sure that the file is a FileCache file */
        if (SCE_Zlib_Decompress (filedata, size, &data) < 0)
            goto fail;
        if (SCE_Array_GetSize (&data) != SCE_VGrid_GetSize (&node->grid)) {
            SCEE_Log (76);
            SCEE_LogMsg ("corrupted voxel archive %s: size does not match",
                         node->fname);
            return SCE_ERROR;
        }
        memcpy (grid, SCE_Array_Get (&data), SCE_Array_GetSize (&data));
        SCE_Array_Clear (&data);
        node->is_sync = SCE_TRUE;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

static int SCE_VOctree_CompressNode (SCE_SVoxelOctreeNode *node)
{
    SCE_SArray data;

    SCE_Array_Init (&data);
    /* 9: maximum compression level hehe */
    if (SCE_Zlib_Compress (SCE_VGrid_GetRaw (&node->grid),
                           SCE_VGrid_GetSize (&node->grid), 9, &data) < 0)
        goto fail;

    SCE_File_Rewind (&node->file);
    SCE_File_Truncate (&node->file, SCE_Array_GetSize (&data));
    if (SCE_File_Write (SCE_Array_Get (&data), 1, SCE_Array_GetSize (&data),
                        &node->file) != SCE_Array_GetSize (&data))
        goto fail;
    SCE_Array_Clear (&data);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/* makes sure the node file data are in memory */
static int
SCE_VOctree_CacheFile (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node)
{
    if (!node->is_open) {
        if (SCE_File_Open (&node->file, vo->fs, node->fname, SCE_FILE_READ |
                           SCE_FILE_WRITE | SCE_FILE_CREATE) < 0)
            goto fail;
        node->is_open = SCE_TRUE;
    }
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/* update compressed data */
static int
SCE_VOctree_SyncNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node)
{
    if (SCE_VOctree_CacheFile (vo, node) < 0)
        goto fail;
    if (!node->is_sync) {
        if (SCE_VOctree_CompressNode (node) < 0)
            goto fail;
        node->is_sync = SCE_TRUE;
        SCEE_SendMsg ("synced %d bytes %s\n", SCE_VGrid_GetSize (&node->grid),
                      node->fname);
    } else {
    SCEE_SendMsg ("NOT synced %d bytes %s\n", SCE_VGrid_GetSize (&node->grid),
                  node->fname);
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


/* makes sure the node grid data are in memory */
static int
SCE_VOctree_CacheGrid (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node)
{
    if (!node->cached) {
        /* TODO: dont cache the file if it dont exist, see DecompressNode() */
        if (SCE_VOctree_CacheFile (vo, node) < 0)
            goto fail;
        if (SCE_VOctree_DecompressNode (node) < 0)
            goto fail;
        node->cached = SCE_TRUE;
        SCE_List_Appendl (&vo->cached, &node->it);
        vo->n_cached++;
    }
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}
/* removes a node's grid from memory */
static void
SCE_VOctree_UncacheGrid (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node)
{
    if (node->cached) {
        SCE_VGrid_SetRaw (&node->grid, NULL);
        node->cached = SCE_FALSE;
        SCE_List_Removel (&node->it);
        vo->n_cached--;
    }
}


static int
SCE_VOctree_CopyFromNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                          const SCE_SLongRect3 *node_rect,
                          const SCE_SLongRect3 *area, SCE_SVoxelGrid *grid)
{
    SCE_SLongRect3 src_region, dst_region;

    if (SCE_VOctree_CacheGrid (vo, node) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    SCE_Rectangle3_Intersectionl (node_rect, area, &dst_region);
    src_region = dst_region;
    SCE_Rectangle3_SubOriginl (&dst_region, area);
    SCE_Rectangle3_SubOriginl (&src_region, node_rect);
    SCE_VGrid_Copy (&dst_region, grid, &src_region, &node->grid);

    return SCE_OK;
}

static int
SCE_VOctree_Get (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                 const SCE_SLongRect3 *node_rect, SCEuint level, SCEuint depth,
                 const SCE_SLongRect3 *area, SCE_SVoxelGrid *grid)
{
    SCE_SLongRect3 inter;
    SCEuint clevel;
    /* TODO: hardcoded patterns */
    SCEubyte empty_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    SCEubyte full_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    full_pattern[0] = 255;

    if (!SCE_Rectangle3_Intersectionl (node_rect, area, &inter))
        return SCE_OK;

    clevel = level + depth;

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        SCE_Rectangle3_SubOriginl (&inter, area);
        SCE_VGrid_Fill (grid, &inter, empty_pattern);
        break;
    case SCE_VOCTREE_NODE_FULL:
        SCE_Rectangle3_SubOriginl (&inter, area);
        /* full_pattern[1] = node->material; */
        SCE_VGrid_Fill (grid, &inter, full_pattern);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        if (level == clevel) {
            /* copy data from current node */
            if (SCE_VOctree_CopyFromNode (vo, node, node_rect, area, grid) < 0)
                goto fail;
        } else {
            SCE_Rectangle3_SubOriginl (&inter, area);
            if (node->in_volume > (vo->w * vo->h * vo->d) / 2)
                SCE_VGrid_Fill (grid, &inter, full_pattern);
            else
                SCE_VGrid_Fill (grid, &inter, empty_pattern);
        }
        break;
    case SCE_VOCTREE_NODE_NODE:
        if (depth == 0) {
            /* copy data from current node */
            if (SCE_VOctree_CopyFromNode (vo, node, node_rect, area, grid) < 0)
                goto fail;
        } else {
            /* recurse */
            size_t i;
            SCE_SLongRect3 r;
            for (i = 0; i < 8; i++) {
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_VOctree_Get (vo, node->children[i], &r, level,
                                     depth - 1, area, grid) < 0)
                    goto fail;
            }
        }
        break;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


int SCE_VOctree_GetRegion (SCE_SVoxelOctree *vo, SCEuint level,
                           const SCE_SLongRect3 *area, SCEubyte *data)
{
    SCEuint depth;
    SCE_SLongRect3 node_rect;
    SCE_SVoxelGrid grid;

    depth = vo->max_depth - level;
    SCE_Rectangle3_SetFromOriginl (&node_rect, vo->x, vo->y, vo->z,
                                   vo->w, vo->h, vo->d);
    SCE_Rectangle3_Pow2l (&node_rect, depth);

    SCE_VGrid_Init (&grid);
    grid.w = SCE_Rectangle3_GetWidthl (area);
    grid.h = SCE_Rectangle3_GetHeightl (area);
    grid.d = SCE_Rectangle3_GetDepthl (area);
    grid.data = data;
    grid.n_cmp = SCE_VOCTREE_VOXEL_ELEMENTS;

    return SCE_VOctree_Get (vo, &vo->root, &node_rect, level, depth,
                            area, &grid);
}


static int
SCE_VOctree_CopyToNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                        const SCE_SLongRect3 *node_rect,
                        const SCE_SLongRect3 *area, const SCE_SVoxelGrid *grid,
                        long *diff)
{
    SCE_SLongRect3 src_region, dst_region;

    if (SCE_VOctree_CacheGrid (vo, node) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    SCE_Rectangle3_Intersectionl (node_rect, area, &dst_region);
    src_region = dst_region;
    SCE_Rectangle3_SubOriginl (&src_region, area);
    SCE_Rectangle3_SubOriginl (&dst_region, node_rect);
    *diff = SCE_VGrid_CopyStats (&dst_region, &node->grid, &src_region, grid);
    node->is_sync = SCE_FALSE;

    return SCE_OK;
}

static void
SCE_VOctree_EraseNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node)
{
    SCE_VOctree_UncacheGrid (vo, node);
    SCE_VGrid_Clear (&node->grid);
    SCE_VGrid_Init (&node->grid);
    SCE_File_Close (&node->file);
    remove (node->fname);
    node->is_open = SCE_FALSE;
    node->is_sync = SCE_FALSE;
    node->cached = SCE_FALSE;   /* redundant */
}

static int
SCE_VOctree_Set (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                 const SCE_SLongRect3 *node_rect, SCEuint level, SCEuint depth,
                 const SCE_SLongRect3 *area, SCE_SVoxelGrid *grid)
{
    size_t i;
    SCE_SLongRect3 inter, region, local_rect;
    long diff, x, y, z;
    SCEuint clevel;             /* current level */
    /* TODO: hardcoded patterns */
    SCEubyte empty_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    SCEubyte full_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    full_pattern[0] = 255;

    if (!SCE_Rectangle3_Intersectionl (node_rect, area, &inter))
        return SCE_OK;

    clevel = level + depth;
    local_rect = *node_rect;
    SCE_Rectangle3_GetOriginlv (&local_rect, &x, &y, &z);
    SCE_VOctree_SetNodeOrigin (node, x, y, z);
    SCE_Rectangle3_Pow2l (&local_rect, -depth);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        /* check if the grid is empty first, since the grid is more likely
           to be small, this test is quite fast and can save us useless
           file creation/deletion, which is very expensive */
        region = inter;
        SCE_Rectangle3_SubOriginl (&region, area);
        if (SCE_VGrid_IsEmpty (grid, &region))
            break;

        /* create this node */
        SCE_VOctree_MakeNodeFilename (vo, &local_rect, clevel, node->fname);
        node->level = clevel;
        SCE_VOctree_SetNodeGrid (node,
                                 SCE_Rectangle3_GetWidthl (&local_rect),
                                 SCE_Rectangle3_GetHeightl (&local_rect),
                                 SCE_Rectangle3_GetDepthl (&local_rect), 1);
        if (SCE_VOctree_CacheGrid (vo, node) < 0)
            goto fail;
        SCE_VGrid_Fill (&node->grid, NULL, empty_pattern);

        if (depth == 0) {
            /* simple copy */
            if (SCE_VOctree_CopyToNode (vo, node, &local_rect, area, grid,
                                        &diff) < 0)
                goto fail;
            node->in_volume += diff;
            if (node->in_volume == 0) {
                /* this case should never happen because we tested
                   'grid' */
                SCE_VOctree_EraseNode (vo, node);
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                SCE_VOctree_EraseNode (vo, node);
                node->status = SCE_VOCTREE_NODE_FULL;
            } else
                node->status = SCE_VOCTREE_NODE_LEAF;
        } else {
            node->status = SCE_VOCTREE_NODE_NODE;
            /* create 8 children and recurse */
            for (i = 0; i < 8; i++) {
                if (!(node->children[i] = SCE_VOctree_CreateNode ()))
                    goto fail;
                node->children[i]->status = SCE_VOCTREE_NODE_EMPTY;
                node->children[i]->in_volume = 0;
            }
            for (i = 0; i < 8; i++) {
                SCE_SLongRect3 r;
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_VOctree_Set (vo, node->children[i], &r, level,
                                     depth - 1, area, grid) < 0)
                    goto fail;
            }
        }
        break;
    case SCE_VOCTREE_NODE_FULL:
        /* first check if the grid is full, since the grid is more likely
           to be small, this test is quite fast and can save us useless
           file creation/deletion, which is very expensive */
        region = inter;
        SCE_Rectangle3_SubOriginl (&region, area);
        if (SCE_VGrid_IsFull (grid, &region))
            break;

        /* create this node */
        SCE_VOctree_MakeNodeFilename (vo, &local_rect, clevel, node->fname);
        node->level = clevel;
        SCE_VOctree_SetNodeGrid (node,
                                 SCE_Rectangle3_GetWidthl (&local_rect),
                                 SCE_Rectangle3_GetHeightl (&local_rect),
                                 SCE_Rectangle3_GetDepthl (&local_rect), 1);
        if (SCE_VOctree_CacheGrid (vo, node) < 0)
            goto fail;
        SCE_VGrid_Fill (&node->grid, NULL, full_pattern);

        if (depth == 0) {
            /* simple copy */
            if (SCE_VOctree_CopyToNode (vo, node, &local_rect, area, grid,
                                        &diff) < 0)
                goto fail;
            node->in_volume += diff;
            if (node->in_volume == 0) {
                SCE_VOctree_EraseNode (vo, node);
                node->status = SCE_VOCTREE_NODE_EMPTY;
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                /* this case should never happend because we tested
                   'grid' */
                SCE_VOctree_EraseNode (vo, node);
            } else
                node->status = SCE_VOCTREE_NODE_LEAF;
        } else {
            node->status = SCE_VOCTREE_NODE_NODE;
            /* create 8 children and recurse */
            for (i = 0; i < 8; i++) {
                if (!(node->children[i] = SCE_VOctree_CreateNode ()))
                    goto fail;
                node->children[i]->status = SCE_VOCTREE_NODE_FULL;
                node->children[i]->in_volume = vo->w * vo->h * vo->d;
            }
            for (i = 0; i < 8; i++) {
                SCE_SLongRect3 r;
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_VOctree_Set (vo, node->children[i], &r, level,
                                     depth - 1, area, grid) < 0)
                    goto fail;
            }
        }
        break;
    case SCE_VOCTREE_NODE_LEAF:
        if (depth == 0) {
            /* simple copy */
            if (SCE_VOctree_CopyToNode (vo, node, &local_rect, area, grid,
                                        &diff) < 0)
                goto fail;
            /* note that the file already exists if the octree has been
               exclusively created using this function, according to
               NODE_FULL/NODE_EMPTY cases */
            node->in_volume += diff;
            if (node->in_volume == 0) {
                SCE_VOctree_EraseNode (vo, node);
                node->status = SCE_VOCTREE_NODE_EMPTY;
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                SCE_VOctree_EraseNode (vo, node);
                node->status = SCE_VOCTREE_NODE_FULL;
            }
        } else {
            SCE_EVoxelOctreeStatus status = SCE_VOCTREE_NODE_EMPTY;
            long in_volume = 0;
            node->status = SCE_VOCTREE_NODE_NODE;

            if (node->in_volume > (vo->w * vo->h * vo->d) / 2) { /* tkt. */
                status = SCE_VOCTREE_NODE_FULL;
                in_volume = vo->w * vo->h * vo->d;
            }

            /* create 8 children and recurse */
            for (i = 0; i < 8; i++) {
                if (!(node->children[i] = SCE_VOctree_CreateNode ()))
                    goto fail;
                node->children[i]->status = status;
                node->children[i]->in_volume = in_volume;
            }
            for (i = 0; i < 8; i++) {
                SCE_SLongRect3 r;
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_VOctree_Set (vo, node->children[i], &r, level,
                                     depth - 1, area, grid) < 0)
                    goto fail;
            }
        }
        break;
    case SCE_VOCTREE_NODE_NODE:
        if (depth == 0) {
            /* simple copy */
            if (SCE_VOctree_CopyToNode (vo, node, &local_rect, area, grid,
                                        &diff) < 0)
                goto fail;
            node->in_volume += diff;
        } else {
            int j = 0;
            /* recurse */
            for (i = 0; i < 8; i++) {
                SCE_SLongRect3 r;
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_VOctree_Set (vo, node->children[i], &r, level,
                                     depth - 1, area, grid) < 0)
                    goto fail;
                if (node->children[i]->status == SCE_VOCTREE_NODE_EMPTY)
                    j++;
                else if (node->children[i]->status == SCE_VOCTREE_NODE_FULL)
                    j--;
            }
            if (j == 8 || j == -8) {
                SCE_VOctree_DeleteChildren (node);
                SCE_VOctree_EraseNode (vo, node);
                node->status = SCE_VOCTREE_NODE_FULL;
                if (j == 8)
                    node->status = SCE_VOCTREE_NODE_EMPTY;
            }
        }
        break;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

int SCE_VOctree_SetRegion (SCE_SVoxelOctree *vo, SCEuint level,
                           const SCE_SLongRect3 *area, const SCEubyte *data)
{
    SCEuint depth;
    SCE_SLongRect3 node_rect;
    SCE_SVoxelGrid grid;

    depth = vo->max_depth - level;
    SCE_Rectangle3_SetFromOriginl (&node_rect, vo->x, vo->y, vo->z,
                                   vo->w, vo->h, vo->d);
    SCE_Rectangle3_Pow2l (&node_rect, depth);

    SCE_VGrid_Init (&grid);
    grid.w = SCE_Rectangle3_GetWidthl (area);
    grid.h = SCE_Rectangle3_GetHeightl (area);
    grid.d = SCE_Rectangle3_GetDepthl (area);
    grid.data = data;
    grid.n_cmp = SCE_VOCTREE_VOXEL_ELEMENTS;

    return SCE_VOctree_Set (vo, &vo->root, &node_rect, level, depth,
                            area, &grid);
}


static SCE_EVoxelOctreeStatus
SCE_VOctree_Node (const SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                  const SCE_SLongRect3 *node_rect, SCEuint level,
                  SCEuint depth, long x, long y, long z, char *fname)
{
    SCEuint clevel;

    clevel = level + depth;

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
    case SCE_VOCTREE_NODE_FULL:
        return node->status;
    case SCE_VOCTREE_NODE_LEAF:
        if (level == clevel) {
            strcpy (fname, node->fname);
            return node->status;
        } else {
            if (node->in_volume > (vo->w * vo->h * vo->d) / 2)
                return SCE_VOCTREE_NODE_FULL;
            else
                return SCE_VOCTREE_NODE_EMPTY;
        }
    case SCE_VOCTREE_NODE_NODE:
        if (depth == 0) {
            strcpy (fname, node->fname);
            return node->status;
        } else {
            /* recurse */
            size_t i;
            SCE_SLongRect3 r;
            for (i = 0; i < 8; i++) {
                SCE_VOctree_ConstructRect (node_rect, i, &r);
                if (SCE_Rectangle3_IsInl (&r, x, y, z)) {
                    return SCE_VOctree_Node (vo, node->children[i], &r, level,
                                             depth - 1, x, y, z, fname);
                }
            }
        }
    }

#ifdef SCE_DEBUG
    SCEE_SendMsg ("voctree: querying a node that is not in the tree\n");
#endif
    return SCE_VOCTREE_NODE_EMPTY; /* wtf? */
}


int SCE_VOctree_GetNode (SCE_SVoxelOctree *vo, SCEuint level, long x, long y,
                         long z, char *fname)
{
    SCEuint depth;
    SCE_SLongRect3 node_rect;

    depth = vo->max_depth - level;
    SCE_Rectangle3_SetFromOriginl (&node_rect, vo->x, vo->y, vo->z,
                                   vo->w, vo->h, vo->d);
    SCE_Rectangle3_Pow2l (&node_rect, depth);

    if (SCE_Rectangle3_IsInl (&node_rect, x, y, z))
        return SCE_VOctree_Node (vo, &vo->root, &node_rect, level, depth,
                                 x, y, z, fname);
    else
        return -1;
}


void SCE_VOctree_UpdateCache (SCE_SVoxelOctree *vo)
{
    while (vo->n_cached > vo->max_cached) {
        SCE_SVoxelOctreeNode *node = NULL;
        node = SCE_List_GetData (SCE_List_GetFirst (&vo->cached));
        SCE_VOctree_SyncNode (vo, node);
        SCE_VOctree_UncacheGrid (vo, node);
    }
}

int SCE_VOctree_SyncCache (SCE_SVoxelOctree *vo)
{
    SCE_SListIterator *it = NULL;

    SCE_List_ForEach (it, &vo->cached) {
        SCE_SVoxelOctreeNode *node = NULL;
        node = SCE_List_GetData (it);
        if (SCE_VOctree_SyncNode (vo, node) < 0)
            goto fail;
    }

    /* files have potentially been open */
    /* NOTE: maybe this operation should be performed in the loop above */
    SCE_FileCache_Update (vo->fcache);
    if (SCE_FileCache_Sync (vo->fcache) < 0)
        goto fail;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

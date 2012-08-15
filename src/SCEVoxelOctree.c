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
   updated: 15/08/2012 */

#include <time.h>
#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVoxelOctree.h"

static void SCE_VOctree_InitNode (SCE_SVoxelOctreeNode *node)
{
    size_t i;
    node->status = SCE_VOCTREE_NODE_EMPTY;
    for (i = 0; i < 8; i++)
        node->children[i] = NULL;
    SCE_VFile_Init (&node->vf);
    node->in_volume = 0;
    node->material = 0;
}
static void SCE_VOctree_DeleteNode (SCE_SVoxelOctreeNode*);
static void SCE_VOctree_ClearNode (SCE_SVoxelOctreeNode *node)
{
    size_t i;
    for (i = 0; i < 8; i++)
        SCE_VOctree_DeleteNode (node->children[i]);
    SCE_VFile_Close (&node->vf);
    SCE_VFile_Clear (&node->vf);
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


void SCE_VOctree_Init (SCE_SVoxelOctree *vo)
{
    SCE_VOctree_InitNode (&vo->root);
    vo->max_depth = 0;
    vo->x = vo->y = vo->z = 0;
    vo->w = vo->h = vo->d = 0;
    memset (vo->prefix, 0, sizeof vo->prefix);
    vo->fs = NULL;
    vo->fcache = NULL;
}
void SCE_VOctree_Clear (SCE_SVoxelOctree *vo)
{
    SCE_VOctree_ClearNode (&vo->root);
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
void SCE_VOctree_SetFileCache (SCE_SVoxelOctree *vo, SCE_SGZFileCache *cache)
{
    vo->fcache = cache;
}


void SCE_VOctree_GetOriginv (const SCE_SVoxelOctree *vo, long *x, long *y,
                             long *z)
{
    *x = vo->x;
    *y = vo->y;
    *z = vo->z;
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


static int
SCE_VOctree_LoadNode (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                      FILE *fp)
{
    size_t i;

    fread (&node->status, sizeof node->status, 1, fp);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        break;
    case SCE_VOCTREE_NODE_FULL:
        node->in_volume = vo->w * vo->h * vo->d;
        fread (&node->material, sizeof node->material, 1, fp);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        fread (&node->in_volume, sizeof node->in_volume, 1, fp);
        fread (&node->material, sizeof node->material, 1, fp);
        break;
    case SCE_VOCTREE_NODE_NODE:
        fread (&node->in_volume, sizeof node->in_volume, 1, fp);
        fread (&node->material, sizeof node->material, 1, fp);
        for (i = 0; i < 8; i++) {
            if (!(node->children[i] = SCE_VOctree_CreateNode ()))
                goto fail;
            SCE_VOctree_LoadNode (vo, node->children[i], fp);
        }
        break;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

int SCE_VOctree_Load (SCE_SVoxelOctree *vo, const char *fname)
{
    FILE *fp = NULL;

    if (!(fp = fopen (fname, "rb"))) {
        SCEE_LogErrno (fname);
        return SCE_ERROR;
    }

    fread (&vo->max_depth, sizeof vo->max_depth, 1, fp);
    fread (&vo->x, sizeof vo->x, 1, fp);
    fread (&vo->y, sizeof vo->y, 1, fp);
    fread (&vo->z, sizeof vo->z, 1, fp);
    fread (&vo->w, sizeof vo->w, 1, fp);
    fread (&vo->h, sizeof vo->h, 1, fp);
    fread (&vo->d, sizeof vo->d, 1, fp);

    if (SCE_VOctree_LoadNode (vo, &vo->root, fp) < 0) {
        SCEE_LogSrc ();
        fclose (fp);
        return SCE_ERROR;
    }

    fclose (fp);

    return SCE_OK;
}

static void SCE_VOctree_SaveNode (SCE_SVoxelOctreeNode *node, FILE *fp)
{
    size_t i;

    fwrite (&node->status, sizeof node->status, 1, fp);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        break;
    case SCE_VOCTREE_NODE_FULL:
        fwrite (&node->material, sizeof node->material, 1, fp);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        fwrite (&node->in_volume, sizeof node->in_volume, 1, fp);
        fwrite (&node->material, sizeof node->material, 1, fp);
        break;
    case SCE_VOCTREE_NODE_NODE:
        fwrite (&node->in_volume, sizeof node->in_volume, 1, fp);
        fwrite (&node->material, sizeof node->material, 1, fp);
        for (i = 0; i < 8; i++)
            SCE_VOctree_SaveNode (node->children[i], fp);
        break;
    }
}

int SCE_VOctree_Save (SCE_SVoxelOctree *vo, const char *fname)
{
    FILE *fp = NULL;

    if (!(fp = fopen (fname, "wb"))) {
        SCEE_LogErrno (fname);
        return SCE_ERROR;
    }

    fwrite (&vo->max_depth, sizeof vo->max_depth, 1, fp);
    fwrite (&vo->x, sizeof vo->x, 1, fp);
    fwrite (&vo->y, sizeof vo->y, 1, fp);
    fwrite (&vo->z, sizeof vo->z, 1, fp);
    fwrite (&vo->w, sizeof vo->w, 1, fp);
    fwrite (&vo->h, sizeof vo->h, 1, fp);
    fwrite (&vo->d, sizeof vo->d, 1, fp);

    SCE_VOctree_SaveNode (&vo->root, fp);

    fclose (fp);

    return SCE_OK;
}

static void
SCE_VOctree_GetNodeFilename (const SCE_SVoxelOctree *vo,
                             const SCE_SLongRect3 *node_rect, SCEuint level,
                             char *fname)
{
    long p1[3], p2[3];
    SCE_Rectangle3_GetPointslv (node_rect, p1, p2);
    sprintf (fname, "%s/lod%u/%ld_%ld_%ld", vo->prefix, level,
             p1[0], p1[1], p1[2]);
}

static int
SCE_VOctree_CopyFromFile (const SCE_SVoxelOctree *vo, SCE_SVoxelFile *vf,
                          const SCE_SLongRect3 *node_rect, SCEuint level,
                          const SCE_SLongRect3 *area, SCE_SVoxelGrid *grid)
{
    SCE_SLongRect3 src_region, dst_region;

    if (!SCE_VFile_IsOpen (vf)) {
        char fname[256] = {0};

        SCE_VOctree_GetNodeFilename (vo, node_rect, level, fname);

        SCE_VFile_SetDimensions (vf, SCE_Rectangle3_GetWidthl (node_rect),
                                 SCE_Rectangle3_GetHeightl (node_rect),
                                 SCE_Rectangle3_GetDepthl (node_rect));
        SCE_VFile_SetNumComponents (vf, grid->n_cmp);
        if (SCE_VFile_Open (vf, vo->fs, vo->fcache, fname) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }

    SCE_Rectangle3_Intersectionl (node_rect, area, &dst_region);
    src_region = dst_region;
    SCE_Rectangle3_SubOriginl (&dst_region, area);
    SCE_Rectangle3_SubOriginl (&src_region, node_rect);
    SCE_VFile_GetRegion (vf, &src_region, grid, &dst_region);

    return SCE_OK;
}

static void SCE_VOctree_ConstructRect (const SCE_SLongRect3 *parent,
                                       SCEuint id, SCE_SLongRect3 *child)
{
    long p1[3], p2[3];
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
SCE_VOctree_Get (const SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
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
//        full_pattern[1] = node->material;
        SCE_VGrid_Fill (grid, &inter, full_pattern);
        break;
    case SCE_VOCTREE_NODE_LEAF:
        if (level == clevel) {
            /* copy data from current node */
            if (SCE_VOctree_CopyFromFile (vo, &node->vf, node_rect, level,
                                          area, grid) < 0)
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
            if (SCE_VOctree_CopyFromFile (vo, &node->vf, node_rect, level,
                                          area, grid) < 0)
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


int SCE_VOctree_GetRegion (const SCE_SVoxelOctree *vo, SCEuint level,
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
SCE_VOctree_CopyToFile (SCE_SVoxelOctree *vo, SCE_SVoxelFile *vf,
                        const SCE_SLongRect3 *node_rect,
                        const SCE_SLongRect3 *area, const SCE_SVoxelGrid *grid,
                        SCE_SVoxelFileStats *diff)
{
    SCE_SLongRect3 src_region, dst_region;

    SCE_Rectangle3_Intersectionl (node_rect, area, &dst_region);
    src_region = dst_region;
    SCE_Rectangle3_SubOriginl (&src_region, area);
    SCE_Rectangle3_SubOriginl (&dst_region, node_rect);
    SCE_VFile_SetRegion (vf, &dst_region, grid, &src_region);
    SCE_VFile_GetStatsv (vf, diff);

    return SCE_OK;
}

static int
SCE_VOctree_Set (SCE_SVoxelOctree *vo, SCE_SVoxelOctreeNode *node,
                 const SCE_SLongRect3 *node_rect, SCEuint level, SCEuint depth,
                 const SCE_SLongRect3 *area, const SCE_SVoxelGrid *grid)
{
    size_t i;
    SCE_SLongRect3 inter, region, local_rect;
    SCE_SVoxelFileStats diff;
    char fname[256] = {0};
    SCEuint clevel;             /* current level */
    /* TODO: hardcoded patterns */
    SCEubyte empty_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    SCEubyte full_pattern[SCE_VOCTREE_VOXEL_ELEMENTS] = {0};
    full_pattern[0] = 255;

    if (!SCE_Rectangle3_Intersectionl (node_rect, area, &inter))
        return SCE_OK;

    clevel = level + depth;
    local_rect = *node_rect;
    SCE_Rectangle3_Pow2l (&local_rect, -depth);

    switch (node->status) {
    case SCE_VOCTREE_NODE_EMPTY:
        /* first check if the grid is empty, since the grid is more likely
           to be small, this test is quite fast and can save us useless
           file creation/deletion, which is very expensive */
        region = inter;
        SCE_Rectangle3_SubOriginl (&region, area);
        if (SCE_VGrid_IsEmpty (grid, &region))
            break;

        /* create this node */
        SCE_VOctree_GetNodeFilename (vo, &local_rect, clevel, fname);
        if (SCE_VFile_Open (&node->vf, vo->fs, vo->fcache, fname) < 0)
            goto fail;
        SCE_VFile_SetDimensions (&node->vf,
                                 SCE_Rectangle3_GetWidthl (&local_rect),
                                 SCE_Rectangle3_GetHeightl (&local_rect),
                                 SCE_Rectangle3_GetDepthl (&local_rect));
        SCE_VFile_SetNumComponents (&node->vf, grid->n_cmp);
        SCE_VFile_Fill (&node->vf, empty_pattern);

        if (depth == 0) {
            /* simple copy */
            SCE_VOctree_CopyToFile (vo, &node->vf, &local_rect, area, grid,
                                    &diff);
            node->in_volume += diff.in_volume;
            if (node->in_volume == 0) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
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
        SCE_VOctree_GetNodeFilename (vo, &local_rect, clevel, fname);
        if (SCE_VFile_Open (&node->vf, vo->fs, vo->fcache, fname) < 0)
            goto fail;
        SCE_VFile_SetDimensions (&node->vf,
                                 SCE_Rectangle3_GetWidthl (&local_rect),
                                 SCE_Rectangle3_GetHeightl (&local_rect),
                                 SCE_Rectangle3_GetDepthl (&local_rect));
        SCE_VFile_SetNumComponents (&node->vf, grid->n_cmp);
        SCE_VFile_Fill (&node->vf, full_pattern);

        if (depth == 0) {
            /* simple copy */
            SCE_VOctree_CopyToFile (vo, &node->vf, &local_rect, area, grid,
                                    &diff);
            node->in_volume += diff.in_volume;
            if (node->in_volume == 0) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
                node->status = SCE_VOCTREE_NODE_EMPTY;
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
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
            SCE_VOctree_GetNodeFilename (vo, &local_rect, clevel, fname);
            /* simple copy */
            SCE_VOctree_CopyToFile (vo, &node->vf, &local_rect, area, grid,
                                    &diff);
            /* note that the file already exists if the octree has been
               exclusively created using this function, according to
               NODE_FULL/NODE_EMPTY cases */
            node->in_volume += diff.in_volume;
            if (node->in_volume == 0) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
                node->status = SCE_VOCTREE_NODE_EMPTY;
            } else if (node->in_volume == vo->w * vo->h * vo->d) {
                SCE_VFile_Close (&node->vf);
                remove (fname);
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
            if (!SCE_VFile_IsOpen (&node->vf)) {
                SCE_VOctree_GetNodeFilename (vo, &local_rect, level, fname);
                SCE_VFile_SetDimensions (&node->vf,
                                         SCE_Rectangle3_GetWidthl (&local_rect),
                                         SCE_Rectangle3_GetHeightl(&local_rect),
                                         SCE_Rectangle3_GetDepthl(&local_rect));
                SCE_VFile_SetNumComponents (&node->vf, grid->n_cmp);

                if (SCE_VFile_Open (&node->vf, vo->fs, vo->fcache, fname) < 0) {
                    SCEE_LogSrc ();
                    return SCE_ERROR;
                }
            }
            SCE_VOctree_CopyToFile (vo, &node->vf, &local_rect, area, grid,
                                    &diff);
            node->in_volume += diff.in_volume;
        } else {
            int j = 0;
            SCE_VOctree_GetNodeFilename (vo, &local_rect, clevel, fname);
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
                SCE_VFile_Close (&node->vf);
                remove (fname);
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
            SCE_VOctree_GetNodeFilename (vo, node_rect, level, fname);
            return node->status;
        } else {
            if (node->in_volume > (vo->w * vo->h * vo->d) / 2)
                return SCE_VOCTREE_NODE_FULL;
            else
                return SCE_VOCTREE_NODE_EMPTY;
        }
    case SCE_VOCTREE_NODE_NODE:
        if (depth == 0) {
            SCE_VOctree_GetNodeFilename (vo, node_rect, level, fname);
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

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
   updated: 15/05/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEVoxelOctree.h"
#include "SCE/core/SCEVoxelWorld.h"


static void SCE_VWorld_InitTree (SCE_SVoxelWorldTree *wt)
{
    SCE_VOctree_Init (&wt->vo);
    SCE_List_InitIt (&wt->it);
    SCE_List_SetData (&wt->it, wt);
}
static void SCE_VWorld_ClearTree (SCE_SVoxelWorldTree *wt)
{
    SCE_VOctree_Clear (&wt->vo);
    SCE_List_Remove (&wt->it);
}
static SCE_SVoxelWorldTree* SCE_VWorld_CreateTree (void)
{
    SCE_SVoxelWorldTree *wt = NULL;
    if (!(wt = SCE_malloc (sizeof *wt)))
        SCEE_LogSrc ();
    else
        SCE_VWorld_InitTree (wt);
    return wt;
}
static void SCE_VWorld_DeleteTree (SCE_SVoxelWorldTree *wt)
{
    if (wt) {
        SCE_VWorld_ClearTree (wt);
        SCE_free (wt);
    }
}

static void SCE_VWorld_FreeFunc (void *wt)
{
    SCE_VWorld_DeleteTree (wt);
}

void SCE_VWorld_Init (SCE_SVoxelWorld *vw)
{
    size_t i;

    SCE_List_Init (&vw->trees);
    SCE_List_SetFreeFunc (&vw->trees, SCE_VWorld_FreeFunc);
    vw->w = vw->h = vw->d = 0;
    vw->n_lod = 1;
    memset (vw->prefix, 0, sizeof vw->prefix);
    vw->fs = NULL;
    vw->fcache = NULL;
    vw->max_cached_nodes = 16;

    for (i = 0; i < SCE_MAX_VWORLD_UPDATE_ZONES; i++) {
        SCE_Rectangle3_Initl (&vw->zones[i]);
        vw->zones_level[i] = 0;
    }
    vw->last = vw->first = 0;

    vw->buffer1 = vw->buffer2 = NULL;
    vw->size1 = vw->size2 = 0;
}
void SCE_VWorld_Clear (SCE_SVoxelWorld *vw)
{
    SCE_List_Clear (&vw->trees);
    SCE_free (vw->buffer1);
    SCE_free (vw->buffer2);
}
SCE_SVoxelWorld* SCE_VWorld_Create (void)
{
    SCE_SVoxelWorld *vw = NULL;
    if (!(vw = SCE_malloc (sizeof *vw)))
        SCEE_LogSrc ();
    else
        SCE_VWorld_Init (vw);
    return vw;
}
void SCE_VWorld_Delete (SCE_SVoxelWorld *vw)
{
    if (vw) {
        SCE_VWorld_Clear (vw);
        SCE_free (vw);
    }
}


void SCE_VWorld_SetDimensions (SCE_SVoxelWorld *vw, SCEulong w, SCEulong h,
                               SCEulong d)
{
    vw->w = w;
    vw->h = h;
    vw->d = d;
}
SCEulong SCE_VWorld_GetWidth (const SCE_SVoxelWorld *vw)
{
    return vw->w;
}
SCEulong SCE_VWorld_GetHeight (const SCE_SVoxelWorld *vw)
{
    return vw->h;
}
SCEulong SCE_VWorld_GetDepth (const SCE_SVoxelWorld *vw)
{
    return vw->d;
}
SCEulong SCE_VWorld_GetTotalWidth (const SCE_SVoxelWorld *vw)
{
    return vw->w << (vw->n_lod - 1);
}
SCEulong SCE_VWorld_GetTotalHeight (const SCE_SVoxelWorld *vw)
{
    return vw->h << (vw->n_lod - 1);
}
SCEulong SCE_VWorld_GetTotalDepth (const SCE_SVoxelWorld *vw)
{
    return vw->d << (vw->n_lod - 1);
}


void SCE_VWorld_SetNumLevels (SCE_SVoxelWorld *vw, SCEuint level)
{
    vw->n_lod = level;
}
SCEuint SCE_VWorld_GetNumLevels (SCE_SVoxelWorld *vw)
{
    return vw->n_lod;
}
void SCE_VWorld_SetPrefix (SCE_SVoxelWorld *vw, const char *prefix)
{
    strncpy (vw->prefix, prefix, sizeof vw->prefix - 1);
}


/**
 * \brief Must be called before any octree is added to the world
 * \param vw voxel world
 * \param fs file system
 */
void SCE_VWorld_SetFileSystem (SCE_SVoxelWorld *vw, SCE_SFileSystem *fs)
{
    vw->fs = fs;
}
/**
 * \brief Must be called before any octree is added to the world
 * \param vw voxel world
 * \param fs file system
 */
void SCE_VWorld_SetFileCache (SCE_SVoxelWorld *vw, SCE_SFileCache *cache)
{
    vw->fcache = cache;
}
void SCE_VWorld_SetMaxCachedNodes (SCE_SVoxelWorld *vw, SCEulong max_cached)
{
    vw->max_cached_nodes = max_cached;
}


static void
SCE_VWorld_SetTreePrefix (char *prefix, const SCE_SVoxelWorld *vw,
                          long x, long y, long z)
{
    sprintf (prefix, "%s/region_%ld_%ld_%ld", vw->prefix, x, y, z);
}

SCE_SVoxelWorldTree* SCE_VWorld_AddNewTree (SCE_SVoxelWorld *vw,
                                            long x, long y, long z)
{
    char prefix[128] = {0};
    SCE_SVoxelWorldTree *wt = NULL;

    if (!(wt = SCE_VWorld_CreateTree ())) {
        SCEE_LogSrc ();
        return NULL;
    }

    SCE_VWorld_SetTreePrefix (prefix, vw, x, y, z);
    /* TODO: create directory */

    SCE_VOctree_SetMaxDepth (&wt->vo, vw->n_lod - 1);
    SCE_VOctree_SetOrigin (&wt->vo, x * vw->w, y * vw->h, z * vw->d);
    SCE_VOctree_SetDimensions (&wt->vo, vw->w, vw->h, vw->d);
    SCE_VOctree_SetPrefix (&wt->vo, prefix);
    SCE_VOctree_SetFileSystem (&wt->vo, vw->fs);
    SCE_VOctree_SetFileCache (&wt->vo, vw->fcache);
    SCE_VOctree_SetMaxCachedNodes (&wt->vo, vw->max_cached_nodes);
    SCE_List_Appendl (&vw->trees, &wt->it);

    return wt;
}
SCE_SVoxelWorldTree* SCE_VWorld_GetTree (SCE_SVoxelWorld *vw,
                                         long x, long y, long z)
{
    SCE_SListIterator *it = NULL;
    long a, b, c;

    x *= vw->w;
    y *= vw->h;
    z *= vw->d;

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        SCE_VOctree_GetOriginv (&wt->vo, &a, &b, &c);
        if (a == x && b == y && c == z)
            return wt;
    }
    return NULL;
}


int SCE_VWorld_Build (SCE_SVoxelWorld *vw)
{
    vw->size1 = vw->w * vw->h * vw->d;
    vw->size2 = vw->size1 / 8;

    if (!(vw->buffer1 = SCE_malloc (vw->size1))) goto fail;
    if (!(vw->buffer2 = SCE_malloc (vw->size2))) goto fail;

    memset (vw->buffer1, 0, vw->size1);
    memset (vw->buffer2, 0, vw->size2);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


int SCE_VWorld_Load (SCE_SVoxelWorld *vw, const char *fname)
{
    SCE_SFile fp;
    unsigned int n = 0, i;

    SCE_File_Init (&fp);
    if (SCE_File_Open (&fp, NULL, fname, SCE_FILE_READ) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    /* need those data for AddNewTree() */
    vw->w = SCE_Decode_StreamLong (&fp);
    vw->h = SCE_Decode_StreamLong (&fp);
    vw->d = SCE_Decode_StreamLong (&fp);
    vw->n_lod = SCE_Decode_StreamLong (&fp);

    n = SCE_Decode_StreamLong (&fp);

    for (i = 0; i < n; i++) {
        long coord[3];
        SCE_SVoxelOctree *vo = NULL;
        SCE_SVoxelWorldTree *wt = NULL;

        coord[0] = SCE_Decode_StreamLong (&fp);
        coord[1] = SCE_Decode_StreamLong (&fp);
        coord[2] = SCE_Decode_StreamLong (&fp);
        if (SCE_VWorld_AddNewTree (vw, coord[0], coord[1], coord[2]) < 0) {
            SCEE_LogSrc ();
            SCE_File_Close (&fp);
            return SCE_ERROR;
        }
    }

    SCE_File_Close (&fp);

    return SCE_OK;
}

int SCE_VWorld_Save (const SCE_SVoxelWorld *vw, const char *fname)
{
    SCE_SFile fp;
    unsigned int n = 0;
    SCE_SListIterator *it = NULL;

    SCE_File_Init (&fp);
    if (SCE_File_Open (&fp, NULL, fname, SCE_FILE_WRITE |SCE_FILE_CREATE) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    SCE_Encode_StreamLong (vw->w, &fp);
    SCE_Encode_StreamLong (vw->h, &fp);
    SCE_Encode_StreamLong (vw->d, &fp);
    SCE_Encode_StreamLong (vw->n_lod, &fp);

    n = SCE_List_GetLength (&vw->trees);
    SCE_Encode_StreamLong (n, &fp);

    SCE_List_ForEach (it, &vw->trees) {
        long x, y, z;
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);

        SCE_VOctree_GetOriginv (&wt->vo, &x, &y, &z);
        x /= vw->w;
        y /= vw->h;
        z /= vw->d;
        SCE_Encode_StreamLong (x, &fp);
        SCE_Encode_StreamLong (y, &fp);
        SCE_Encode_StreamLong (z, &fp);
    }

    SCE_File_Close (&fp);

    return SCE_OK;
}


int SCE_VWorld_LoadTree (SCE_SVoxelWorld *vw, long x, long y, long z)
{
    SCE_SVoxelWorldTree *wt = NULL;
    char prefix[128] = {0};

    wt = SCE_VWorld_GetTree (vw, x, y, z);
    if (!wt) {
        SCEE_Log (43333);
        SCEE_LogMsg ("no tree at coordinates %ld %ld %ld", x, y, z);
        return SCE_ERROR;
    }

    SCE_VWorld_SetTreePrefix (prefix, vw, x, y, z);
    /* TODO: use a macro for "octree.bin" */
    strncat (prefix, "/octree.bin", sizeof prefix - strlen (prefix) - 1);
    if (SCE_VOctree_Load (&wt->vo, prefix) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    return SCE_OK;
}
int SCE_VWorld_LoadAllTrees (SCE_SVoxelWorld *vw)
{
    SCE_SListIterator *it = NULL;
    long x, y, z;
    char prefix[128] = {0};

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);

        SCE_VOctree_GetOriginv (&wt->vo, &x, &y, &z);
        x /= vw->w;
        y /= vw->h;
        z /= vw->d;
        SCE_VWorld_SetTreePrefix (prefix, vw, x, y, z);
        /* TODO: use a macro for "octree.bin" */
        strncat (prefix, "/octree.bin", sizeof prefix - strlen (prefix) - 1);
        if (SCE_VOctree_Load (&wt->vo, prefix) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }

    return SCE_OK;
}

int SCE_VWorld_SaveTree (SCE_SVoxelWorld *vw, long x, long y, long z)
{
    SCE_SVoxelWorldTree *wt = NULL;
    char prefix[128] = {0};

    wt = SCE_VWorld_GetTree (vw, x, y, z);
    if (!wt) {
        SCEE_Log (43333);
        SCEE_LogMsg ("no tree at coordinates %ld %ld %ld", x, y, z);
        return SCE_ERROR;
    }

    SCE_VWorld_SetTreePrefix (prefix, vw, x, y, z);
    /* TODO: use a macro for "octree.bin" */
    strncat (prefix, "/octree.bin", sizeof prefix - strlen (prefix) - 1);
    if (SCE_VOctree_Save (&wt->vo, prefix) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    return SCE_OK;
}
int SCE_VWorld_SaveAllTrees (SCE_SVoxelWorld *vw)
{
    SCE_SListIterator *it = NULL;
    long x, y, z;
    char prefix[128] = {0};

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);

        SCE_VOctree_GetOriginv (&wt->vo, &x, &y, &z);
        x /= vw->w;
        y /= vw->h;
        z /= vw->d;
        SCE_VWorld_SetTreePrefix (prefix, vw, x, y, z);
        /* TODO: use a macro for "octree.bin" */
        strncat (prefix, "/octree.bin", sizeof prefix - strlen (prefix) - 1);
        if (SCE_VOctree_Save (&wt->vo, prefix) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }

    return SCE_OK;
}


static void SCE_VWorld_PushZone (SCE_SVoxelWorld *vw,
                                 const SCE_SLongRect3 *r, int level)
{
    vw->zones[vw->last] = *r;
    vw->zones_level[vw->last] = level;
    vw->last = SCE_Math_Ring (vw->last + 1, SCE_MAX_VWORLD_UPDATE_ZONES);
}
static int SCE_VWorld_PopZone (SCE_SVoxelWorld *vw, SCE_SLongRect3 *r)
{
    if (vw->first == vw->last)
        return -1;
    else {
        int level = vw->zones_level[vw->first];
        *r = vw->zones[vw->first];
        vw->first = SCE_Math_Ring (vw->first + 1, SCE_MAX_VWORLD_UPDATE_ZONES);
        return level;
    }
}


int SCE_VWorld_GetRegion (SCE_SVoxelWorld *vw, SCEuint level,
                          const SCE_SLongRect3 *region,
                          SCEubyte *data)
{
    SCE_SListIterator *it = NULL;

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelOctree *vo = NULL;
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        vo = &wt->vo;
        if (SCE_VOctree_GetRegion (vo, level, region, data) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }

    return SCE_OK;
}
static int
SCE_VWorld_Set (SCE_SVoxelWorld *vw, SCEuint level,
                const SCE_SLongRect3 *region, const SCEubyte *data)
{
    SCE_SListIterator *it = NULL;

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelOctree *vo = NULL;
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        vo = &wt->vo;
        if (SCE_VOctree_SetRegion (vo, level, region, data) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }

    SCE_VWorld_PushZone (vw, region, level);

    return SCE_OK;
}
int SCE_VWorld_SetRegion (SCE_SVoxelWorld *vw, const SCE_SLongRect3 *region,
                          const SCEubyte *data)
{
    return SCE_VWorld_Set (vw, 0, region, data);
}

int SCE_VWorld_GetNextUpdatedRegion (SCE_SVoxelWorld *vw, SCE_SLongRect3 *zone)
{
    return SCE_VWorld_PopZone (vw, zone);
}



static void
SCE_VWorld_ComputeLODPoint (SCE_SVoxelGrid *in, SCE_SVoxelGrid *out,
                            SCEulong x, SCEulong y, SCEulong z)
{
    SCEubyte buf[28] = {0};
    float kernel[28] = {
        /* slice 0 */
        1.0/8.0, 1.0/4.0, 1.0/8.0,
        1.0/4.0, 1.0/2.0, 1.0/4.0,
        1.0/8.0, 1.0/4.0, 1.0/8.0,

        /* slice 1 */
        1.0/4.0, 1.0/2.0, 1.0/4.0,
        1.0/2.0, 1.0/1.0, 1.0/2.0,
        1.0/4.0, 1.0/2.0, 1.0/4.0,

        /* slice 2 */
        1.0/8.0, 1.0/4.0, 1.0/8.0,
        1.0/4.0, 1.0/2.0, 1.0/4.0,
        1.0/8.0, 1.0/4.0, 1.0/8.0,
    };
    float value = 0.0;
    size_t i, j, k, offset;
    SCEubyte *ptr = NULL;

    offset = 0;
    for (k = z - 1; k < z + 2; k++) {
        for (j = y - 1; j < y + 2; j++) {
            for (i = x - 1; i < x + 2; i++) {
                buf[offset] = *SCE_VGrid_Offset (in, i, j, k);
                offset++;
            }
        }
    }

    for (i = 0; i < 28; i++)
        value += (kernel[i] * buf[i]) / 256.0;

    value /= 8.0;

    ptr = SCE_VGrid_Offset (out, (x - 1) / 2, (y - 1) / 2, (z - 1) / 2);
    *ptr = value * 256.0;
}


static void
SCE_VWorld_ComputeLOD (SCE_SVoxelGrid *in, SCE_SVoxelGrid *out)
{
    SCEulong x, y, z;

    for (z = 1; z < in->d - 1; z += 2) {
        for (y = 1; y < in->h - 1; y += 2) {
            for (x = 1; x < in->w - 1; x += 2)
                SCE_VWorld_ComputeLODPoint (in, out, x, y, z);
        }
    }
}

int SCE_VWorld_GenerateLOD (SCE_SVoxelWorld *vw, SCEuint level,
                            const SCE_SLongRect3 *zone,
                            SCE_SLongRect3 *updated)
{
    size_t i;
    SCE_SLongRect3 src, dst;
    long p1[3], p2[3];

    src = dst = *zone;

    /* compute source (high lod) and destination (low lod) regions */
    SCE_Rectangle3_GetPointslv (&dst, p1, p2);
    for (i = 0; i < 3; i++) {
        p1[i] = p1[i] / 2;
        p2[i] = (p2[i] + 1) / 2;
    }
    SCE_Rectangle3_Setlv (&dst, p1, p2);
    for (i = 0; i < 3; i++) {
        p1[i] = p1[i] * 2 - 1;
        p2[i] = p2[i] * 2 + 1;
    }
    SCE_Rectangle3_Setlv (&src, p1, p2);

    /* if one of the region exceed the size in vw's buffers,
       split LOD generation */
    /* TODO: doesn't account the number of bytes per voxel */
    if (SCE_Rectangle3_GetAreal (&src) > vw->size1 ||
        SCE_Rectangle3_GetAreal (&dst) > vw->size2) {
        SCE_SLongRect3 up1, up2;
        SCE_Rectangle3_SplitMaxl (zone, &src, &dst);
#if 0
        printf ("split %ld %ld %ld %ld %ld %ld into:\n"
                "%ld %ld %ld - %ld %ld %ld -- and -- %ld %ld %ld - %ld %ld %ld\n",
                zone->p1[0], zone->p1[1], zone->p1[2],
                zone->p2[0], zone->p2[1], zone->p2[2],

                src.p1[0], src.p1[1], src.p1[2],
                src.p2[0], src.p2[1], src.p2[2],

                dst.p1[0], dst.p1[1], dst.p1[2],
                dst.p2[0], dst.p2[1], dst.p2[2]);
#endif
        /* that will lead to many PushZone() while one whould have suffice */
        if (SCE_VWorld_GenerateLOD (vw, level, &src, &up1) < 0) goto fail;
        if (SCE_VWorld_GenerateLOD (vw, level, &dst, &up2) < 0) goto fail;
        if (updated)
            SCE_Rectangle3_Unionl (&up1, &up2, updated);
    } else {
        SCE_SVoxelGrid in, out;

        SCE_VGrid_Init (&in);
        SCE_VGrid_Init (&out);

        SCE_VGrid_SetDimensions (&in, SCE_Rectangle3_GetWidthl (&src),
                                 SCE_Rectangle3_GetHeightl (&src),
                                 SCE_Rectangle3_GetDepthl (&src));
        SCE_VGrid_SetDimensions (&out, SCE_Rectangle3_GetWidthl (&dst),
                                 SCE_Rectangle3_GetHeightl (&dst),
                                 SCE_Rectangle3_GetDepthl (&dst));

        SCE_VGrid_SetNumComponents (&in, 1);
        SCE_VGrid_SetNumComponents (&out, 1);
        /* TODO: initialize with default "non defined" density value, not 0 */
        memset (vw->buffer1, 0, vw->size1);
        memset (vw->buffer2, 0, vw->size2);
        in.data = vw->buffer1;
        out.data = vw->buffer2;

        /* retrieve source voxels */
        if (SCE_VWorld_GetRegion (vw, level, &src, in.data) < 0)
            goto fail;
        SCE_VWorld_ComputeLOD (&in, &out);
        if (SCE_VWorld_Set (vw, level + 1, &dst, out.data) < 0)
            goto fail;

        if (updated)
            *updated = dst;
    }

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

int SCE_VWorld_GenerateAllLOD (SCE_SVoxelWorld *vw, SCEuint level,
                               const SCE_SLongRect3 *zone)
{
    SCEuint i;
    SCE_SLongRect3 area = *zone;

    for (i = level; i < vw->n_lod - 1; i++) {
        if (SCE_VWorld_GenerateLOD (vw, i, &area, &area) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }
    return SCE_OK;
}


int SCE_VWorld_GetNode (SCE_SVoxelWorld *vw, SCEuint level, long x, long y,
                        long z, char *fname)
{
    SCE_SListIterator *it = NULL;
    int r;

    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelOctree *vo = NULL;
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        vo = &wt->vo;
        r = SCE_VOctree_GetNode (vo, level, x, y, z, fname);
        if (r > -1)
            return r;
    }

    return SCE_VOCTREE_NODE_EMPTY;
}

void SCE_VWorld_UpdateCache (SCE_SVoxelWorld *vw)
{
    SCE_SListIterator *it = NULL;
    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        SCE_VOctree_UpdateCache (&wt->vo);
    }
}
int SCE_VWorld_SyncCache (SCE_SVoxelWorld *vw)
{
    SCE_SListIterator *it = NULL;
    SCE_List_ForEach (it, &vw->trees) {
        SCE_SVoxelWorldTree *wt = SCE_List_GetData (it);
        if (SCE_VOctree_SyncCache (&wt->vo) < 0) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }
    return SCE_OK;
}

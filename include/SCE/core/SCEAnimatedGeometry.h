/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2010  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 06/04/2009
   updated: 06/08/2009 */

#ifndef SCEANIMATEDGEOMETRY_H
#define SCEANIMATEDGEOMETRY_H

#include "SCE/core/SCESkeleton.h"
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES 4

typedef struct sce_svertex SCE_SVertex;
struct sce_svertex {
    size_t weight_id;     /**< First weight ID */
    size_t weight_count;  /**< Num attached weights */
};

typedef struct sce_svertexweight SCE_SVertexWeight;
struct sce_svertexweight {
    float weight;          /**< Weight */
    size_t joint_id;       /**< Joint ID */
    size_t next_vertex_id; /**< ID of the first weight of the next vertex*/
};

typedef struct sce_sanimatedgeometry SCE_SAnimatedGeometry;

typedef void (*SCE_FApplySkeletonFunc)(SCE_SAnimatedGeometry*, SCE_SSkeleton*);

struct sce_sanimatedgeometry {
    SCE_SGeometry *geom;
    SCE_SSkeleton *baseskel;    /* bind pose skeleton */
    SCE_SSkeleton *animskel;    /* current interpolated frame */
    int canfree_baseskel, canfree_animskel, canfree_indices;

    SCE_SVertex *vertices;
    SCE_SVertexWeight *weights;
    SCEindices *indices;
    unsigned int n_vertices;
    unsigned int n_weights;
    unsigned int n_indices;

    SCEvertices *base[SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES];
    int local[SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES];
    SCEvertices *output[SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES];
    SCE_SGeometryArray *arrays[SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES];

    SCE_FApplySkeletonFunc applyskel;
};

int SCE_Init_AnimGeom (void);
void SCE_Quit_AnimGeom (void);

int SCE_AnimGeom_GetResourceType (void);

SCE_SAnimatedGeometry* SCE_AnimGeom_Create (void);
void SCE_AnimGeom_Delete (SCE_SAnimatedGeometry*);

SCE_SGeometry* SCE_AnimGeom_GetGeometry (SCE_SAnimatedGeometry*);

void SCE_AnimGeom_SetBaseSkeleton (SCE_SAnimatedGeometry*, SCE_SSkeleton*, int);
SCE_SSkeleton* SCE_AnimGeom_GetBaseSkeleton (SCE_SAnimatedGeometry*);
void SCE_AnimGeom_SetAnimSkeleton (SCE_SAnimatedGeometry*, SCE_SSkeleton*, int);
SCE_SSkeleton* SCE_AnimGeom_GetAnimSkeleton (SCE_SAnimatedGeometry*);
int SCE_AnimGeom_AllocateAnimSkeleton (SCE_SAnimatedGeometry*);

int SCE_AnimGeom_AllocateVertices (SCE_SAnimatedGeometry*, size_t);
int SCE_AnimGeom_AllocateWeights (SCE_SAnimatedGeometry*, size_t);
int SCE_AnimGeom_AllocateBaseVertices (SCE_SAnimatedGeometry*,
                                       SCE_EVertexAttribute, int);

SCE_SVertex* SCE_AnimGeom_GetVertices (SCE_SAnimatedGeometry*);
SCE_SVertexWeight* SCE_AnimGeom_GetWeights (SCE_SAnimatedGeometry*);
SCEvertices* SCE_AnimGeom_GetBaseVertices (SCE_SAnimatedGeometry*, int);

void SCE_AnimGeom_SetIndices (SCE_SAnimatedGeometry*, size_t, SCEindices*, int);

void SCE_AnimGeom_ApplySkeleton (SCE_SAnimatedGeometry*, SCE_SSkeleton*);
void SCE_AnimGeom_ApplyBaseSkeleton (SCE_SAnimatedGeometry*);
void SCE_AnimGeom_ApplyAnimSkeleton (SCE_SAnimatedGeometry*);

/*void SCE_AnimGeom_SetLocal (SCE_SAnimatedGeometry*, SCE_SSkeleton*);*/
int SCE_AnimGeom_SetGlobal (SCE_SAnimatedGeometry*);

int SCE_AnimGeom_BuildGeometry (SCE_SAnimatedGeometry*);

SCE_SAnimatedGeometry* SCE_AnimGeom_Load (const char*, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

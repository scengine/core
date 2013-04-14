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

/* created: 19/04/2012
   updated: 11/04/2013 */

#ifndef SCEFORESTTREE_H
#define SCEFORESTTREE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_MAX_FTREE_DEGREE 4  /* completely arbitrary number */
#define SCE_MAX_FTREE_BUSH_TYPES 8 /* ... also arbitrary */

typedef struct sce_sforesttreenode SCE_SForestTreeNode;
struct sce_sforesttreenode {
    SCE_SForestTreeNode *parent;

    SCE_TMatrix4x3 matrix;
    SCE_TVector3 plane;
    float radius;
    float distance;
    SCEuint n_polygons;   /* tessellation of the node (number of vertices) */

    int leaf_index;   /**< Whether this node should generate a bush */
    SCE_TMatrix4x3 leaf_matrix; /**< Bush's matrix */

    SCEuint index;        /* index of this node (which is a vertex) */
    SCE_SForestTreeNode *children[SCE_MAX_FTREE_DEGREE];
    size_t n_children;
    size_t n_nodes;
    size_t n_branches;
    size_t n_leaves[SCE_MAX_FTREE_BUSH_TYPES];

    /* tree geom */
    size_t n_vertices1;
    size_t n_indices1;
    /* final geom */
    size_t n_vertices2;
    size_t n_indices2;
    SCE_SListIterator it, it2;
};

typedef struct sce_sforesttree SCE_SForestTree;
struct sce_sforesttree {
    SCE_SForestTreeNode root;

    SCE_SGeometry tree_geom;
    SCEvertices *matrix_data;
    SCEvertices *drad_data;          /* distance and radius */
    SCEuint *npoly_data;
    SCEindices *indices_data;
    SCE_SGeometryArray ar1, ar2, ar3, ar4, ar5, ar6, ind;

    size_t *vindex;             /* temporary buffer to store location
                                   of generated final vertices */

    SCE_SGeometry final_geom;
    SCEvertices *vertices;
    SCEindices *indices;
    SCE_SGeometryArray pos, nor, tc, idx;

    /* TODO: matrix type */
    float *bushes[SCE_MAX_FTREE_BUSH_TYPES];

    SCEuint index_counter;      /* Internal use for vertex arrays generation */
    SCEuint vertex_counter;     /* Internal use for vertex arrays generation */

    /*SCE_SGeometryArrayUser u1, u2, u3, u4;*/
};

/* generation parameters */
typedef struct sce_sforesttreeparameters SCE_SForestTreeParameters;
struct sce_sforesttreeparameters {
    float grow_dist;            /* growing distance factor */
    float kill_dist;            /* points p as |p - closest_node| < kill_dist
                                   will be removed */
    float radius;               /* radius of influence */
    long max_nodes;             /* maximum number of nodes to generate */
};

void SCE_FTree_InitNode (SCE_SForestTreeNode*);
void SCE_FTree_ClearNode (SCE_SForestTreeNode*);
SCE_SForestTreeNode* SCE_FTree_CreateNode (void);
void SCE_FTree_DeleteNode (SCE_SForestTreeNode*);

void SCE_FTree_Init (SCE_SForestTree*);
void SCE_FTree_Clear (SCE_SForestTree*);
SCE_SForestTree* SCE_FTree_Create (void);
void SCE_FTree_Delete (SCE_SForestTree*);

void SCE_FTree_CopyNode (SCE_SForestTreeNode*, const SCE_SForestTreeNode*);

SCE_SForestTreeNode* SCE_FTree_GetRootNode (SCE_SForestTree*);
void SCE_FTree_AddNode (SCE_SForestTreeNode*, SCE_SForestTreeNode*);
void SCE_FTree_RemoveNode (SCE_SForestTreeNode*);

void SCE_FTree_SetNodeRadius (SCE_SForestTreeNode*, float);
float SCE_FTree_GetNodeRadius (const SCE_SForestTreeNode*);
float SCE_FTree_GetNodeDistance (const SCE_SForestTreeNode*);
SCEuint SCE_FTree_GetNodeNumVertices (const SCE_SForestTreeNode*);
void SCE_FTree_SetNodeNumVertices (SCE_SForestTreeNode*, SCEuint);

void SCE_FTree_SetNodeMatrix (SCE_SForestTreeNode*, const SCE_TMatrix4x3);
float* SCE_FTree_GetNodeMatrix (SCE_SForestTreeNode*);
void SCE_FTree_GetNodeMatrixv (SCE_SForestTreeNode*, SCE_TMatrix4x3);
size_t SCE_FTree_GetNumNodeChildren (const SCE_SForestTreeNode*);

void SCE_FTree_SetBush (SCE_SForestTreeNode*, int);
int SCE_FTree_GetBush (const SCE_SForestTreeNode*);
void SCE_FTree_SetBushMatrix (SCE_SForestTreeNode*, const SCE_TMatrix4x3);

void SCE_FTree_CountNodes (SCE_SForestTree*);
int SCE_FTree_Build (SCE_SForestTree*);

void SCE_FTree_UpdateTreeGeometry (SCE_SForestTree*);
void SCE_FTree_UpdateFinalGeometry (SCE_SForestTree*);
void SCE_FTree_UpdateBushesMatrices (SCE_SForestTree*);

SCE_SGeometry* SCE_FTree_GetTreeGeometry (SCE_SForestTree*);
SCE_SGeometry* SCE_FTree_GetFinalGeometry (SCE_SForestTree*);
size_t SCE_FTree_GetBushNumInstances (SCE_SForestTree*, SCEuint);
float* SCE_FTree_GetBushMatrix (SCE_SForestTree*, SCEuint, SCEuint);

size_t SCE_FTree_GetNodeNumNodes (const SCE_SForestTreeNode*);
size_t SCE_FTree_GetNodeNumBranches (const SCE_SForestTreeNode*);

size_t SCE_FTree_GetNumNodes (const SCE_SForestTree*);
size_t SCE_FTree_GetNumBranches (const SCE_SForestTree*);

int SCE_FTree_SpaceColonization (SCE_SForestTree*,
                                 const SCE_SForestTreeParameters*,
                                 const SCE_TVector3, SCE_TVector3*, size_t);

void SCE_FTree_ComputeRadius (SCE_SForestTree*, float);
void SCE_FTree_ReduceVertexCount (SCE_SForestTree*);

size_t SCE_FTree_GetSerializedSize (const SCE_SForestTree*);

void SCE_FTree_Serialize (const SCE_SForestTree*, SCE_SFile*);
int SCE_FTree_Deserialize (SCE_SForestTree*, SCE_SFile*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

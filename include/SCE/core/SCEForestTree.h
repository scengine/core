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

/* created: 19/04/2012
   updated: 20/04/2012 */

#ifndef SCEFORESTTREE_H
#define SCEFORESTTREE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_MAX_FTREE_DEGREE 4  /* completely arbitrary number */

typedef struct sce_sforesttreenode SCE_SForestTreeNode;
struct sce_sforesttreenode {
    SCE_SForestTreeNode *parent;

    SCE_TMatrix4x3 matrix;
    SCE_TVector3 plane;
    float radius;
    float distance;
    SCEuint index;        /* index of this node (which is a vertex) */
    SCE_SForestTreeNode *children[SCE_MAX_FTREE_DEGREE];
    size_t n_children;
    size_t n_vertices;
    size_t n_indices;           /* number of required indices */
    size_t n_nodes;             /* number of subnodes */
    SCE_SListIterator it;
};

typedef struct sce_sforesttree SCE_SForestTree;
struct sce_sforesttree {
    SCE_SForestTreeNode root;
    SCE_SGeometry geom;

    SCEvertices *matrix_data;
    SCEvertices *drad_data;          /* distance and radius */
    SCEindices *indices_data;

    SCEuint index_counter;      /* Internal use for vertex arrays generation */
    SCEuint vertex_counter;     /* Internal use for vertex arrays generation */

    SCE_SGeometryArray ar1, ar2, ar3, ar4, ar5, ar6, ind;
    /*SCE_SGeometryArrayUser u1, u2, u3, u4;*/
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

void SCE_FTree_SetNodeMatrix (SCE_SForestTreeNode*, const SCE_TMatrix4x3);
float* SCE_FTree_GetNodeMatrix (SCE_SForestTreeNode*);
void SCE_FTree_GetNodeMatrixv (SCE_SForestTreeNode*, SCE_TMatrix4x3);
size_t SCE_FTree_GetNumNodeChildren (const SCE_SForestTreeNode*);

int SCE_FTree_Build (SCE_SForestTree*);
void SCE_FTree_Update (SCE_SForestTree*);

SCE_SGeometry* SCE_FTree_GetGeometry (SCE_SForestTree*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

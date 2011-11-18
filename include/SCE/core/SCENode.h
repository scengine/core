/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2011  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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
 
/* created: 10/07/2007
   updated: 18/11/2011 */

#ifndef SCENODE_H
#define SCENODE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEOctree.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup node
 * @{
 */

typedef enum {
    /** \brief Those nodes have SCE_SNode::matrix pointing to a float that is
     * their matrix */
    SCE_SINGLE_MATRIX_NODE,
    /** \brief Those nodes are designed for being used into a tree structure.
     * basically, you may want to use this type if your node is attached to a
     * parent node. */
    SCE_TREE_NODE,
    /** \brief Let the SCE_Node_AddNode() decides */
    SCE_AUTO_NODE_TYPE
} SCE_ENodeType;

/**
 * \brief Default matrices array for the node groups
 */
typedef enum {
    SCE_NODE_READ_MATRIX = 0,
    SCE_NODE_WRITE_MATRIX,
    /*    SCE_NODE_STANDBY_MATRIX */     /* unused, but available for reading */
} SCE_ENodeMatrixArray;

/** \copydoc sce_snode */
typedef struct sce_snode SCE_SNode;
typedef float* (*SCE_FGetNodeMatrix)(SCE_SNode*, size_t);
typedef void (*SCE_FNodeCallback)(SCE_SNode*, void*);
typedef void (*SCE_FNodeTransformCallback)(const SCE_SNode*, SCE_SNode*);
typedef void (*SCE_FNodeUpdate)(SCE_SNode*);

typedef struct sce_snodegroup SCE_SNodeGroup;

/**
 * \brief Node definition structure
 */
struct sce_snode {
    SCE_SOctreeElement element; /**< Element */
    SCE_SNode *parent;          /**< Parent node */
    /* yes: even a single matrix node can has children! */
    SCE_SList child;            /**< Children */
    SCE_SList toupdate;         /**< Children to update! */
    SCE_ENodeType type;         /**< Does the node have a final matrix in
                                 * \c matrix ? */
    SCE_FNodeUpdate update;     /**< Update function */
    float *matrix;              /**< Pointer to the matrix(ces) of the node */
    SCE_SNodeGroup *group;      /**< Group of the node */
    SCE_SListIterator it;       /**< Own iterator */
    unsigned int marks;         /**< Has the node moved since the last update?*/
    SCE_FNodeCallback moved;
    void *movedparam;
    SCE_FNodeTransformCallback transform;
    void *udata;                /**< User-defined data */
};

struct sce_snodegroup {
    size_t *ids;
    size_t n_ids;
};

/** @} */

SCE_SNode* SCE_Node_Create (void);
void SCE_Node_Delete (SCE_SNode*);
void SCE_Node_DeleteRecursive (SCE_SNode*);

SCE_SNodeGroup* SCE_Node_CreateGroup (size_t);
void SCE_Node_DeleteGroup (SCE_SNodeGroup*);

void SCE_Node_Switch (SCE_SNodeGroup*, size_t, size_t);
int SCE_Node_AddNode (SCE_SNodeGroup*, SCE_SNode*, SCE_ENodeType);
int SCE_Node_AddNodeRecursive (SCE_SNodeGroup*, SCE_SNode*, SCE_ENodeType);
void SCE_Node_RemoveNode (SCE_SNode*);

SCE_ENodeType SCE_Node_GetType (SCE_SNode*);

void SCE_Node_Attach (SCE_SNode*, SCE_SNode*);
void SCE_Node_Detach (SCE_SNode*);

void SCE_Node_Insert (SCE_SNode*, SCE_SNode*);

/* gl calls in this module are very ugly */
void SCE_Node_MultMatrix (SCE_SNode*) SCE_GNUC_DEPRECATED;
void SCE_Node_LoadMatrix (SCE_SNode*) SCE_GNUC_DEPRECATED;

float* SCE_Node_GetMatrix (SCE_SNode*, SCE_ENodeMatrixArray);
float* SCE_Node_GetFinalMatrix (SCE_SNode*);
void SCE_Node_SetMatrix (SCE_SNode*, SCE_TMatrix4);
void SCE_Node_GetMatrixv (SCE_SNode*, SCE_TMatrix4);
void SCE_Node_GetFinalMatrixv (SCE_SNode*, SCE_TMatrix4);
void SCE_Node_CopyMatrix (SCE_SNode*, SCE_SNode*);

void SCE_Node_SetOnMovedCallback (SCE_SNode*, SCE_FNodeCallback, void*);
void SCE_Node_SetTransformCallback (SCE_SNode*, SCE_FNodeTransformCallback);
void SCE_Node_TransformTranslation (SCE_SNode*);
void SCE_Node_TransformTranslationNormalize (SCE_SNode*);

void SCE_Node_HasMoved (SCE_SNode*);
void SCE_Node_HasNotMoved (SCE_SNode*);
int SCE_Node_IsMoved (SCE_SNode*);

void SCE_Node_Force (SCE_SNode*);
void SCE_Node_DontForce (SCE_SNode*);
int SCE_Node_IsForced (SCE_SNode*);

void SCE_Node_Update (SCE_SNode*);
void SCE_Node_UpdateRecursive (SCE_SNode*);
void SCE_Node_UpdateRootRecursive (SCE_SNode*);
#if 0
void SCE_Node_FastUpdateRecursive (SCE_SNode*, unsigned int);
void SCE_Node_FastUpdateRootRecursive (SCE_SNode*, unsigned int);
#endif

int SCE_Node_HasParent (SCE_SNode*);
SCE_SNode* SCE_Node_GetParent (SCE_SNode*);

SCE_SOctreeElement* SCE_Node_GetElement (SCE_SNode*);

SCE_SList* SCE_Node_GetChildrenList (SCE_SNode*);

void SCE_Node_SetData (SCE_SNode*, void*);
#if 1
void* SCE_Node_GetData (SCE_SNode*);
#else
#define SCE_Node_GetData(n) (((SCE_SNode*)(n))->udata)
#endif

void SCE_Node_Use (SCE_SNode*) SCE_GNUC_DEPRECATED;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

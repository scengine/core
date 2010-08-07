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
    GNU General Public License for more details

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/
 
/* created: 10/07/2007
   updated: 28/02/2010 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCENode.h"

/**
 * \file SCENode.c
 * \copydoc node
 * \file SCENode.h
 * \copydoc node
 */

/**
 * \defgroup node Nodes managment
 * \ingroup interface
 * \brief Nodes and node trees managment
 */

/** @{ */

#define SCE_NODE_HAS_MOVED (1u)
#define SCE_NODE_FORCE (SCE_NODE_HAS_MOVED << 1)

/* used to kick "if" instructions. it's more probably that the nodes have
   user-defined callbacks, so an useless "if" can be kicked */
/* TODO: isn't that a stupid thing? */
static void SCE_Node_NullFunc (SCE_SNode *n)
{
}

static void SCE_Node_UpdateTree (SCE_SNode *node)
{
    SCE_Matrix4_Mul (SCE_Node_GetFinalMatrix (node->parent),
                     SCE_Node_GetMatrix (node, SCE_NODE_READ_MATRIX),
                     SCE_Node_GetFinalMatrix (node));
}
static void SCE_Node_UpdateTreeFun (SCE_SNode *node)
{
    SCE_Matrix4_Mul (SCE_Node_GetFinalMatrix (node->parent),
                     SCE_Node_GetMatrix (node, SCE_NODE_READ_MATRIX),
                     SCE_Node_GetFinalMatrix (node));
    if (node->marks & SCE_NODE_HAS_MOVED)
        node->moved (node, node->movedparam);
}
static void SCE_Node_UpdateFun (SCE_SNode *node)
{
    if (node->marks & SCE_NODE_HAS_MOVED)
        node->moved (node, node->movedparam);
}

static void SCE_Node_SetUpdateFunc (SCE_SNode *n)
{
    if (n->type == SCE_TREE_NODE) {
        if (n->moved == NULL) n->update = SCE_Node_UpdateTree;
        else n->update = SCE_Node_UpdateTreeFun;
    } else {
        if (n->moved == NULL) n->update = SCE_Node_NullFunc;
        else n->update = SCE_Node_UpdateFun;
    }
}

static void SCE_Node_YouDontHaveParent (void *n)
{
    SCE_SNode *node = n;
    node->parent = NULL;
}
static void SCE_Node_Init (SCE_SNode *node)
{
    node->element = NULL;
    node->parent = NULL;
    SCE_List_Init (&node->child);
    SCE_List_SetFreeFunc (&node->child, SCE_Node_YouDontHaveParent);
    SCE_List_Init (&node->toupdate);
    SCE_List_SetFreeFunc (&node->toupdate, SCE_Node_YouDontHaveParent);
    node->update = SCE_Node_NullFunc;
    node->matrix = NULL;
    node->group = NULL;
    SCE_List_InitIt (&node->it);
    SCE_List_SetData (&node->it, node);
    node->marks = 0;
    node->moved = NULL;
    node->movedparam = NULL;
    node->udata = NULL;
}

static size_t default_ids[2] = {0, 1};
static SCE_SNodeGroup default_group = {default_ids, 2};
/**
 * \brief Creates a new node
 * \returns a newly allocated SCE_SNode on success or NULL on error
 */
SCE_SNode* SCE_Node_Create (void)
{
    SCE_SNode *node = NULL;
    if (!(node = SCE_malloc (sizeof *node)))
        goto fail;
    SCE_Node_Init (node);
    if (!(node->element = SCE_Octree_CreateElement ()))
        goto fail;
    if (SCE_Node_AddNode (&default_group, node, SCE_SINGLE_MATRIX_NODE) < 0)
        goto fail;
    /* by default, the data of the element is the node */
    SCE_List_SetData (&node->element->it, node);
    return node;
fail:
    SCE_Node_Delete (node);
    SCEE_LogSrc ();
    return NULL;
}

/**
 * \brief Deletes a node
 * \param node a node
 * \note This function doesn't do anything with the node's data.
 */
void SCE_Node_Delete (SCE_SNode *node)
{
    if (node) {
        SCE_Node_Detach (node);
        SCE_Octree_DeleteElement (node->element);
        SCE_List_Clear (&node->child);
        SCE_List_Clear (&node->toupdate);
        SCE_Node_RemoveNode (node);
        SCE_free (node);
    }
}
/**
 * \brief Deletes a node and all its children
 * \param node a node
 * 
 * This function deletes a node recursivly.
 * \sa SCE_Node_Delete()
 */
void SCE_Node_DeleteRecursive (SCE_SNode *node)
{
    if (node) {
        SCE_SListIterator *i = NULL, *p = NULL;
        SCE_List_AppendAll (&node->child, &node->toupdate);
        SCE_List_ForEachProtected (p, i, &node->child)
            SCE_Node_DeleteRecursive (SCE_List_GetData (i));
        SCE_Node_Delete (node);
    }
}


static void SCE_Node_InitGroup (SCE_SNodeGroup *ngroup)
{
    ngroup->ids = NULL;
    ngroup->n_ids = 0;
}
/**
 * \brief Creates a node group
 * \param n group size
 */
SCE_SNodeGroup* SCE_Node_CreateGroup (size_t n)
{
    SCE_SNodeGroup *ngroup = SCE_malloc (sizeof *ngroup);
    if (!ngroup)
        SCEE_LogSrc ();
    else {
        size_t i;
        SCE_Node_InitGroup (ngroup);
        if (!(ngroup->ids = SCE_malloc (n * sizeof *ngroup->ids))) {
            SCEE_LogSrc ();
            SCE_free (ngroup), ngroup = NULL;
        }
        for (i = 0; i < n; i++)
            ngroup->ids[i] = i;
        ngroup->n_ids = n;
    }
    return ngroup;
}
/**
 * \brief Deletes a node group
 * \param ngroup the node group to delete
 */
void SCE_Node_DeleteGroup (SCE_SNodeGroup *ngroup)
{
    if (ngroup) {
        SCE_free (ngroup->ids);
        SCE_free (ngroup);
    }
}


/**
 * \brief Swaps two identifiers (usually SCE_NODE_READ_MATRIX and
 * SCE_NODE_WRITE_MATRIX)
 * \param ngroup a node group
 * \param id1 the first ID to swap with \p id2
 * \param id2 the second ID to swap with \p id1
 */
void SCE_Node_Switch (SCE_SNodeGroup *ngroup, size_t id1, size_t id2)
{
    size_t tmp = ngroup->ids[id1];
    ngroup->ids[id1] = ngroup->ids[id2];
    ngroup->ids[id2] = tmp;
}
/**
 * \brief 
 */
int SCE_Node_AddNode (SCE_SNodeGroup *ngroup, SCE_SNode *node, SCE_ENodeType t)
{
    size_t i;
    size_t size = ngroup->n_ids * sizeof (SCE_TMatrix4);
    if (t == SCE_TREE_NODE)
        size += sizeof (SCE_TMatrix4);
    SCE_free (node->matrix);
    if (!(node->matrix = SCE_malloc (size))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    /* TODO: fixed value 16 sux */
    for (i = 0; i < ngroup->n_ids; i++)
        SCE_Matrix4_Identity (&node->matrix[i * 16]);
    node->type = t;
    node->group = ngroup;
    SCE_Node_SetUpdateFunc (node);
    return SCE_OK;
}
/**
 * \brief 
 */
void SCE_Node_RemoveNode (SCE_SNode *node)
{
    SCE_free (node->matrix), node->matrix = NULL;
    node->group = NULL;
}

/**
 * \brief Returns the type of a node
 * \param node a node
 */
SCE_ENodeType SCE_Node_GetType (SCE_SNode *node)
{
    return node->type;
}

/**
 * \brief Attaches a node to another
 * \param node a node
 * \param child a node to make child of \p node
 * 
 * This function makes the \p child node child of the \p node node. If the \p
 * child node has already a parent node, it will be detached before from its
 * current parent.
 */
void SCE_Node_Attach (SCE_SNode *node, SCE_SNode *child)
{
    SCE_Node_Detach (child);
    child->parent = node;
    SCE_List_Appendl (&node->child, &child->it);
    SCE_Node_HasMoved (child);
}

/**
 * \brief Detaches a node from its parent
 * \param node the node to detach
 * \todo allow to keep the position of \p node ? m = fm
 */
void SCE_Node_Detach (SCE_SNode *node)
{
    if (node->parent) {
        SCE_List_Removel (&node->it);
        node->parent = NULL;
        /* yo dawg, if this node was a child then it will be reattached
           as a child and then updated, otherwise it is now a root
           and it does not need any update */
        /*SCE_Node_HasMoved (node);*/
    }
}

/**
 * \brief Inserts a node between another and its parent
 * \param node the node to insert
 * \param child future child of \p node
 *
 * Inserts \p node between \p child and its parent. If \p child has no parent,
 * this function just attach \p child to \p node.
 * \sa SCE_Node_Attach()
 * \note This function is just a bit useless.
 */
void SCE_Node_Insert (SCE_SNode *node, SCE_SNode *child)
{
    if (child->parent)
        SCE_Node_Attach (child->parent, node);
    SCE_Node_Attach (node, child);
}


#if 0
/**
 * \deprecated
 * \brief Calls SCE_CMultMatrix() with the final matrix of the given node
 * \param node a node
 */
void SCE_Node_MultMatrix (SCE_SNode *node)
{
    SCE_CMultMatrix (SCE_Node_GetFinalMatrix (node));
}
/**
 * \deprecated
 * \brief Calls SCE_CLoadMatrix() with the final matrix of the given node
 * \param node a node
 */
void SCE_Node_LoadMatrix (SCE_SNode *node)
{
    SCE_CLoadMatrix (SCE_Node_GetFinalMatrix (node));
}
#endif


float* SCE_Node_GetMatrix (SCE_SNode *node, size_t id)
{
    size_t offset = node->group->ids[id];
    /* TODO: sizeof float sux a lot! */
    return &node->matrix[offset * (sizeof (SCE_TMatrix4) / sizeof (float))];
}
float* SCE_Node_GetFinalMatrix (SCE_SNode *node)
{
    /* TODO: ugly if */
    if (node->type == SCE_SINGLE_MATRIX_NODE) {
        return SCE_Node_GetMatrix (node, SCE_NODE_READ_MATRIX);
    } else {
        size_t offset = node->group->n_ids;
        /* TODO: sizeof float sux a lot! */
        return &node->matrix[offset * (sizeof (SCE_TMatrix4) / sizeof (float))];
    }
}

/**
 * \brief Copies the given matrix into a node
 *
 * Does not call SCE_Node_HasMoved() so you have to do it by yourself
 * if wou want the SCEngine to update this node into the scene manager.
 */
void SCE_Node_SetMatrix (SCE_SNode *node, SCE_TMatrix4 m)
{
    SCE_Matrix4_Copy (SCE_Node_GetMatrix (node, SCE_NODE_WRITE_MATRIX), m);
}
/**
 * \brief Copies the SCE_NODE_READ_MATRIX of \p node into \p m
 * \param node a node
 * \param m a matrix where copy the node's local matrix
 * \see SCE_Node_GetFinalMatrixv(), SCE_Node_GetMatrix()
 */
void SCE_Node_GetMatrixv (SCE_SNode *node, SCE_TMatrix4 m)
{
    SCE_Matrix4_Copy (m, SCE_Node_GetMatrix (node, SCE_NODE_READ_MATRIX));
}
/**
 * \brief Copies the final node's matrix into \p m
 * \param node a node
 * \param m a matrix where copy the real node's matrix
 * \see SCE_Node_GetFinalMatrix(), SCE_Node_GetMatrixv()
 */
void SCE_Node_GetFinalMatrixv (SCE_SNode *node, SCE_TMatrix4 m)
{
    SCE_Matrix4_Copy (m, SCE_Node_GetFinalMatrix (node));
}
/**
 * \brief Copies the read matrix of \p b into the write matrix of \p a
 */
void SCE_Node_CopyMatrix (SCE_SNode *a, SCE_SNode *b)
{
    SCE_Node_GetMatrixv (b, SCE_Node_GetMatrix (a, SCE_NODE_WRITE_MATRIX));
}


void SCE_Node_SetOnMovedCallback (SCE_SNode *node, SCE_FNodeCallback f, void *p)
{
    node->moved = f;
    node->movedparam = p;
    SCE_Node_SetUpdateFunc (node);
}


static void SCE_Node_ToUpdate (SCE_SNode *node)
{
    SCE_List_Removel (&node->it);
    SCE_List_Appendl (&node->parent->toupdate, &node->it);
}
static void SCE_Node_ToUpdateRec (SCE_SNode *node)
{
    if (node->parent) {
        SCE_Node_ToUpdate (node);
        SCE_Node_ToUpdateRec (node->parent);
    }
}
/**
 * \brief Marks the node \p node and its children
 * as a moved node since the last update
 */
void SCE_Node_HasMoved (SCE_SNode *node)
{
    SCE_FLAG_ADD (node->marks, SCE_NODE_HAS_MOVED);
    SCE_Node_ToUpdate (node);
}
static void SCE_Node_NotToUpdate (SCE_SNode *node)
{
    SCE_List_Removel (&node->it);
    SCE_List_Appendl (&node->parent->child, &node->it);
}
static void SCE_Node_NotToUpdateRec (SCE_SNode *node)
{
    if (node->parent) {
        SCE_Node_NotToUpdate (node);
        SCE_Node_NotToUpdateRec (node->parent);
    }
}
/**
 * \brief Defines the node \p node as an unmoved node since the last update
 */
void SCE_Node_HasNotMoved (SCE_SNode *node)
{
    SCE_FLAG_REMOVE (node->marks, SCE_NODE_HAS_MOVED);
    SCE_Node_NotToUpdateRec (node);
}
/**
 * \brief Indicates if the given node has moved since the last update
 * \todo currently not used
 */
int SCE_Node_IsMoved (SCE_SNode *node)
{
    return SCE_FLAG_TEST (node->marks, SCE_NODE_HAS_MOVED);
}

/**
 * \brief Forces the updating of \p node and its children
 */
void SCE_Node_Force (SCE_SNode *node)
{
    SCE_SListIterator *it;
    SCE_FLAG_ADD (node->marks, SCE_NODE_FORCE);
    SCE_List_ForEach (it, &node->child)
        SCE_Node_Force (SCE_List_GetData (it));
}
/**
 * \brief Disables the update forcing of \p node
 */
void SCE_Node_DontForce (SCE_SNode *node)
{
    SCE_FLAG_REMOVE (node->marks, SCE_NODE_FORCE);
}
/**
 * \brief Did someone force the updating of \p node?
 */
int SCE_Node_IsForced (SCE_SNode *node)
{
    return SCE_FLAG_TEST (node->marks, SCE_NODE_FORCE);
}


static void SCE_Node_UpdateRecForce (SCE_SNode *node)
{
    SCE_SListIterator *i = NULL;
    node->update (node);
    SCE_List_AppendAll (&node->child, &node->toupdate);
    SCE_List_ForEach (i, &node->child)
        SCE_Node_UpdateRecForce (SCE_List_GetData (i));
    node->marks = 0;
}
void SCE_Node_UpdateRecursive (SCE_SNode *node)
{
    SCE_SListIterator *i = NULL;
    if (node->marks) {
        node->update (node);
        SCE_List_ForEach (i, &node->toupdate)
            SCE_Node_UpdateRecForce (SCE_List_GetData (i));
        SCE_List_ForEach (i, &node->child)
            SCE_Node_UpdateRecForce (SCE_List_GetData (i));
        node->marks = 0;
    } else {
        SCE_List_ForEach (i, &node->toupdate)
            SCE_Node_UpdateRecursive (SCE_List_GetData (i));
    }
    SCE_List_AppendAll (&node->child, &node->toupdate);
}
void SCE_Node_UpdateRootRecursive (SCE_SNode *node)
{
    SCE_SListIterator *i = NULL;
    SCE_List_ForEach (i, &node->toupdate)
        SCE_Node_UpdateRecursive (SCE_List_GetData (i));
    SCE_List_AppendAll (&node->child, &node->toupdate);
}

/* not very able to determine how many nodes needs to be updated */
#if 0
static void SCE_Node_FastRec (void *node)
{
    SCE_SListIterator *i = NULL;
    SCE_Node_Update (node);
    SCE_List_ForEach (i, &((SCE_SNode*)node)->child)
        SCE_Node_UpdateRecursive (SCE_List_GetData (i));
}
static void SCE_Node_FastRecTree (void *node)
{
    SCE_SListIterator *i = NULL;
    SCE_Node_UpdateTree (node);
    SCE_List_ForEach (i, &((SCE_SNode*)node)->child)
        SCE_Node_UpdateRecursive (SCE_List_GetData (i));
}
/**
 * \brief Updates a node and its children
 * \param node the node to update recursivly
 * \see SCE_Node_UpdateRecursive(), SCE_Node_FastUpdateRootRecursive()
 */
void SCE_Node_FastUpdateRecursive (SCE_SNode *node, unsigned int n)
{
    if (SCE_Node_GetType (node) == SCE_TREE_NODE) {
        SCE_Node_Update (node);
        SCE_List_FastForEach4 (&node->child, n, SCE_Node_FastRecTree);
    } else {
        SCE_List_FastForEach4 (&node->child, n, SCE_Node_FastRec);
    }
}
/**
 * \brief Updates a node and all its child
 * \param node the node to update recursivly
 * \see SCE_Node_FastUpdateRecursive(), SCE_Node_UpdateRootRecursive()
 */
void SCE_Node_FastUpdateRootRecursive (SCE_SNode *node, unsigned int n)
{
    if (SCE_Node_GetType (node) == SCE_TREE_NODE)
        SCE_Node_UpdateRoot (node);
    SCE_List_FastForEach4 (&node->child, n, SCE_Node_FastRec);
}
#endif

/**
 * \brief Checks if a node has a parent
 * \param node a node
 * \returns SCE_TRUE if the \p node node have a parent, SCE_FALSE otherwise.
 */
int SCE_Node_HasParent (SCE_SNode *node)
{
    return (node->parent ? SCE_TRUE : SCE_FALSE);
}

/**
 * \brief Gets the parent of a node
 * \param node a node
 * \returns the parent node of \p node
 */
SCE_SNode* SCE_Node_GetParent (SCE_SNode *node)
{
    return node->parent;
}

/**
 * \brief Returns the element of a node
 * \param node a node
 * \sa SCE_SNode::element
 */
SCE_SOctreeElement* SCE_Node_GetElement (SCE_SNode *node)
{
    return node->element;
}

/**
 * \brief Gets the list of the children of a node
 * \param node a node
 */
SCE_SList* SCE_Node_GetChildrenList (SCE_SNode *node)
{
    return &node->child;
}

/**
 * \brief Sets data of a node
 * \param node a node
 * \param data the data to set to \p node
 */
void SCE_Node_SetData (SCE_SNode *node, void *data)
{
    node->udata = data;
    SCE_List_SetData (&node->element->it, data);
}

#if 1
/**
 * \brief Gets data of a node
 * \param node a node
 * \returns the data of the \p node node
 */
void* SCE_Node_GetData (SCE_SNode *node)
{
    return node->udata;
}
#endif


#if 0
/**
 * \deprecated
 * \brief Calls SCE_CLoadIdentityMatrix() if \p node is null,
 * \param node a node
 *  SCE_Node_MultMatrix() otherwise
 */
void SCE_Node_Use (SCE_SNode *node)
{
    if (node)
        SCE_Node_MultMatrix (node);
    else
        SCE_CLoadIdentityMatrix ();
}
#endif

/** @} */

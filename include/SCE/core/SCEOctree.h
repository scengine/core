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
 
/* created: 06/05/2008
   updated: 20/09/2010 */

#ifndef SCEOCTREE_H
#define SCEOCTREE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBoundingBox.h"
#include "SCE/core/SCEBoundingSphere.h"
#include "SCE/core/SCEFrustum.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup octree
 * @{
 */

/** \copydoc sce_soctreeelement */
typedef struct sce_soctreeelement SCE_SOctreeElement;
/** \copydoc sce_soctree */
typedef struct sce_soctree SCE_SOctree;

/**
 * \brief Representing functions used to add an element to an octree
 */
typedef void (*SCE_FOctreeInsertFunc)(SCE_SOctree *tree,SCE_SOctreeElement *el);

struct sce_soctreeelement {
    SCE_SListIterator it, it2;
    SCE_FOctreeInsertFunc insert;/**< Insert function */
    SCE_SOctree *octree;         /**< Octree */
    SCE_SBoundingSphere *sphere; /**< Sphere used for collision detection */
    void *udata;
};


/**
 * \brief Type for depth-control function of SCE_Octree_RecursiveMake()
 * \param tree the current octree
 * \param param user-defined data
 * \returns SCE_TRUE to stop generation of octree's chlidren or SCE_FALSE to
 *          continue recursive generation
 * 
 * This type is used to control when stop octree's children generation when
 * doing a recursive children generation.
 * \see SCE_Octree_RecursiveMake()
 */
typedef int (*SCE_FOctreeLimitFunc)(SCE_SOctree *tree, void *param);

/**
 * \brief Octree definition structure
 */
struct sce_soctree {
    SCE_SOctree *child[8];  /**< Array of octree's children */
    int visible;            /**< Is octree visible? */
    int partially;          /**< Is octree partially visible? */
    SCE_FOctreeInsertFunc insert; /**< Insert function */
    SCE_SOctree *parent;    /**< Octree's parent */
    SCE_SBoundingBox box;   /**< Octree's bounding box */
    SCE_SList elements;     /**< Elements contained in the octree */
    void *data;             /**< User defined data */
    SCE_SListIterator public_it;
    SCE_SListIterator it;
};

/** @} */

void SCE_Octree_Init (SCE_SOctree*);
void SCE_Octree_Clear (SCE_SOctree*);
SCE_SOctree* SCE_Octree_Create (void);
void SCE_Octree_Delete (SCE_SOctree*);
void SCE_Octree_DeleteRecursive (SCE_SOctree*);

void SCE_Octree_InitElement (SCE_SOctreeElement*);
void SCE_Octree_ClearElement (SCE_SOctreeElement*);
SCE_SOctreeElement* SCE_Octree_CreateElement (void);
void SCE_Octree_DeleteElement (SCE_SOctreeElement*);

void SCE_Octree_SetElementBoundingSphere (SCE_SOctreeElement*,
                                          SCE_SBoundingSphere*);
SCE_SBoundingSphere* SCE_Octree_GetElementBoundingSphere (SCE_SOctreeElement*);
void SCE_Octree_GetElementCenterv (const SCE_SOctreeElement*, SCE_TVector3);
void SCE_Octree_SetElementData (SCE_SOctreeElement*, void*);
void* SCE_Octree_GetElementData (SCE_SOctreeElement*);

void SCE_Octree_SetCenter (SCE_SOctree*, float, float, float);
void SCE_Octree_SetCenterv (SCE_SOctree*, SCE_TVector3);
void SCE_Octree_GetCenterv (SCE_SOctree*, SCE_TVector3);
void SCE_Octree_SetSize (SCE_SOctree*, float, float, float);
void SCE_Octree_SetSizev (SCE_SOctree*, SCE_TVector3);

SCE_SBox* SCE_Octree_GetBox (SCE_SOctree*);
SCE_SBoundingBox* SCE_Octree_GetBoundingBox (SCE_SOctree*);

void SCE_Octree_SetData (SCE_SOctree*, void*);
void* SCE_Octree_GetData (SCE_SOctree*);
SCE_SListIterator* SCE_Octree_GetIterator (SCE_SOctree*);

int SCE_Octree_IsVisible (SCE_SOctree*);
int SCE_Octree_IsPartiallyVisible (SCE_SOctree*);
unsigned int SCE_Octree_GetLevel (SCE_SOctree*);

int SCE_Octree_HasChildren (SCE_SOctree*);
SCE_SOctree** SCE_Octree_GetChildren (SCE_SOctree*);

SCE_SOctree* SCE_Octree_GetParent (SCE_SOctree*);

int SCE_Octree_MakeChildren (SCE_SOctree*, int, float);
int SCE_Octree_RecursiveMake (SCE_SOctree*, unsigned int,
                              SCE_FOctreeLimitFunc, void*, int, float);

void SCE_Octree_DefaultInsertFunc (SCE_SOctree*, SCE_SOctreeElement*);

void SCE_Octree_InsertElement (SCE_SOctree*, SCE_SOctreeElement*);
void SCE_Octree_ReinsertElement (SCE_SOctreeElement*);
void SCE_Octree_RemoveElement (SCE_SOctreeElement*);

void SCE_Octree_MarkVisibles (SCE_SOctree*, SCE_SFrustum*);

/* new API */

void SCE_Octree_FetchNodesBB (SCE_SOctree*, const SCE_SBoundingBox*,
                              SCE_SList*);
void SCE_Octree_FetchNodesRect (SCE_SOctree*, const SCE_SLongRect3*,
                                SCE_SList*);
void SCE_Octree_FetchNodesBS (SCE_SOctree*, const SCE_SBoundingSphere*,
                              SCE_SList*);
void SCE_Octree_FetchNodesFrustum (SCE_SOctree*, const SCE_SFrustum*,
                                   SCE_SList*);

void SCE_Octree_FetchElementsBB (SCE_SOctree*, const SCE_SBoundingBox*,
                                 SCE_SList*);
void SCE_Octree_FetchElementsRect (SCE_SOctree*, const SCE_SLongRect3*,
                                   SCE_SList*);
void SCE_Octree_FetchElementsBS (SCE_SOctree*, const SCE_SBoundingSphere*,
                                 SCE_SList*);
void SCE_Octree_FetchElementsFrustum (SCE_SOctree*, const SCE_SFrustum*,
                                      SCE_SList*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

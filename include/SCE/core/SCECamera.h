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
 
/* created: 21/12/2006
   updated: 24/10/2011 */

#ifndef SCECAMERA_H
#define SCECAMERA_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBoundingSphere.h"
#include "SCE/core/SCECone.h"
#include "SCE/core/SCEFrustum.h"
#include "SCE/core/SCENode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup camera
 * @{
 */

/** \copydoc sce_sviewport */
typedef struct sce_sviewport SCE_SViewport;
/**
 * \brief An OpenGL's viewport that defines and rectangular region where render
 */
struct sce_sviewport {
    int x; /**< X coordinate of the origin of the viewport */
    int y; /**< Y coordinate of the origin of the viewport */
    int w; /**< width of the viewport */
    int h; /**< height of the viewport */
}; 

/** \copydoc sce_scamera */
typedef struct sce_scamera SCE_SCamera;
/**
 * \brief A camera structure
 *
 * A camera structure stores information about the view point to render the
 * scene, the vision's angle, the orientation of the view, etc.
 */
struct sce_scamera {
    SCE_TMatrix4 finalview;    /**< Combined \c view and \c world matrices */
    SCE_TMatrix4 finalviewinv; /**< Inverse of \c finalview */
    SCE_TMatrix4 view;         /**< View matrix */
    SCE_TMatrix4 viewinv;      /**< Inverse of \c view */
    SCE_TMatrix4 proj;         /**< Projection matrix */
    SCE_TMatrix4 projinv;      /**< Inverse of \c proj */
    SCE_TMatrix4 finalviewproj;/**< Final view projection matrix */
    SCE_TMatrix4 finalviewprojinv; /**< Final inverse view projection matrix */
    SCE_SViewport viewport;    /**< Camera's viewport (GL viewport) */
    SCE_SFrustum frustum;      /**< Camera's frustum */
    SCE_SBoundingSphere sphere;/**< Bounding sphere for the octree element */
    SCE_SNode *node;           /**< Node of the camera */
    SCE_SListIterator it;      /**< Own iterator (used by the scene manager) */
};

/** @} */

void SCE_Camera_Init (SCE_SCamera*);

SCE_SCamera* SCE_Camera_Create (void);
void SCE_Camera_Delete (SCE_SCamera*);

void SCE_Camera_SetViewport (SCE_SCamera*, int, int, int, int);
SCE_SViewport* SCE_Camera_GetViewport (SCE_SCamera*);

void SCE_Camera_SetProjection (SCE_SCamera*, float, float, float, float);
void SCE_Camera_SetProjectionFromCone (SCE_SCamera*, const SCE_SCone*, float);

float* SCE_Camera_GetView (SCE_SCamera*);
float* SCE_Camera_GetViewInverse (SCE_SCamera*);
float* SCE_Camera_GetProj (SCE_SCamera*);
float* SCE_Camera_GetProjInverse (SCE_SCamera*);
float* SCE_Camera_GetFinalViewProj (SCE_SCamera*);
float* SCE_Camera_GetFinalViewProjInverse (SCE_SCamera*);

void SCE_Camera_SetPosition (SCE_SCamera*, float, float, float);
void SCE_Camera_SetPositionv (SCE_SCamera*, const SCE_TVector3);
void SCE_Camera_GetPositionv (const SCE_SCamera*, SCE_TVector3);

#if 0
void SCE_Camera_SetDirectionv (SCE_SCamera*, const SCE_TVector3);
#endif

SCE_SNode* SCE_Camera_GetNode (SCE_SCamera*);
SCE_SFrustum* SCE_Camera_GetFrustum (SCE_SCamera*);
float SCE_Camera_GetNear (const SCE_SCamera*);
float SCE_Camera_GetFar (const SCE_SCamera*);

float* SCE_Camera_GetFinalView (SCE_SCamera*);
float* SCE_Camera_GetFinalViewInverse (SCE_SCamera*);

void SCE_Camera_GetBase(SCE_SCamera*, SCE_TVector3, SCE_TVector3, SCE_TVector3);

SCE_SListIterator* SCE_Camera_GetIterator (SCE_SCamera*);

void SCE_Camera_Update (SCE_SCamera*);

float SCE_Camera_Project (SCE_SCamera*, SCE_TVector3);
void SCE_Camera_UnProject (SCE_SCamera*, SCE_TVector3);
void SCE_Camera_Line (SCE_SCamera*, SCE_TVector2, SCE_SLine3*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

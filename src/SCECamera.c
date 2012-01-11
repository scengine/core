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

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCECone.h"
#include "SCE/core/SCECamera.h"

/**
 * \file SCECamera.c
 * \copydoc camera
 * \file SCECamera.h
 * \copydoc camera
 */

/**
 * \defgroup camera Camera
 * \ingroup interface
 * \internal
 * \brief
 * @{
 */

/**
 * \brief Initializes a camera structure
 * \param cam the structure to initialize
 */
void SCE_Camera_Init (SCE_SCamera *cam)
{
    SCE_Matrix4_Identity (cam->finalview);
    SCE_Matrix4_Identity (cam->finalviewinv);
    SCE_Matrix4_Identity (cam->view);
    SCE_Matrix4_Identity (cam->viewinv);
    SCE_Matrix4_Identity (cam->proj);
    SCE_Matrix4_Identity (cam->projinv);
    cam->viewport.x = cam->viewport.y = 0;
    cam->viewport.w = cam->viewport.h = 512; /* NOTE: dimensions de l'ecran */
    SCE_Frustum_Init (&cam->frustum);
    SCE_BoundingSphere_Init (&cam->sphere);
    /* TODO: epsilon */
    SCE_BoundingSphere_GetSphere (&cam->sphere)->radius = 0.00001f;
    cam->node = NULL;
    SCE_List_InitIt (&cam->it);
    SCE_List_SetData (&cam->it, cam);
}

/**
 * \brief Creates a new camera
 * \returns the new camera
 */
SCE_SCamera* SCE_Camera_Create (void)
{
    SCE_SCamera *cam = NULL;
    if (!(cam = SCE_malloc (sizeof *cam)))
        goto fail;
    SCE_Camera_Init (cam);
    if (!(cam->node = SCE_Node_Create ()))
        goto fail;
    SCE_Node_SetData (cam->node, cam);
    SCE_Node_GetElement (cam->node)->sphere = &cam->sphere;
    return cam;
fail:
    SCE_Camera_Delete (cam);
    SCEE_LogSrc ();
    return NULL;
}
/**
 * \brief Deletes an existing camera created by SCE_Camera_Create()
 * \param cam the camera to delete
 */
void SCE_Camera_Delete (SCE_SCamera *cam)
{
    if (cam) {
        SCE_Node_Delete (cam->node);
        SCE_List_Remove (&cam->it);
        SCE_free (cam);
    }
}

/**
 * \brief Sets the viewport of a camera
 * \sa SCE_SViewport
 */
void SCE_Camera_SetViewport (SCE_SCamera *cam, int x, int y, int w, int h)
{
    cam->viewport.x = x;
    cam->viewport.y = y;
    cam->viewport.w = w;
    cam->viewport.h = h;
}

/**
 * \brief Gets the viewport of a camera
 * \sa SCE_SViewport
 */
SCE_SViewport* SCE_Camera_GetViewport (SCE_SCamera *cam)
{
    return &cam->viewport;
}


/**
 * \brief Sets the projection matrix of a camera
 * \param cam a camera
 * \param a FOV angle
 * \param r frustum ratio
 * \param n near plane
 * \param f far plane
 * \sa SCE_Camera_GetProj()
 */
void SCE_Camera_SetProjection (SCE_SCamera *cam, float a, float r,
                               float n, float f)
{
    SCE_Matrix4_Projection (cam->proj, a, r, n, f);
}

/**
 * \brief Sets a camera's projection matrix that wraps a cone
 * \param cam a camera
 * \param cone a cone
 * \param near distance of the near plane
 */
void SCE_Camera_SetProjectionFromCone (SCE_SCamera *cam, const SCE_SCone *cone,
                                       float near)
{
    /* little threshold for the far plane :> */
    SCE_Camera_SetProjection (cam, SCE_Cone_GetAngle (cone) * 2.0, 1.0, near,
                              SCE_Cone_GetHeight (cone) + 0.0001);
}


/**
 * \brief Gets the view matrix of a camera
 * \returns a pointer to the internal matrix of \p cam
 */
float* SCE_Camera_GetView (SCE_SCamera *cam)
{
    return cam->view;
}
/**
 * \brief Gets the inverse of the view matrix of a camera
 * \returns a pointer to the internal matrix of \p cam
 */
float* SCE_Camera_GetViewInverse (SCE_SCamera *cam)
{
    return cam->viewinv;
}
/**
 * \brief Gets the projection's matrix of a camera
 * \returns a pointer to the internal matrix of \p cam
 * \sa SCE_Camera_SetProjection()
 */
float* SCE_Camera_GetProj (SCE_SCamera *cam)
{
    return cam->proj;
}
/**
 * \brief Gets the inverse of the projection matrix of a camera
 * \returns a pointer to the internal matrix of \p cam
 */
float* SCE_Camera_GetProjInverse (SCE_SCamera *cam)
{
    return cam->projinv;
}

/**
 * \brief Returns the final view projection matrix
 * \sa SCE_Camera_GetFinalViewProjInverse()
 */
float* SCE_Camera_GetFinalViewProj (SCE_SCamera *cam)
{
    return cam->finalviewproj;
}

/**
 * \brief Returns the final inverse view projection matrix
 * \sa SCE_Camera_GetFinalViewProj()
 */
float* SCE_Camera_GetFinalViewProjInverse (SCE_SCamera *cam)
{
    return cam->finalviewprojinv;
}


/**
 * \brief Sets a camera's position
 * \param cam a camera
 * \param x,y,z new position of the camera
 * \note This function modifies the position of the camera node but not the
 * camera view matrix (SCE_Camera_GetView())
 * \sa SCE_Camera_SetPositionv()
 */
void SCE_Camera_SetPosition (SCE_SCamera *cam, float x, float y, float z)
{
    SCE_TVector3 pos;
    SCE_Vector3_Set (pos, x, y, z);
    SCE_Camera_SetPositionv (cam, pos);
}
/**
 * \brief Sets a camera's position
 * \param cam a camera
 * \param pos new position of the camera
 * \note This function modifies the position of the camera node but not the
 * camera view matrix (SCE_Camera_GetView())
 * \sa SCE_Camera_SetPosition()
 */
void SCE_Camera_SetPositionv (SCE_SCamera *cam, const SCE_TVector3 pos)
{
    /* TODO: float maybe wrong matrix type */
    float *mat = SCE_Node_GetMatrix (cam->node, SCE_NODE_WRITE_MATRIX);
    SCE_Matrix4_SetTranslation (mat, pos);
    SCE_Node_HasMoved (cam->node);
}
/**
 * \brief Gets the position of a camera
 */
void SCE_Camera_GetPositionv (const SCE_SCamera *cam, SCE_TVector3 pos)
{
    SCE_Matrix4_GetTranslation (cam->finalviewinv, pos);
}

/**
 * \brief Gets the node of a camera
 * \returns the node of \p cam
 * \sa SCE_SNode
 */
SCE_SNode* SCE_Camera_GetNode (SCE_SCamera *cam)
{
    return cam->node;
}

/**
 * \brief Gets the frustum of a camera
 * \sa SCE_SFrustum
 * \sa SCE_Camera_GetNear()
 */
SCE_SFrustum* SCE_Camera_GetFrustum (SCE_SCamera *cam)
{
    return &cam->frustum;
}

/**
 * \brief Gets the distance of the near plane of a camera projection matrix
 * \param cam a camera
 * \returns distance of the near plane
 * \sa SCE_Camera_GetFar(), SCE_Camera_GetFrustum()
 */
float SCE_Camera_GetNear (const SCE_SCamera *cam)
{
    SCE_TVector3 pos;
    SCE_Camera_GetPositionv (cam, pos);
    /* TODO: hack, DistanceToPointv() can return a negative value */
    return SCE_Math_Fabsf (
        SCE_Plane_DistanceToPointv (&cam->frustum.planes[SCE_FRUSTUM_NEAR],
                                    pos));
}
/**
 * \brief Gets the distance of the far plane of a camera projection matrix
 * \param cam a camera
 * \returns distance of the far plane
 * \sa SCE_Camera_GetNear(), SCE_Camera_GetFrustum()
 */
float SCE_Camera_GetFar (const SCE_SCamera *cam)
{
    SCE_TVector3 pos;
    SCE_Camera_GetPositionv (cam, pos);
    /* TODO: hack, DistanceToPointv() can return a negative value */
    return SCE_Math_Fabsf (
        SCE_Plane_DistanceToPointv (&cam->frustum.planes[SCE_FRUSTUM_FAR],
                                    pos));
}

/**
 * \brief Gets the final view matrix of a camera
 */
float* SCE_Camera_GetFinalView (SCE_SCamera *cam)
{
    return cam->finalview;
}
/**
 * \brief Gets the invers of the final view matrix of a camera
 */
float* SCE_Camera_GetFinalViewInverse (SCE_SCamera *cam)
{
    return cam->finalviewinv;
}

/**
 * \brief Gets the base vectors of a camera
 * \sa SCE_Camera_GetPositionv(), SCE_Camera_GetFinalViewInverse()
 */
void SCE_Camera_GetBase (SCE_SCamera *cam, SCE_TVector3 x, SCE_TVector3 y,
                         SCE_TVector3 z)
{
    SCE_GetMat4C3 (0, cam->finalviewinv, x);
    SCE_GetMat4C3 (1, cam->finalviewinv, y);
    SCE_GetMat4C3 (2, cam->finalviewinv, z);
}

/**
 * \brief Returns the iterator of a camera
 */
SCE_SListIterator* SCE_Camera_GetIterator (SCE_SCamera *cam)
{
    return &cam->it;
}


/* TODO: node's matrix isn't updated, false positionning in the octree! */
static void SCE_Camera_UpdateView (SCE_SCamera *cam)
{
    SCE_TMatrix4 mat;
    SCE_Node_GetFinalMatrixv (cam->node, mat);
    SCE_Matrix4_InverseCopy (mat);
    SCE_Matrix4_Mul (cam->view, mat, cam->finalview);
}

/**
 * \internal
 * \brief Updates the frustum of a camera from its matrices
 *
 * Updates the internal frustum structure of \p cam from its view and projection
 * matrices.
 * \sa SCE_Frustum_MakeFromMatrices()
 * \todo Calling this function in the callback of the node.. ?
 */
static void SCE_Camera_UpdateFrustum (SCE_SCamera *cam)
{
    SCE_Camera_UpdateView (cam);
    SCE_Frustum_MakeFromMatrices (&cam->frustum, cam->finalview, cam->proj);
}

static void SCE_Camera_UpdateViewProj (SCE_SCamera *cam)
{
    SCE_Matrix4_Mul (cam->proj, cam->finalview, cam->finalviewproj);
}

/**
 * \internal
 * \brief Updates a camera
 *
 * Computes the final view matrix by combining the view matrix with the node
 * final matrix. Computes the inverse matrices. Updates the frustum.
 *
 * \warning Must be called only one time per frame
 */
void SCE_Camera_Update (SCE_SCamera *cam)
{
    SCE_Camera_UpdateFrustum (cam);
    SCE_Camera_UpdateViewProj (cam);
    SCE_Matrix4_Inverse (cam->finalview, cam->finalviewinv);
    SCE_Matrix4_Inverse (cam->view, cam->viewinv);
    SCE_Matrix4_Inverse (cam->proj, cam->projinv);
    SCE_Matrix4_Inverse (cam->finalviewproj, cam->finalviewprojinv);
}

#if 0
/* deprecated */
/**
 * \brief Defines the used camera by setting the OpenGL's matrices and viewport
 * \param cam the camera to use
 */
void SCE_Camera_Use (SCE_SCamera *cam)
{
    SCE_CViewport (cam->viewport.x, cam->viewport.y,
                   cam->viewport.w, cam->viewport.h);
    SCE_CSetActiveMatrix (SCE_MAT_PROJECTION);
    SCE_CLoadMatrix (cam->proj);  /* NOTE: Load ou Mult ? */
    SCE_CSetActiveMatrix (SCE_MAT_MODELVIEW);
    SCE_CLoadMatrix (cam->finalview);  /* Load ou Mult ? */
}
#endif


/**
 * \brief Projects a point from world space to the screen space of a camera
 * \param cam a camera
 * \param u the point to project
 * \returns 1 / w
 * \sa SCE_Camera_UnProject(), SCE_Camera_Line()
 */
float SCE_Camera_Project (SCE_SCamera *cam, SCE_TVector3 u)
{
    float inv;
    SCE_TVector4 v;
    SCE_Vector4_Set (v, u[0], u[1], u[2], 1.0);
    SCE_Matrix4_MulV4Copy (cam->finalviewproj, v);
    inv = 1.0 / v[3];
    SCE_Vector3_Operator2 (u, =, v, *, inv);
    return inv;
}
/**
 * \brief Unprojects a point from the screen space of a camera to world space
 * \param cam a camera
 * \param u the point to unproject, coordinates must be between -1 and 1
 * \sa SCE_Camera_Project(), SCE_Camera_Line()
 */
void SCE_Camera_UnProject (SCE_SCamera *cam, SCE_TVector3 u)
{
    float inv;
    SCE_TVector4 v;
    SCE_Vector4_Set (v, u[0], u[1], u[2], 1.0);
    SCE_Matrix4_MulV4Copy (cam->finalviewprojinv, v);
    inv = 1.0 / v[3];
    SCE_Vector3_Operator2 (u, =, v, *, inv);
}
/**
 * \brief Gets the world space line that crosses both an unprojected point
 * and the position of the camera
 * \param cam a camera
 * \param p a 2D point on the screen, coordinates must be between -1 and 1
 * \param l the resulting line
 *
 * For example if \p is 0,0 then the line will be the view vector of the camera
 * \sa SCE_Camera_Project(), SCE_Camera_UnProject()
 */
void SCE_Camera_Line (SCE_SCamera *cam, SCE_TVector2 p, SCE_SLine3 *l)
{
    SCE_TVector3 v, pos;
    SCE_Camera_GetPositionv (cam, pos);
    SCE_Vector3_Set (v, p[0], p[1], 0.0);
    SCE_Camera_UnProject (cam, v);
    SCE_Line3_Set (l, pos, v);
    SCE_Line3_Normalize (l);
}

/** @} */

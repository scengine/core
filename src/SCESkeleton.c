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

/* created: 05/04/2009
   updated: 15/05/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCESkeleton.h"

/**
 * \defgroup skeleton Skeleton
 * \ingroup interface
 * \internal
 * \brief Skeleton for animations
 */

/** @{ */

/**
 * \brief Initializes a skeleton structure
 */
void SCE_Skeleton_Init (SCE_SSkeleton *skel)
{
    unsigned int i;
    skel->joints = NULL;
    skel->n_joints = 0;
    for (i = 0; i < SCE_MAX_SKELETON_MATRICES; i++)
        skel->mat[i] = NULL;
}

/**
 * \brief Creates a new skeleton
 */
SCE_SSkeleton* SCE_Skeleton_Create (void)
{
    SCE_SSkeleton *skel = NULL;
    if (!(skel = SCE_malloc (sizeof *skel)))
        SCEE_LogSrc ();
    else
        SCE_Skeleton_Init (skel);
    return skel;
}

/**
 * \brief Deletes a skeleton
 */
void SCE_Skeleton_Delete (SCE_SSkeleton *skel)
{
    if (skel)
    {
        unsigned int i;
        SCE_Skeleton_FreeJoints (skel);
        for (i = 0; i < SCE_MAX_SKELETON_MATRICES; i++)
            SCE_free (skel->mat[i]);
        SCE_free (skel);
    }
}


/**
 * \brief Sets joints for a skeleton
 * \sa SCE_Skeleton_GetJoints(), SCE_Skeleton_GetNumJoints(),
 * SCE_Skeleton_AllocateJoints()
 */
void SCE_Skeleton_SetJoints (SCE_SSkeleton *skel, SCE_SJoint *j, unsigned int n)
{
    SCE_Skeleton_FreeJoints (skel);
    skel->joints = j;
    skel->n_joints = n;
}
/**
 * \brief Gets the joints of a skeleton
 * \sa SCE_Skeleton_GetNumJoints(), SCE_Skeleton_SetJoints(),
 * SCE_Skeleton_AllocateJoints()
 */
SCE_SJoint* SCE_Skeleton_GetJoints (SCE_SSkeleton *skel)
{
    return skel->joints;
}

/**
 * \brief Gets the number of joints of a skeleton
 * \sa SCE_Skeleton_GetJoints(), SCE_Skeleton_SetJoints()
 */
unsigned int SCE_Skeleton_GetNumJoints (SCE_SSkeleton *skel)
{
    return skel->n_joints;
}


/**
 * \brief Allocates \p n_joints joints in a skeleton
 * \param n_joints the number of joints to allocate
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \sa SCE_Skeleton_FreeJoints()
 */
int SCE_Skeleton_AllocateJoints (SCE_SSkeleton *skel, unsigned int n_joints)
{
    unsigned int i;

    SCE_Skeleton_FreeJoints (skel);
    skel->joints = SCE_malloc (n_joints * sizeof *skel->joints);
    if (!skel->joints)
    {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    if (SCE_Skeleton_AllocateMatrices (skel, 0) < 0)
    {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    skel->n_joints = n_joints;
    for (i = 0; i < n_joints; i++)
        SCE_Joint_Init (&skel->joints[i]);
    return SCE_OK;
}

/**
 * \brief Frees all the joints of a skeleton
 * \sa SCE_Skeleton_AllocateJoints()
 */
void SCE_Skeleton_FreeJoints (SCE_SSkeleton *skel)
{
    SCE_free (skel->joints), skel->joints = NULL;
    skel->n_joints = 0;
}


/**
 * \brief Allocates an array of matrices for a skeleton
 * \param n the array number, maximum is SCE_MAX_SKELETON_MATRICES - 1
 * \sa SCE_Skeleton_FreeMatrices(), SCE_MAX_SKELETON_MATRICES
 */
int SCE_Skeleton_AllocateMatrices (SCE_SSkeleton *skel, unsigned int n)
{
    SCE_free (skel->mat[n]);
    if (!(skel->mat[n] = SCE_malloc (skel->n_joints * 12 * sizeof (float))))
    {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    SCE_Skeleton_Identity (skel, n);
    return SCE_OK;
}
/**
 * \brief Frees matrices of a skeleton
 * \param n the number of the array to delete
 */
void SCE_Skeleton_FreeMatrices (SCE_SSkeleton *skel, unsigned int n)
{
    SCE_free (skel->mat[n]), skel->mat[n] = NULL;
}


/**
 * \brief Makes sure that the parent of any joint is stored before him
 * \todo I think this function doesn't work lolptdr.
 */
void SCE_Skeleton_SortJoints (SCE_SSkeleton *skel)
{
    int i, id;
    for (i = 0; i < skel->n_joints; i++)
    {
        id = SCE_Joint_GetParentID (&skel->joints[i]);
        /* if a parent is stored after its children */
        if (id > i)
        {
            /* then exchange them */
            SCE_Joint_Exchange (&skel->joints[i], &skel->joints[id]);
            SCE_Joint_SetParentID (&skel->joints[id], i);
            i--;                /* don't skip the switched parent joint */
        }
    }
}


/**
 * \brief Computes the absolute position and orientation of the joints of a
 * skeleton
 * \sa SCE_Skeleton_ComputeAbsoluteMatrices()
 */
void SCE_Skeleton_ComputeAbsoluteJoints (SCE_SSkeleton *skel)
{
    unsigned int i;
    int id;
    SCE_SJoint *j1 = NULL, *j2 = NULL;
    SCE_TVector3 v;

    for (i = 0; i < skel->n_joints; i++)
    {
        j1 = &skel->joints[i];
        id = SCE_Joint_GetParentID (j1);
        if (id >= 0)
        {
            j2 = &skel->joints[id];
            SCE_Quaternion_RotateV3 (j2->orientation, j1->position, v);
            SCE_Vector3_Operator2v (j1->position, =, j2->position, +, v);
            SCE_Quaternion_MulCopyInv (j2->orientation, j1->orientation);
            SCE_Quaternion_Normalize (j1->orientation);
        }
    }
}


/**
 * \brief Computes the matrices of the joints of a skeleton
 * \param n the array matrix to use as output
 *
 * This function calls SCE_Joint_ComputeMatrix() for each joint of \p skel
 * \sa SCE_Joint_ComputeMatrix(), SCE_Skeleton_AllocateMatrices()
 */
void SCE_Skeleton_ComputeMatrices (SCE_SSkeleton *skel, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < skel->n_joints; i++)
        SCE_Joint_ComputeMatrix (&skel->joints[i], &skel->mat[n][i * 12]);
}

/**
 * \brief Loads the identity matrix into each matrices of the \p n th array of
 * the given skeleton
 * \sa SCE_Skeleton_ComputeMatrices(), SCE_MAX_SKELETON_MATRICES
 */
void SCE_Skeleton_Identity (SCE_SSkeleton *skel, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < skel->n_joints; i++)
        SCE_Matrix4x3_Identity (&skel->mat[n][i * 12]);
}
/**
 * \brief Computes the inverse matrices of the \p n1 th matrix array of \p skel1
 * and store the result into the \p n2 th matrix array of \p skel2
 * \sa SCE_Skeleton_ComputeMatrices(), SCE_Skeleton_Mul(),
 * SCE_MAX_SKELETON_MATRICES
 */
void SCE_Skeleton_Inverse (SCE_SSkeleton *skel1, unsigned int n1,
                           SCE_SSkeleton *skel2, unsigned int n2)
{
    unsigned int i;
    for (i = 0; i < skel2->n_joints * 12; i += 12)
        SCE_Matrix4x3_Inverse (&skel1->mat[n1][i], &skel2->mat[n2][i]);
}
/**
 * \brief Computes the absolute matrices of a skeleton
 * \param skel1,n1 source matrices
 * \param skel2,n2 destination matrices
 */
void SCE_Skeleton_Absolute (SCE_SSkeleton *skel1, unsigned int n1,
                            SCE_SSkeleton *skel2, unsigned int n2)
{
    unsigned int i;
    int id;

    for (i = 0; i < skel2->n_joints; i++)
    {
        id = SCE_Joint_GetParentID (&skel1->joints[i]);
        if (id < 0)
            SCE_Matrix4x3_Copy (&skel2->mat[n2][i * 12], &skel1->mat[n1][i * 12]);
        else
            SCE_Matrix4x3_Mul (&skel1->mat[n1][id * 12], &skel1->mat[n1][i * 12],
                               &skel2->mat[n2][i * 12]);
    }
}
#if 0
/**
 * \deprecated
 * \brief Computes the absolute matrices of a skeleton
 * \param skel skeleton
 * \param n source and destination matrices
 */
void SCE_Skeleton_AbsoluteCopy (SCE_SSkeleton *skel, unsigned int n);
{
    unsigned int i;
    int id;
    SCE_TMatrix4x3 m;

    for (i = 0; i < skel->n_joints; i++)
    {
        id = SCE_Joint_GetParentID (&skel->joints[i]);
        if (id >= 0)
        {
            SCE_Matrix4x3_Mul (&skel->mat[n][id * 12], &skel->mat[n][i * 12],m);
            SCE_Matrix4x3_Copy (&skel->mat[n][i * 12], m);
        }
    }
}
#endif
/**
 * \brief Multiplies the matrices of skeletons
 * \param skel1,n1 matrix1
 * \param skel2,n2 matrix2
 * \param skel3,n3 matrix1 * matrix2
 */
void SCE_Skeleton_Mul (SCE_SSkeleton *skel1, unsigned int n1,
                       SCE_SSkeleton *skel2, unsigned int n2,
                       SCE_SSkeleton *skel3, unsigned int n3)
{
    unsigned int i;
    for (i = 0; i < skel3->n_joints * 12; i += 12)
        SCE_Matrix4x3_Mul (&skel1->mat[n1][i], &skel2->mat[n2][i],
                           &skel3->mat[n3][i]);
}
/**
 * \brief Multiplies the matrices of skeletons
 * \param skel1,n1 matrix1 = matrix1 * matrix2
 * \param skel2,n2 matrix2
 */
void SCE_Skeleton_MulCopy (SCE_SSkeleton *skel1, unsigned int n1,
                           SCE_SSkeleton *skel2, unsigned int n2)
{
    unsigned int i;
    for (i = 0; i < skel2->n_joints * 12; i += 12)
        SCE_Matrix4x3_MulCopy (&skel1->mat[n1][i], &skel2->mat[n2][i]);
}


/**
 * \brief Interpolates two skeletons using SLERP for the quaternions
 * \param skel1,skel2 the skeletons to interpolate
 * \param w the interpolation factor, must be in [0; 1]
 * \param result where store the resulting skeleton
 *
 * Interpolates two skeletons (\p skel1 and \p skel2) and store the result in
 * \p result. \p result must have enough joints allocated.
 * \sa SCE_Skeleton_InterpolateLinear(), SCE_Skeleton_InterpolateSLERPIndexed()
 */
void SCE_Skeleton_InterpolateSLERP (SCE_SSkeleton *skel1, SCE_SSkeleton *skel2,
                                    float w, SCE_SSkeleton *result)
{
    unsigned int i;
    for (i = 0; i < skel1->n_joints; i++)
    {
        SCE_Joint_InterpolateSLERP (&skel1->joints[i], &skel2->joints[i],
                                    w, &result->joints[i]);
    }
}
/**
 * \brief Interpolates two skeletons using SLERP for the quaternions
 *
 * This function does what SCE_Skeleton_InterpolateSLERP() does but only
 * under the indexed joints.
 * \sa SCE_Skeleton_InterpolateSLERP(), SCE_Skeleton_InterpolateLinear()
 */
void SCE_Skeleton_InterpolateSLERPIndexed (SCE_SSkeleton *skel1,
                                           SCE_SSkeleton *skel2, float w,
                                           SCE_SSkeleton *result,
                                           unsigned int *indices,
                                           unsigned int count)
{
    unsigned int i, id;
    for (i = 0; i < count; i++)
    {
        id = indices[i];
        SCE_Joint_InterpolateSLERP (&skel1->joints[id], &skel2->joints[id],
                                    w, &result->joints[id]);
    }
}

/**
 * \brief Interpolates two skeletons linearly
 * \param skel1,skel2 the skeletons between which interpolate
 * \param w the interpolation factor, must be in [0; 1]
 * \param result where store the resulting skeleton
 *
 * Interpolates two skeletons (\p skel1 and \p skel2) and store the result in
 * \p result. \p result must have enough joints allocated.
 * \sa SCE_Skeleton_InterpolateSLERP(), SCE_Skeleton_InterpolateLinearIndexed()
 */
void SCE_Skeleton_InterpolateLinear (SCE_SSkeleton *skel1, SCE_SSkeleton *skel2,
                                     float w, SCE_SSkeleton *result)
{
    unsigned int i;
    for (i = 0; i < skel1->n_joints; i++)
    {
        SCE_Joint_InterpolateLinear (&skel1->joints[i], &skel2->joints[i],
                                     w, &result->joints[i]);
    }
}
/**
 * \brief Interpolates two skeletons linearly
 *
 * This function does what SCE_Skeleton_InterpolateLinear() does but only
 * under the indexed joints.
 * \sa SCE_Skeleton_InterpolateLinear(), SCE_Skeleton_InterpolateSLERP()
 */
void SCE_Skeleton_InterpolateLinearIndexed (SCE_SSkeleton *skel1,
                                            SCE_SSkeleton *skel2, float w,
                                            SCE_SSkeleton *result,
                                            unsigned int *indices,
                                            unsigned int count)
{
    unsigned int i, id;
    for (i = 0; i < count; i++)
    {
        id = indices[i];
        SCE_Joint_InterpolateLinear (&skel1->joints[id], &skel2->joints[id],
                                     w, &result->joints[id]);
    }
}

/**
 * \brief Interpolates the matrices of two skeletons
 * \param s1,n1 matrix1
 * \param s2,n2 matrix2
 * \param w bias
 * \param r,nr = (\p matrix1 * \p w) + (\p matrix2 * (1 - \p w))
 * \sa SCE_Skeleton_InterpolateMatrices0(), SCE_Skeleton_InterpolateLinear()
 */
void SCE_Skeleton_InterpolateMatrices (SCE_SSkeleton *s1, unsigned int n1,
                                       SCE_SSkeleton *s2, unsigned int n2,
                                       float w,
                                       SCE_SSkeleton *r, unsigned int nr)
{
    unsigned int i;
    for (i = 0; i < r->n_joints * 12; i += 12)
    {
        SCE_Matrix4x3_Interpolate (&s1->mat[n1][i], &s2->mat[n2][i], w,
                                   &r->mat[nr][i]);
    }
}
/**
 * \brief Interpolates the matrices of two skeletons
 *
 * Calls SCE_Skeleton_InterpolateMatrices (\p s1, 0, \p s2, 0, \p r, 0)
 * \sa SCE_Skeleon_InterpolateMatrices(), SCE_Skeleton_InterpolateLinear()
 */
void SCE_Skeleton_InterpolateMatrices0 (SCE_SSkeleton *s1, SCE_SSkeleton *s2,
                                        float w, SCE_SSkeleton *r)
{
    unsigned int i;
    for (i = 0; i < r->n_joints * 12; i += 12)
    {
        SCE_Matrix4x3_Interpolate (&s1->mat[0][i], &s2->mat[0][i], w,
                                   &r->mat[0][i]);
    }
}

/* @} */

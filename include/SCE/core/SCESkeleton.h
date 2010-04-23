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
   updated: 13/05/2009 */

#ifndef SCESKELETON_H
#define SCESKELETON_H

#include "SCE/core/SCEJoint.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @{ */

/** \brief Maximum arrays of matrices in the skeletons */
#define SCE_MAX_SKELETON_MATRICES 4

/** \copydoc sce_sskeleton */
typedef struct sce_sskeleton SCE_SSkeleton;
/**
 * \brief A skeleton for skeletal animations
 */
struct sce_sskeleton
{
    SCE_SJoint *joints;         /**< The joints of the skeleton */
    unsigned int n_joints;      /**< Number of joints */
    float *mat[SCE_MAX_SKELETON_MATRICES]; /**< Temporary (or not) matrices */
};

/** @} */

void SCE_Skeleton_Init (SCE_SSkeleton*);
SCE_SSkeleton* SCE_Skeleton_Create (void);
void SCE_Skeleton_Delete (SCE_SSkeleton*);

void SCE_Skeleton_SetJoints (SCE_SSkeleton*, SCE_SJoint*, unsigned int);
SCE_SJoint* SCE_Skeleton_GetJoints (SCE_SSkeleton*);
unsigned int SCE_Skeleton_GetNumJoints (SCE_SSkeleton*);

int SCE_Skeleton_AllocateJoints (SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_FreeJoints (SCE_SSkeleton*);

int SCE_Skeleton_AllocateMatrices (SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_FreeMatrices (SCE_SSkeleton*, unsigned int);

void SCE_Skeleton_SortJoints (SCE_SSkeleton*);

void SCE_Skeleton_ComputeAbsoluteJoints (SCE_SSkeleton*);

void SCE_Skeleton_ComputeMatrices (SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_Identity (SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_Inverse (SCE_SSkeleton*, unsigned int,
                           SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_Absolute (SCE_SSkeleton*, unsigned int,
                            SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_Mul (SCE_SSkeleton*, unsigned int,
                       SCE_SSkeleton*, unsigned int,
                       SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_MulCopy (SCE_SSkeleton*, unsigned int,
                           SCE_SSkeleton*, unsigned int);

void SCE_Skeleton_InterpolateSLERP (SCE_SSkeleton*, SCE_SSkeleton*,
                                    float, SCE_SSkeleton*);
void SCE_Skeleton_InterpolateSLERPIndexed (SCE_SSkeleton*, SCE_SSkeleton*,
                                           float, SCE_SSkeleton*, unsigned int*,
                                           unsigned int);
void SCE_Skeleton_InterpolateLinear (SCE_SSkeleton*, SCE_SSkeleton*,
                                     float, SCE_SSkeleton*);
void SCE_Skeleton_InterpolateLinearIndexed (SCE_SSkeleton*, SCE_SSkeleton*,
                                            float, SCE_SSkeleton*,
                                            unsigned int*, unsigned int);
void SCE_Skeleton_InterpolateMatrices (SCE_SSkeleton*, unsigned int,
                                       SCE_SSkeleton*, unsigned int, float,
                                       SCE_SSkeleton*, unsigned int);
void SCE_Skeleton_InterpolateMatrices0 (SCE_SSkeleton*, SCE_SSkeleton*,
                                        float, SCE_SSkeleton*);

/*void SCE_Skeleton_SetLocalJoints*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

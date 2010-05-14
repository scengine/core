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

/* created: 04/04/2009
   updated: 15/05/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEJoint.h"

/**
 * \defgroup joint Joint
 * \ingroup interface
 * \internal
 * \brief Joint of a skeleton
 */

/** @{ */

/**
 * \brief Initializes a joint structure
 */
void SCE_Joint_Init (SCE_SJoint *joint)
{
    joint->parent = -1;
    SCE_Quaternion_Identity (joint->orientation);
    SCE_Vector3_Set (joint->position, 0.0, 0.0, 0.0);
}

/**
 * \brief Creates a new joint
 */
SCE_SJoint* SCE_Joint_Create (void)
{
    SCE_SJoint *joint = NULL;
    if (!(joint = SCE_malloc (sizeof *joint)))
        SCEE_LogSrc ();
    else
        SCE_Joint_Init (joint);
    return joint;
}

/**
 * \brief Deletes a joint
 */
void SCE_Joint_Delete (SCE_SJoint *joint)
{
    if (joint) {
        SCE_free (joint);
    }
}


/**
 * \brief Gets parent ID
 * \sa SCE_Joint_SetParentID()
 */
int SCE_Joint_GetParentID (SCE_SJoint *joint)
{
    return joint->parent;
}
/**
 * \brief Sets parent ID
 * \sa SCE_Joint_GetParentID()
 */
void SCE_Joint_SetParentID (SCE_SJoint *joint, int id)
{
    joint->parent = id;
}


/**
 * \brief Gets the orientation quaternion of a joint
 */
float* SCE_Joint_GetOrientation (SCE_SJoint *joint)
{
    return joint->orientation;
}
/**
 * \brief Gets the position vector of a joint
 */
float* SCE_Joint_GetPosition (SCE_SJoint *joint)
{
    return joint->position;
}

/**
 * \brief Copies \p j2 into \p j1
 * \sa SCE_Joint_Exchange()
 */
void SCE_Joint_Copy (SCE_SJoint *j1, SCE_SJoint *j2)
{
    j1->parent = j2->parent;
    SCE_Vector3_Copy (j1->position, j2->position);
    SCE_Quaternion_Copy (j1->orientation, j2->orientation);
}
/**
 * \brief Exchanges \p j2 and \p j1
 * \sa SCE_Joint_Copy()
 */
void SCE_Joint_Exchange (SCE_SJoint *j1, SCE_SJoint *j2)
{
    SCE_SJoint j;
    SCE_Joint_Copy (&j, j2);
    SCE_Joint_Copy (j2, j1);
    SCE_Joint_Copy (j1, &j);
}


#if 0
void SCE_Joint_InterpolateSpline (SCE_SJoint *joint1, SCE_SJoint *joint2,
                                  SCE_SJoint *joint3, float weight,
                                  SCE_SJoint *result)
{
    result->parent = joint1->parent;
    SCE_Quaternion_Spline (joint1->orientation, joint2->orientation,
                           joint3->orientation, weight, result->orientation);
    SCE_Vector3_Spline (joint1->position, joint2->position, joint3->position,
                        weight, result->position);
}
#endif
/**
 * \brief Interpolates two joints using SLERP for the quaternions
 *
 * Interpolates \p joint1 and \p joint2 with the factor \p w and store the
 * result in \p r.
 * \sa SCE_Joint_InterpolateLinear()
 */
void SCE_Joint_InterpolateSLERP (SCE_SJoint *joint1, SCE_SJoint *joint2,
                                 float w, SCE_SJoint *result)
{
    float w_ = 1.0f - w;
    result->parent = joint1->parent;
    SCE_Quaternion_SLERP (joint1->orientation, joint2->orientation,
                          w, result->orientation);
    SCE_Vector3_Operator2 (result->position, =, joint1->position, *, w_);
    SCE_Vector3_Operator2 (result->position, +=, joint2->position, *, w);
}
/**
 * \brief Interpolates two joints linearly
 *
 * Interpolates \p joint1 and \p joint2 with the factor \p w and store the
 * result in \p r.
 * \sa SCE_Joint_InterpolateSLERP()
 */
void SCE_Joint_InterpolateLinear (SCE_SJoint *joint1, SCE_SJoint *joint2,
                                  float w, SCE_SJoint *result)
{
    float w_ = 1.0f - w;
    result->parent = joint1->parent;
    SCE_Quaternion_Linear (joint1->orientation, joint2->orientation,
                           w, result->orientation);
    SCE_Vector3_Operator2 (result->position, =, joint1->position, *, w_);
    SCE_Vector3_Operator2 (result->position, +=, joint2->position, *, w);
}

/**
 * \brief Builds the matrix of a joint based on its orientation and its position
 */
void SCE_Joint_ComputeMatrix (SCE_SJoint *joint, SCE_TMatrix4x3 m)
{
    SCE_Matrix4x3_FromQuaternion (m, joint->orientation);
    m[3] = joint->position[0];
    m[7] = joint->position[1];
    m[11] = joint->position[2];
}

/* @} */

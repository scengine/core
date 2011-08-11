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

/* created: 11/08/2011
   updated: 12/08/2011 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCECone.h"

void SCE_Cone_Init (SCE_SCone *cone)
{
    cone->height = 1.0;
    cone->radius = 1.0;
    cone->angle = M_PI / 4.0;
    SCE_Cone_SetPosition (cone, 0.0, 0.0, 0.0);
    SCE_Cone_SetOrientation (cone, 0.0, 0.0, -1.0);
}

/**
 * \brief Copies a cone
 * \param dst destination
 * \param src source
 *
 * Copies \p src into \p dst.
 */
void SCE_Cone_Copy (SCE_SCone *dst, const SCE_SCone *src)
{
    dst->height = src->height;
    dst->radius = src->radius;
    dst->angle = src->angle;
    SCE_Vector3_Copy (dst->position, src->position);
    SCE_Vector3_Copy (dst->orientation, src->orientation);
}

/**
 * \brief Sets the height of a cone, angle-conservative
 * \param cone a cone
 * \param h new height of the cone
 * \sa SCE_Cone_GetHeight(), SCE_Cone_SetAngle(), SCE_Cone_SetRadius()
 */
void SCE_Cone_SetHeight (SCE_SCone *cone, float h)
{
    float ratio = h / cone->height;
    cone->height = h;
    cone->radius *= ratio;
}
/**
 * \brief Gets the height of a cone
 * \param cone a cone
 * \return the height of \p cone
 * \sa SCE_Cone_SetHeight(), SCE_Cone_GetRadius(), SCE_Cone_GetAngle()
 */
float SCE_Cone_GetHeight (const SCE_SCone *cone)
{
    return cone->height;
}

/**
 * \brief Sets the radius of the base of a cone, height-conservative
 * \param cone a cone
 * \param r new radius of the base of the cone, in radians
 * \sa SCE_Cone_GetRadius(), SCE_Cone_SetHeight(), SCE_Cone_SetAngle()
 */
void SCE_Cone_SetRadius (SCE_SCone *cone, float r)
{
    cone->radius = r;
    cone->angle = SCE_Math_Atanf (r / cone->height);
}
/**
 * \brief Gets the radius of the base of a cone
 * \param cone a cone
 * \return the radius, in radians, of the base of \p cone
 * \sa SCE_Cone_SetRadius(), SCE_Cone_GetHeight(), SCE_Cone_GetAngle()
 */
float SCE_Cone_GetRadius (const SCE_SCone *cone)
{
    return cone->radius;
}

/**
 * \brief Sets the angle of a cone, height-conservative
 * \param cone a cone
 * \param r new angle of the cone
 * \sa SCE_Cone_GetAngle(), SCE_Cone_SetHeight(), SCE_Cone_SetRadius()
 */
void SCE_Cone_SetAngle (SCE_SCone *cone, float a)
{
    cone->angle = a;
    cone->radius = cone->height * SCE_Math_Tanf (a);
}
/**
 * \brief Gets the angle of a cone
 * \param cone a cone
 * \return the angle of \p cone
 * \sa SCE_Cone_SetAngle(), SCE_Cone_GetHeight(), SCE_Cone_GetRadius()
 */
float SCE_Cone_GetAngle (const SCE_SCone *cone)
{
    return cone->angle;
}


void SCE_Cone_SetPosition (SCE_SCone *cone, float x, float y, float z)
{
    SCE_Vector3_Set (cone->position, x, y, z);
}
void SCE_Cone_SetPositionv (SCE_SCone *cone, SCE_TVector3 pos)
{
    SCE_Vector3_Copy (cone->position, pos);
}
void SCE_Cone_GetPositionv (const SCE_SCone *cone, SCE_TVector3 pos)
{
    SCE_Vector3_Copy (pos, cone->position);
}

void SCE_Cone_SetOrientation (SCE_SCone *cone, float x, float y, float z)
{
    SCE_Vector3_Set (cone->orientation, x, y, z);
}
void SCE_Cone_SetOrientationv (SCE_SCone *cone, SCE_TVector3 ori)
{
    SCE_Vector3_Copy (cone->orientation, ori);
}
void SCE_Cone_GetOrientationv (const SCE_SCone *cone, SCE_TVector3 ori)
{
    SCE_Vector3_Copy (ori, cone->orientation);
}

static void SCE_Cone_ApplyMatrix4 (SCE_SCone *cone, const SCE_TMatrix4 m)
{
    float scale;
    SCE_TVector3 v;
    SCE_Matrix4_MulV3Copy (m, cone->position);
    SCE_Matrix4_MulV3Copyw (m, cone->orientation, 0.0);
    SCE_Vector3_Normalize (cone->orientation); /* cancel scaling */
    SCE_Matrix4_GetScale (m, v);
    /* biggest scale factor */
    scale = MAX (v[0], v[1]);
    scale = MAX (scale, v[2]);
    SCE_Cone_SetHeight (cone, cone->height * scale);
}


/**
 * \brief A cone is modified such that the distance between two points
 * from the old and the new cone is greater or equal to a defined offset
 * \param cone a cone
 * \param n offset
 */
void SCE_Cone_Offset (SCE_SCone *cone, float n)
{
    float top = n / sin (cone->angle);
    SCE_Cone_SetHeight (cone, cone->height + n + top);
    SCE_Vector3_Operator2 (cone->position, -=, cone->orientation, *, top);
}


/**
 * \brief Applies a matrix to a cone
 * \param cone a cone
 * \param m a matrix
 * \param save a backup cone where \p cone will be stored, can be NULL
 * \sa SCE_Cone_Pop()
 */
void SCE_Cone_Push (SCE_SCone *cone, const SCE_TMatrix4 m, SCE_SCone *save)
{
    if (save)
        SCE_Cone_Copy (save, cone);
    SCE_Cone_ApplyMatrix4 (cone, m);
}
/**
 * \brief Restores a cone backuped
 * \param cone a cone
 * \param save a backup
 * \sa SCE_Cone_Push()
 */
void SCE_Cone_Pop (SCE_SCone *cone, const SCE_SCone *save)
{
    SCE_Cone_Copy (cone, save);
}

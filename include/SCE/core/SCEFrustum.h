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
 
/* created: 28/02/2008
   updated: 23/06/2009 */

#ifndef SCEFRUSTUM_H
#define SCEFRUSTUM_H

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEBoundingBox.h"
#include "SCE/core/SCEBoundingSphere.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SCE_FRUSTUM_OUT 0
#define SCE_FRUSTUM_IN 1
#define SCE_FRUSTUM_PARTIALLY 2

typedef struct sce_sfrustum SCE_SFrustum;
struct sce_sfrustum
{
    SCE_SPlane planes[6];
};

void SCE_Frustum_Init (SCE_SFrustum*);

void SCE_Frustum_MakeFromMatrices (SCE_SFrustum*, SCE_TMatrix4, SCE_TMatrix4);

int SCE_Frustum_BoundingBoxIn (SCE_SFrustum*, SCE_SBoundingBox*);
int SCE_Frustum_BoundingSphereIn (SCE_SFrustum*, SCE_SBoundingSphere*);

int SCE_Frustum_BoundingBoxInBool (SCE_SFrustum*, SCE_SBoundingBox*);
int SCE_Frustum_BoundingSphereInBool (SCE_SFrustum*, SCE_SBoundingSphere*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

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
 
/* created: 08/01/2009
   updated: 29/10/2010 */

#ifndef SCECOLLIDE_H
#define SCECOLLIDE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBoundingBox.h"
#include "SCE/core/SCEBoundingSphere.h"
#include "SCE/core/SCECone.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_COLLIDE_OUT 0
#define SCE_COLLIDE_IN 1
#define SCE_COLLIDE_PARTIALLY 2

int SCE_Collide_PlanesWithPoint (const SCE_SPlane*, size_t, float, float,float);
int SCE_Collide_PlanesWithPointv (const SCE_SPlane*, size_t, const SCE_TVector3);

int SCE_Collide_PlanesWithBB (const SCE_SPlane*, size_t, const SCE_SBoundingBox*);
int SCE_Collide_PlanesWithBBBool (const SCE_SPlane*, size_t, const SCE_SBoundingBox*);
int SCE_Collide_PlanesWithBS (const SCE_SPlane*, size_t, const SCE_SBoundingSphere*);
int SCE_Collide_PlanesWithBSBool (const SCE_SPlane*, size_t, const SCE_SBoundingSphere*);

/*int SCE_Collide_RectWithBS (const SCE_SFloatRect*, SCE_SBoundingSphere*);*/

int SCE_Collide_AABBWithPoint (const SCE_SBoundingBox*, float, float, float);
int SCE_Collide_AABBWithPointv (const SCE_SBoundingBox*, const SCE_TVector3);
int SCE_Collide_AABBWithLine (const SCE_SBoundingBox*, const SCE_SLine3*);
int SCE_Collide_AABBWithBS (const SCE_SBoundingBox*, const SCE_SBoundingSphere*);
int SCE_Collide_AABBWithBSBool (const SCE_SBoundingBox*, const SCE_SBoundingSphere*);

int SCE_Collide_BBWithPoint (const SCE_SBoundingBox*, float, float, float);
int SCE_Collide_BBWithPointv (const SCE_SBoundingBox*, const SCE_TVector3);
int SCE_Collide_BBWithLine (const SCE_SBoundingBox*, const SCE_SLine3*);
int SCE_Collide_BBWithBS (const SCE_SBoundingBox*, const SCE_SBoundingSphere*);
int SCE_Collide_BBWithBB (const SCE_SBoundingBox*, const SCE_SBoundingBox*);
int SCE_Collide_BBWithBBBool (const SCE_SBoundingBox*, const SCE_SBoundingBox*);

int SCE_Collide_BSWithPoint (const SCE_SBoundingSphere*, float, float, float);
int SCE_Collide_BSWithPointv (const SCE_SBoundingSphere*, const SCE_TVector3);
int SCE_Collide_BSWithBB (const SCE_SBoundingSphere*, const SCE_SBoundingBox*);
int SCE_Collide_BSWithBS (const SCE_SBoundingSphere*, const SCE_SBoundingSphere*);

int SCE_Collide_BCWithPoint (const SCE_SCone*, float, float, float);
int SCE_Collide_BCWithPointv (const SCE_SCone*, const SCE_TVector3);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

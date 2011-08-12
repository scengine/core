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

int SCE_Collide_PlanesWithPoint (SCE_SPlane*, size_t, float, float,float);
int SCE_Collide_PlanesWithPointv (SCE_SPlane*, size_t, SCE_TVector3);

int SCE_Collide_PlanesWithBB (SCE_SPlane*, size_t, SCE_SBoundingBox*);
int SCE_Collide_PlanesWithBBBool (SCE_SPlane*, size_t, SCE_SBoundingBox*);
int SCE_Collide_PlanesWithBS (SCE_SPlane*, size_t, SCE_SBoundingSphere*);
int SCE_Collide_PlanesWithBSBool (SCE_SPlane*, size_t, SCE_SBoundingSphere*);

/*int SCE_Collide_RectWithBS (SCE_SFloatRect*, SCE_SBoundingSphere*);*/

int SCE_Collide_AABBWithPoint (SCE_SBoundingBox*, float, float, float);
int SCE_Collide_AABBWithPointv (SCE_SBoundingBox*, SCE_TVector3);
int SCE_Collide_AABBWithLine (SCE_SBoundingBox*, SCE_SLine3*);
int SCE_Collide_AABBWithBS (SCE_SBoundingBox*, SCE_SBoundingSphere*);
int SCE_Collide_AABBWithBSBool (SCE_SBoundingBox*, SCE_SBoundingSphere*);

int SCE_Collide_BBWithPoint (SCE_SBoundingBox*, float, float, float);
int SCE_Collide_BBWithPointv (SCE_SBoundingBox*, SCE_TVector3);
int SCE_Collide_BBWithLine (SCE_SBoundingBox*, SCE_SLine3*);
int SCE_Collide_BBWithBS (SCE_SBoundingBox*, SCE_SBoundingSphere*);
int SCE_Collide_BBWithBB (SCE_SBoundingBox*, SCE_SBoundingBox*);

int SCE_Collide_BSWithPoint (SCE_SBoundingSphere*, float, float, float);
int SCE_Collide_BSWithPointv (SCE_SBoundingSphere*, SCE_TVector3);
int SCE_Collide_BSWithBB (SCE_SBoundingSphere*, SCE_SBoundingBox*);
int SCE_Collide_BSWithBS (SCE_SBoundingSphere*, SCE_SBoundingSphere*);

int SCE_Collide_BCWithPoint (const SCE_SCone*, float, float, float);
int SCE_Collide_BCWithPointv (const SCE_SCone*, const SCE_TVector3);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

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
 
/* created: 06/01/2009
   updated: 14/01/2009 */

#ifndef SCEBOUNDINGSPHERE_H
#define SCEBOUNDINGSPHERE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCESphere.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sboundingsphere SCE_SBoundingSphere;
struct sce_sboundingsphere {
    SCE_SSphere sphere;
};

void SCE_BoundingSphere_Init (SCE_SBoundingSphere*);
void SCE_BoundingSphere_SetFrom (SCE_SBoundingSphere*, SCE_SSphere*);

void SCE_BoundingSphere_Set (SCE_SBoundingSphere*, float, float, float, float);
void SCE_BoundingSphere_Setv (SCE_SBoundingSphere*, SCE_TVector3, float);

SCE_SSphere* SCE_BoundingSphere_GetSphere (SCE_SBoundingSphere*);
float* SCE_BoundingSphere_GetCenter (SCE_SBoundingSphere*);
float SCE_BoundingSphere_GetRadius (SCE_SBoundingSphere*);

void SCE_BoundingSphere_Push (SCE_SBoundingSphere*, SCE_TMatrix4, SCE_SSphere*);
void SCE_BoundingSphere_Pop (SCE_SBoundingSphere*, SCE_SSphere*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

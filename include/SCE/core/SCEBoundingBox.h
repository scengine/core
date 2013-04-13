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
 
/* created: 27/02/2008
   updated: 03/08/2009 */

#ifndef SCEBOUNDINGBOX_H
#define SCEBOUNDINGBOX_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBox.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sboundingbox SCE_SBoundingBox;
struct sce_sboundingbox {
    SCE_SBox box;
    SCE_SPlane planes[6];
};

void SCE_BoundingBox_Init (SCE_SBoundingBox*);
void SCE_BoundingBox_SetFrom (SCE_SBoundingBox*, const SCE_SBox*);

SCE_SBox* SCE_BoundingBox_GetBox (SCE_SBoundingBox*);

float* SCE_BoundingBox_GetPoints (SCE_SBoundingBox*);

void SCE_BoundingBox_MakePlanes (SCE_SBoundingBox*);
SCE_SPlane* SCE_BoundingBox_GetPlanes (SCE_SBoundingBox*);
void SCE_BoundingBox_GetPlanesv (SCE_SBoundingBox*, SCE_SPlane*);

void SCE_BoundingBox_Push (SCE_SBoundingBox*, SCE_TMatrix4, SCE_SBox*);
void SCE_BoundingBox_Pop (SCE_SBoundingBox*, SCE_SBox*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

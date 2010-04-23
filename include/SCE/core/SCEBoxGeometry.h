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

/* created: 07/08/2009
   updated: 07/08/2009 */

#ifndef SCEBOXGEOMETRY_H
#define SCEBOXGEOMETRY_H

#include "SCE/core/SCEBox.h"
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sce_eboxgeomtexcoordmode {
    SCE_BOX_NONE_TEXCOORD,
    SCE_BOX_INTERIOR_TEXCOORD,
    SCE_BOX_EXTERIOR_TEXCOORD,
    SCE_BOX_CUBEMAP_TEXCOORD
};
typedef enum sce_eboxgeomtexcoordmode SCE_EBoxGeomTexCoordMode;

int SCE_Init_BoxGeom (void);
void SCE_Quit_BoxGeom (void);

int SCE_BoxGeom_Generate (SCE_SBox*, SCE_EPrimitiveType,
                          SCE_EBoxGeomTexCoordMode, SCE_SGeometry*);
SCE_SGeometry* SCE_BoxGeom_Create (SCE_SBox*, SCE_EPrimitiveType,
                                   SCE_EBoxGeomTexCoordMode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

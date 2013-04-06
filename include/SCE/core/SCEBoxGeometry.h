/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2013  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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
   updated: 06/04/2013 */

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

enum sce_eboxgeomnormalmode {
    SCE_BOX_NONE_NORMALS,
    SCE_BOX_SHARP_NORMALS,
    SCE_BOX_SMOOTH_NORMALS
};
typedef enum sce_eboxgeomnormalmode SCE_EBoxGeomNormalMode;

int SCE_Init_BoxGeom (void);
void SCE_Quit_BoxGeom (void);

int SCE_BoxGeom_Generate (SCE_SBox*, SCE_EPrimitiveType,
                          SCE_EBoxGeomTexCoordMode, SCE_EBoxGeomNormalMode,
                          SCE_SGeometry*);
SCE_SGeometry* SCE_BoxGeom_Create (SCE_SBox*, SCE_EPrimitiveType,
                                   SCE_EBoxGeomTexCoordMode,
                                   SCE_EBoxGeomNormalMode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

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

#ifndef SCECONEGEOMETRY_H
#define SCECONEGEOMETRY_H

#include "SCE/core/SCECone.h"
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

int SCE_ConeGeom_Generate (const SCE_SCone*, SCEuint, SCE_SGeometry*);
SCE_SGeometry* SCE_ConeGeom_Create (const SCE_SCone*, SCEuint);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

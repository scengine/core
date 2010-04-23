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

/* created: 03/08/2009
   updated: 24/08/2009 */

#ifndef SCEBOX_H
#define SCEBOX_H

#include <SCE/utils/SCEUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sbox SCE_SBox;
struct sce_sbox {
    SCE_TVector3 p[8];
};

void SCE_Box_Init (SCE_SBox*);
void SCE_Box_Copy (SCE_SBox*, const SCE_SBox*);

void SCE_Box_Set (SCE_SBox*, const SCE_TVector3, float, float, float);
void SCE_Box_Setv (SCE_SBox*, const SCE_TVector3, const SCE_TVector3);
void SCE_Box_SetFromCenter (SCE_SBox*, const SCE_TVector3, float, float, float);
void SCE_Box_SetFromMinMax (SCE_SBox*, const SCE_TVector3, const SCE_TVector3);

void SCE_Box_SetSize (SCE_SBox*, float, float, float);

void SCE_Box_SetCenter (SCE_SBox*, float, float, float);
void SCE_Box_SetCenterv (SCE_SBox*, const SCE_TVector3);

void SCE_Box_GetCenterv (SCE_SBox*, SCE_TVector3);
float* SCE_Box_GetOrigin (SCE_SBox*);
void SCE_Box_GetOriginv (SCE_SBox*, SCE_TVector3);

float* SCE_Box_GetPoints (SCE_SBox*);
void SCE_Box_GetPointsv (SCE_SBox*, SCE_TVector3[8]);

void SCE_Box_MakePlanes (SCE_SBox*, SCE_SPlane[6]);

float SCE_Box_GetWidth (SCE_SBox*);
float SCE_Box_GetHeight (SCE_SBox*);
float SCE_Box_GetDepth (SCE_SBox*);
void SCE_Box_GetDimensionsv (SCE_SBox*, float*, float*, float*);

void SCE_Box_ApplyMatrix4 (SCE_SBox*, SCE_TMatrix4);
void SCE_Box_ApplyMatrix3 (SCE_SBox*, SCE_TMatrix3);
void SCE_Box_ApplyMatrix4x3 (SCE_SBox*, SCE_TMatrix4x3);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

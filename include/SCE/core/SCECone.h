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

#ifndef SCECONE_H
#define SCECONE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_scone SCE_SCone;
struct sce_scone {
    float height;
    float radius;
    float angle;
    SCE_TVector3 position;
    SCE_TVector3 orientation;
};

void SCE_Cone_Init (SCE_SCone*);

void SCE_Cone_Copy (SCE_SCone*, const SCE_SCone*);

void SCE_Cone_SetHeight (SCE_SCone*, float);
float SCE_Cone_GetHeight (const SCE_SCone*);
void SCE_Cone_SetRadius (SCE_SCone*, float);
float SCE_Cone_GetRadius (const SCE_SCone*);
void SCE_Cone_SetAngle (SCE_SCone*, float);
float SCE_Cone_GetAngle (const SCE_SCone*);

void SCE_Cone_SetPosition (SCE_SCone*, float, float, float);
void SCE_Cone_SetPositionv (SCE_SCone*, SCE_TVector3);
void SCE_Cone_GetPositionv (const SCE_SCone*, SCE_TVector3);

void SCE_Cone_SetOrientation (SCE_SCone*, float, float, float);
void SCE_Cone_SetOrientationv (SCE_SCone*, SCE_TVector3);
void SCE_Cone_GetOrientationv (const SCE_SCone*, SCE_TVector3);

void SCE_Cone_Offset (SCE_SCone*, float);

void SCE_Cone_Push (SCE_SCone*, const SCE_TMatrix4, SCE_SCone*);
void SCE_Cone_Pop (SCE_SCone*, const SCE_SCone*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

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

/* created: 04/04/2009
   updated: 15/05/2009 */

#ifndef SCEJOINT_H
#define SCEJOINT_H

#include <SCE/utils/SCEUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct sce_sjoint SCE_SJoint;
struct sce_sjoint
{
    int parent;
    SCE_TQuaternion orientation;
    SCE_TVector3 position;
};

void SCE_Joint_Init (SCE_SJoint*);
SCE_SJoint* SCE_Joint_Create (void);
void SCE_Joint_Delete (SCE_SJoint*);

int SCE_Joint_GetParentID (SCE_SJoint*);
void SCE_Joint_SetParentID (SCE_SJoint*, int);

float* SCE_Joint_GetOrientation (SCE_SJoint*);
float* SCE_Joint_GetPosition (SCE_SJoint*);

void SCE_Joint_Copy (SCE_SJoint*, SCE_SJoint*);
void SCE_Joint_Exchange (SCE_SJoint*, SCE_SJoint*);

void SCE_Joint_InterpolateSLERP (SCE_SJoint*, SCE_SJoint*, float, SCE_SJoint*);
void SCE_Joint_InterpolateLinear (SCE_SJoint*, SCE_SJoint*, float, SCE_SJoint*);

void SCE_Joint_ComputeMatrix (SCE_SJoint*, SCE_TMatrix4x3);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

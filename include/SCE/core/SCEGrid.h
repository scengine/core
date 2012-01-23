/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2012  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 21/01/2012
   updated: 22/01/2012 */

#ifndef SCEGRID_H
#define SCEGRID_H

#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*SCE_FSerializeGrid) (void*, void*);

typedef struct sce_sgrid SCE_SGrid;
struct sce_sgrid {
    void *data;
    int width, height, depth;
    SCE_EType type;
    int built;
    void *udata;
};

void SCE_Grid_Init (SCE_SGrid*);
void SCE_Grid_Clear (SCE_SGrid*);
SCE_SGrid* SCE_Grid_Create (void);
void SCE_Grid_Delete (SCE_SGrid*);

void SCE_Grid_SetType (SCE_SGrid*, SCE_EType);
void SCE_Grid_SetDimensions (SCE_SGrid*, int, int, int);
void SCE_Grid_SetWidth (SCE_SGrid*, int);
void SCE_Grid_SetHeight (SCE_SGrid*, int);
void SCE_Grid_SetDepth (SCE_SGrid*, int);

SCE_EType SCE_Grid_GetType (const SCE_SGrid*);
void* SCE_Grid_GetRaw (SCE_SGrid*);
int SCE_Grid_GetWidth (const SCE_SGrid*);
int SCE_Grid_GetHeight (const SCE_SGrid*);
int SCE_Grid_GetDepth (const SCE_SGrid*);
int SCE_Grid_GetNumPoints (const SCE_SGrid*);
size_t SCE_Grid_GetSize (const SCE_SGrid*);

void SCE_Grid_SetData (SCE_SGrid*, void*);
void* SCE_Grid_GetData (SCE_SGrid*);

int SCE_Grid_Build (SCE_SGrid*);

void SCE_Grid_Serialize (SCE_SGrid*, SCE_FSerializeGrid, void*);

size_t SCE_Grid_GetOffset (const SCE_SGrid*, int, int, int);
void SCE_Grid_GetPoint (const SCE_SGrid*, int, int, int, void*);
void SCE_Grid_SetPoint (const SCE_SGrid*, int, int, int, void*);

int SCE_Grid_ToGeometry (const SCE_SGrid*, SCE_SGeometry*);
SCE_SGeometry* SCE_Grid_CreateGeometryFrom (const SCE_SGrid*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

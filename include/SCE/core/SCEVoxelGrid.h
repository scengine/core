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

/* created: 27/04/2012
   updated: 15/03/2013 */

#ifndef SCEVOXELGRID_H
#define SCEVOXELGRID_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_svoxelgrid SCE_SVoxelGrid;
struct sce_svoxelgrid {
    SCEulong w, h, d;
    size_t n_cmp;
    SCEubyte *data;

    int full_checked, empty_checked;
    int full, empty;
};

void SCE_VGrid_Init (SCE_SVoxelGrid*);
void SCE_VGrid_Clear (SCE_SVoxelGrid*);
SCE_SVoxelGrid* SCE_VGrid_Create (void);
void SCE_VGrid_Delete (SCE_SVoxelGrid*);

void SCE_VGrid_SetDimensions (SCE_SVoxelGrid*, SCEulong, SCEulong, SCEulong);
SCEulong SCE_VGrid_GetWidth (const SCE_SVoxelGrid*);
SCEulong SCE_VGrid_GetHeight (const SCE_SVoxelGrid*);
SCEulong SCE_VGrid_GetDepth (const SCE_SVoxelGrid*);
SCEulong SCE_VGrid_GetNumVoxels (const SCE_SVoxelGrid*);
size_t SCE_VGrid_GetSize (const SCE_SVoxelGrid*);
void* SCE_VGrid_GetRaw (SCE_SVoxelGrid*);
void SCE_VGrid_SetRaw (SCE_SVoxelGrid*, void*);

void SCE_VGrid_SetNumComponents (SCE_SVoxelGrid*, size_t);

int SCE_VGrid_Build (SCE_SVoxelGrid*);

void SCE_VGrid_Fill (SCE_SVoxelGrid*, const SCE_SLongRect3*, const SCEubyte*);
void SCE_VGrid_Copy (const SCE_SLongRect3*, SCE_SVoxelGrid*,
                     const SCE_SLongRect3*, const SCE_SVoxelGrid*);
long SCE_VGrid_CopyStats (const SCE_SLongRect3*, SCE_SVoxelGrid*,
                          const SCE_SLongRect3*, const SCE_SVoxelGrid*);
void SCE_VGrid_CopyStats2 (const SCE_SLongRect3*, SCE_SVoxelGrid*,
                           const SCE_SLongRect3*, const SCE_SVoxelGrid*,
                           long[256]);
long SCE_VGrid_FillStats (const SCE_SLongRect3*, SCE_SVoxelGrid*, SCEubyte);
void SCE_VGrid_FillStats2 (const SCE_SLongRect3*, SCE_SVoxelGrid*, SCEubyte,
                           long[256]);

SCEubyte* SCE_VGrid_Offset (SCE_SVoxelGrid*, SCEulong, SCEulong, SCEulong);

int SCE_VGrid_IsEmpty (SCE_SVoxelGrid*, const SCE_SLongRect3*);
int SCE_VGrid_IsFull (SCE_SVoxelGrid*, const SCE_SLongRect3*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

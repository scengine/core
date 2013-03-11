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

/* created: 09/03/2013
   updated: 10/03/2013 */

#ifndef SCEQEMDECIMATION_H
#define SCEQEMDECIMATION_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sqemvertex SCE_SQEMVertex;
struct sce_sqemvertex {
    SCE_TMatrix4 q;
    SCE_TVector3 v;
    long index;                 /* in case this vertex has been merged, index
                                   to the new vertex, -1 otherwise */
    SCEuint final;
};

typedef struct sce_sqemmesh SCE_SQEMMesh;
struct sce_sqemmesh {
    SCEuint max_vertices;
    SCEuint max_indices;

    SCEuint n_vertices;
    SCEuint n_indices;

    SCE_SQEMVertex *vertices;
    SCEindices *indices;
};

void SCE_QEMD_Init (SCE_SQEMMesh*);
void SCE_QEMD_Clear (SCE_SQEMMesh*);

void SCE_QEMD_SetMaxVertices (SCE_SQEMMesh*, SCEuint);
void SCE_QEMD_SetMaxIndices (SCE_SQEMMesh*, SCEuint);

int SCE_QEMD_Build (SCE_SQEMMesh*);

void SCE_QEMD_Set (SCE_SQEMMesh*, const SCEvertices*, const SCEindices*,
                   SCEuint, SCEuint);
void SCE_QEMD_AnchorVertices (SCE_SQEMMesh*, const SCEindices*, SCEuint);
void SCE_QEMD_Get (SCE_SQEMMesh*, SCEvertices*, SCEindices*, SCEuint*,SCEuint*);

void SCE_QEMD_Process (SCE_SQEMMesh*, SCEuint);
void SCE_QEMD_FixInversion (SCE_SQEMMesh*, const SCEvertices*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

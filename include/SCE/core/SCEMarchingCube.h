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

/* created: 07/03/2013
   updated: 07/03/2013 */

#ifndef SCEMARCHINGCUBE_H
#define SCEMARCHINGCUBE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCEGrid.h"

#ifdef __cplusplus
extern "C" {
#endif

/* indices numbering
 *
 *    y   z
 *     \  |
 *     0\ |2
 *       \|____ x
 *           1
 */

typedef struct sce_smccell SCE_SMCCell;
struct sce_smccell {
    SCEuint x, y, z;
    SCEuint indices[3];
    SCEuint conf;
};

typedef struct sce_smcgenerator SCE_SMCGenerator;
struct sce_smcgenerator {
    size_t n_cells;
    SCE_SMCCell *cells;
    size_t n_indices;
    SCEuint *cell_indices;
    SCEuint x, y, z;
    SCEuint w, h, d;
};

void SCE_MC_Init (SCE_SMCGenerator*);
void SCE_MC_Clear (SCE_SMCGenerator*);

void SCE_MC_SetNumCells (SCE_SMCGenerator*, size_t);
size_t SCE_MC_GetNumCells (const SCE_SMCGenerator*);

int SCE_MC_Build (SCE_SMCGenerator*);

size_t SCE_MC_GenerateVertices (SCE_SMCGenerator*, const SCE_SIntRect3*,
                                const SCE_SGrid*, SCEvertices*);
void SCE_MC_GenerateNormals (SCE_SMCGenerator*, const SCE_SGrid*, SCEvertices*);
size_t SCE_MC_GenerateIndices (const SCE_SMCGenerator*, SCEindices*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

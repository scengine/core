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

/* created: 28/03/2012
   updated: 29/03/2012 */

#ifndef SCEMARCHINGTETRAHEDRA_H
#define SCEMARCHINGTETRAHEDRA_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct sce_s SCE_S;
struct sce_s {
    
};
#endif

size_t SCE_MT_GenerateCell (SCEvertices[3 * 36], const float[8],
                            const SCE_TVector3);

size_t SCE_MT_Generate (SCEvertices*, const unsigned char*,
                        const SCE_SIntRect3*, SCEuint, SCEuint, SCEuint);
void SCE_MT_GenerateNormals (SCEvertices*, const SCEvertices*, size_t,
                             const unsigned char*, SCEuint, SCEuint, SCEuint);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

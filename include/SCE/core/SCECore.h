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
 
/* created: 11/04/2010
   updated: 29/10/2010 */

#ifndef SCENGINE_H
#define SCENGINE_H

/* external dependencies */
#include <stdio.h>
#include <SCE/utils/SCEUtils.h>

/* internal dependencies */
#include "SCE/core/SCEBoundingBox.h"
#include "SCE/core/SCEBoundingSphere.h"
#include "SCE/core/SCECollide.h"
#include "SCE/core/SCEFrustum.h"
#include "SCE/core/SCELevelOfDetail.h"
#include "SCE/core/SCEOctree.h"
#include "SCE/core/SCENode.h"
#include "SCE/core/SCECamera.h"
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCEOBJLoader.h"
#include "SCE/core/SCESphereGeometry.h"
#include "SCE/core/SCEBoxGeometry.h"
#include "SCE/core/SCEParticle.h"
#include "SCE/core/SCEParticleEmitter.h"
#include "SCE/core/SCEParticleModifier.h"
#include "SCE/core/SCEParticleProcessor.h"
#include "SCE/core/SCEParticleSystem.h"
#include "SCE/core/SCEJoint.h"
#include "SCE/core/SCESkeleton.h"
#include "SCE/core/SCEAnimatedGeometry.h"
#include "SCE/core/SCEAnimation.h"
#include "SCE/core/SCEMD5Loader.h"

#ifdef __cplusplus
extern "C" {
#endif

int SCE_Init_Core (FILE*, SCEbitfield);
void SCE_Quit_Core (void);

#if 0
const char* SCE_GetVersionString (void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

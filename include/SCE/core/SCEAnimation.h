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

/* created: 09/04/2009
   updated: 13/05/2009 */

#ifndef SCEANIMATION_H
#define SCEANIMATION_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCESkeleton.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @{ */

#define SCE_LINEAR_INTERPOLATION 1
#define SCE_SLERP_INTERPOLATION 2
#define SCE_SPLINE_INTERPOLATION 3 /* not used yet */
#define SCE_MATRIX_INTERPOLATION 4

typedef void (*SCE_FAnimationInterpFunc)(SCE_SSkeleton*, SCE_SSkeleton*, float,
                                         SCE_SSkeleton*);

/** \copydoc sce_sanimation */
typedef struct sce_sanimation SCE_SAnimation;
/**
 * \brief A skeletal animation
 *
 * A skeletal animation contains many key positions for a given skeleton
 */
struct sce_sanimation
{
    SCE_SSkeleton *baseskel;    /**< Bind pose skeleton */
    int canfree_baseskel;       /**< Can \c baseskel be freed? */
    SCE_SSkeleton **keys;       /**< All the key positions of the animation */
    unsigned int n_keys;        /**< Number of position keys (frames) */
    float freq;                 /**< Frequency of the animation */
    float update_freq;          /**< The \c key updating frequency */
    float weight;               /**< Weight (woot) */
    float elapsed;              /**< Time elapsed since the last frame */
    unsigned int current, next; /**< Current and next frame */
    SCE_SSkeleton *key;         /**< Current key */
    int interp_mode;            /**< Type of interpolation used */
    SCE_FAnimationInterpFunc interp_func; /**< Interpolation function */
};

/** @} */

int SCE_Init_Anim (void);
void SCE_Quit_Anim (void);

int SCE_Anim_GetResourceType (void);

SCE_SAnimation* SCE_Anim_Create (void);
void SCE_Anim_Delete (SCE_SAnimation*);

void SCE_Anim_SetBaseSkeleton (SCE_SAnimation*, SCE_SSkeleton*, int);
SCE_SSkeleton* SCE_Anim_GetBaseSkeleton (SCE_SAnimation*);

int SCE_Anim_SetKeys (SCE_SAnimation*, SCE_SSkeleton**, unsigned int);
SCE_SSkeleton** SCE_Anim_GetKeys (SCE_SAnimation*);
unsigned int SCE_Anim_GetNumKeys (SCE_SAnimation*);
int SCE_Anim_AllocateKeys (SCE_SAnimation*, unsigned int, unsigned int);

void SCE_Anim_SetFrequency (SCE_SAnimation*, float);
void SCE_Anim_SetUpdateFrequency (SCE_SAnimation*, float);
void SCE_Anim_SetInterpolationMode (SCE_SAnimation*, int);

SCE_SSkeleton* SCE_Anim_GetCurrentKey (SCE_SAnimation*);
void SCE_Anim_ComputeCurrentKey (SCE_SAnimation*);

void SCE_Anim_Start (SCE_SAnimation*);
int SCE_Anim_Animate (SCE_SAnimation*, float);
void SCE_Anim_End (SCE_SAnimation*) SCE_GNUC_DEPRECATED;

SCE_SAnimation* SCE_Anim_Load (const char*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

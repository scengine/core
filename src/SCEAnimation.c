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
   updated: 06/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEAnimation.h"

/**
 * \defgroup animation Animation
 * \ingroup interface
 * \internal
 * \brief Skeletal animations
 * @{
 */

static int is_init = SCE_FALSE;
static int resource_type = 0;

/**
 * \internal
 * \brief Initializes the animations manager
 */
int SCE_Init_Anim (void)
{
    if (is_init)
        return SCE_TRUE;
    resource_type = SCE_Resource_RegisterType (SCE_TRUE, NULL, NULL);
    if (resource_type < 0)
        goto fail;
    is_init = SCE_TRUE;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize animation module");
    return SCE_ERROR;
}
void SCE_Quit_Anim (void)
{
    SCE_btstart ();
    SCE_btend ();
}


int SCE_Anim_GetResourceType (void)
{
    return resource_type;
}

/**
 * \internal
 * \brief Initializes an animation structure
 */
static void SCE_Anim_Init (SCE_SAnimation *anim)
{
    anim->baseskel = NULL;
    anim->canfree_baseskel = SCE_FALSE;
    anim->keys = NULL;
    anim->n_keys = 0;
    anim->freq = 1.0;
    anim->update_freq = 1.0;
    anim->elapsed = 0.0;
    anim->weight = 0.0;
    anim->current = 0;
    anim->next = 0;
    anim->key = NULL;
    anim->interp_mode = SCE_LINEAR_INTERPOLATION;
    anim->interp_func = SCE_Skeleton_InterpolateLinear;
}

/**
 * \brief Creates a new animation
 */
SCE_SAnimation* SCE_Anim_Create (void)
{
    SCE_SAnimation* anim = NULL;
    SCE_btstart ();
    if (!(anim = SCE_malloc (sizeof *anim)))
        SCEE_LogSrc ();
    else {
        SCE_Anim_Init (anim);
        if (!(anim->key = SCE_Skeleton_Create ())) {
            SCE_Anim_Delete (anim), anim = NULL;
            SCEE_LogSrc ();
        }
    }
    SCE_btend ();
    return anim;
}

static void SCE_Anim_DeleteKeys (SCE_SAnimation *anim)
{
    unsigned int i;
    for (i = 0; i < anim->n_keys; i++)
        SCE_Skeleton_Delete (anim->keys[i]);
    SCE_free (anim->keys);
    anim->n_keys = 0;
}

/**
 * \brief Deletes an animation
 */
void SCE_Anim_Delete (SCE_SAnimation *anim)
{
    if (anim) {
        if (anim->canfree_baseskel)
            SCE_Skeleton_Delete (anim->baseskel);
        SCE_Anim_DeleteKeys (anim);
        SCE_Skeleton_Delete (anim->key);
        SCE_free (anim);
    }
}


/**
 * \brief Sets the bind pose skeleton for an animation
 */
void SCE_Anim_SetBaseSkeleton (SCE_SAnimation *anim, SCE_SSkeleton *skel,
                               int canfree)
{
    if (anim->canfree_baseskel)
        SCE_Skeleton_Delete (anim->baseskel);
    anim->baseskel = skel;
    anim->canfree_baseskel = canfree;
}
/**
 * \brief Gets the bind pose sekeleton of an animation
 */
SCE_SSkeleton* SCE_Anim_GetBaseSkeleton (SCE_SAnimation *anim)
{
    return anim->baseskel;
}


/**
 * \brief Sets a group of key position for an animation
 * \param keys the key positions
 * \param n_keys the number of keys in \p keys
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \note The array \p keys will be freed on \p anim's deletion, so you have
 * to give SCE allocated memory.
 */
int SCE_Anim_SetKeys (SCE_SAnimation *anim, SCE_SSkeleton **keys,
                      unsigned int n_keys)
{
    int code = SCE_OK;
    unsigned int n;
    SCE_btstart ();
    SCE_Anim_DeleteKeys (anim);
    anim->keys = keys;
    anim->n_keys = n_keys;
    n = SCE_Skeleton_GetNumJoints (keys[0]);
    if (n > 0) {
        if (SCE_Skeleton_AllocateJoints (anim->key, n) < 0)
            goto failure;
        if (SCE_Skeleton_AllocateMatrices (anim->key, 0) < 0)
            goto failure;
    }
    goto success;
failure:
    SCEE_LogSrc ();
    code = SCE_ERROR;
success:
    SCE_btend ();
    return code;
}

/**
 * \brief Gets the keys skeleton of an animation
 */
SCE_SSkeleton** SCE_Anim_GetKeys (SCE_SAnimation *anim)
{
    return anim->keys;
}
/**
 * \brief Gets the number of keys skeleton of an animation
 */
unsigned int SCE_Anim_GetNumKeys (SCE_SAnimation *anim)
{
    return anim->n_keys;
}

/**
 * \brief Allocates memory for the keys skeleton of an animation
 * \param n_keys the number of skeleton to create
 * \param n_joints the number of joints of the skeletons,
 * set 0 doesn't allocate any joint for the keys
 * \returns SCE_ERROR on error, SCE_OK otherwise
 */
int SCE_Anim_AllocateKeys (SCE_SAnimation *anim, unsigned int n_keys,
                           unsigned int n_joints)
{
    int code = SCE_OK;
    SCE_SSkeleton **keys = NULL;
    unsigned int i;

    if (!(keys = SCE_malloc (n_keys * sizeof *keys)))
        goto failure;
    for (i = 0; i < n_keys; i++) {
        if (!(keys[i] = SCE_Skeleton_Create ()))
            goto failure;
        if (n_joints > 0) {
            if (SCE_Skeleton_AllocateJoints (keys[i], n_joints) < 0)
                goto failure;
            if (SCE_Skeleton_AllocateMatrices (keys[i], 0) < 0)
                goto failure;
        }
    }
    if (SCE_Anim_SetKeys (anim, keys, n_keys) < 0)
        goto failure;
    goto success;
failure:
    SCEE_LogSrc ();
    code = SCE_ERROR;
success:
    SCE_btend ();
    return code;
}


/**
 * \brief Sets the frequency of an animation
 */
void SCE_Anim_SetFrequency (SCE_SAnimation *anim, float freq)
{
    anim->freq = freq;
}

/**
 * \brief Sets the update frequency of the interpolated key
 * \sa SCE_Anim_SetFrequency(), SCE_Anim_GetCurrentKey()
 */
void SCE_Anim_SetUpdateFrequency (SCE_SAnimation *anim, float freq)
{
    anim->update_freq = freq;
}

/**
 * \brief Sets the interpolation mode between two keys for an animation
 * \param mode the interpolation mode, can be SCE_INTERPOLATION_LINEAR
 * (default), SCE_INTERPOLATION_SLERP or SCE_INTERPOLATION_SPLINE (but this one
 * isn't yet supported).
 */
void SCE_Anim_SetInterpolationMode (SCE_SAnimation *anim, int mode)
{
    anim->interp_mode = mode;
    switch (mode) {
    case SCE_LINEAR_INTERPOLATION:
        anim->interp_func = SCE_Skeleton_InterpolateLinear;
        break;
    case SCE_SLERP_INTERPOLATION:
        anim->interp_func = SCE_Skeleton_InterpolateSLERP;
        break;
    case SCE_MATRIX_INTERPOLATION:
        anim->interp_func = SCE_Skeleton_InterpolateMatrices0;
#if 0
        break;
    case SCE_SPLINE_INTERPOLATION:
        anim->interp_func = SCE_Skeleton_InterpolateSpline;
#endif
    }
}


/**
 * \brief Gets the current key position
 */
SCE_SSkeleton* SCE_Anim_GetCurrentKey (SCE_SAnimation *anim)
{
    return anim->key;
}

/**
 * \brief Computes the current key position by interpolating it
 */
void SCE_Anim_ComputeCurrentKey (SCE_SAnimation *anim)
{
    anim->interp_func (anim->keys[anim->current], anim->keys[anim->next],
                       anim->weight, anim->key);
}


/**
 * \brief Starts an animation
 * \param anim the animation to start
 * \param start_time start time (in seconds)
 * \sa SCE_Anim_Animate()
 * \todo replace intepolate() by copy()
 */
void SCE_Anim_Start (SCE_SAnimation *anim)
{
    SCE_Skeleton_InterpolateLinear (anim->keys[0],anim->keys[0], 0.5,anim->key);
    anim->elapsed = 0.0;
    anim->weight = 0.0;
    anim->current = 0;
    anim->next = 1;
}

/**
 * \brief Animates an animation
 * \param anim the animation to animate
 * \param current_time the current time (in seconds)
 * \returns a boolean, which indicates if the current interpolated key has
 * changed (can be used to apply (or not) the current key to an animated mesh)
 * \sa SCE_Anim_GetCurrentKey()
 * \todo time-location for the frames, not linear animation
 */
int SCE_Anim_Animate (SCE_SAnimation *anim, float offset)
{
    float addframes;
    int addframesi;
    anim->elapsed += offset;
    addframes = anim->elapsed * anim->freq;
    addframesi = addframes;
    anim->weight = addframes - (int)addframes;
    anim->current += addframesi;
    anim->next += addframesi;
    anim->elapsed -= addframesi / anim->freq;
    while (anim->current >= anim->n_keys)
        anim->current -= anim->n_keys;
    while (anim->next >= anim->n_keys)
        anim->next -= anim->n_keys;
    return 1;
}

/**
 * \deprecated
 * \todo useless function
 * \brief Terminates an animation
 */
void SCE_Anim_End (SCE_SAnimation *anim)
{
    anim->elapsed = 0.0;
    anim->weight = 0.0;
    anim->current = 0;
    anim->next = 1;
}


/**
 * \brief Loads an animation from a file
 */
SCE_SAnimation* SCE_Anim_Load (const char *fname)
{
    SCE_SAnimation *anim = NULL;
    SCE_btstart ();
    if (!(anim = SCE_Resource_Load (resource_type, fname, SCE_FALSE, NULL)))
        SCEE_LogSrc ();
    SCE_btend ();
    return anim;
}

/** @} */

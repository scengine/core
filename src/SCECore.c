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
 
/* created: 16/04/2010
   updated: 23/01/2012 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <pthread.h>
#include "SCE/core/SCECore.h"

/**
 * \defgroup SCEngine SCEngine
 *
 * SCEngine is a free and open-source 3D real time rendering engine written
 * in the C language
 */

/** @{ */

static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
static int init_n = 0;

/**
 * \defgroup core Core features
 * \ingroup SCEngine
 * \internal
 * \brief
 */

/**
 * \brief Initializes the core
 * \param outlog stream used to log informations and errors
 * \param flags currently unuset, set it to 0
 * \returns SCE_ERROR if any error has occured, SCE_OK otherwise
 */
int SCE_Init_Core (FILE *outlog, SCEbitfield flags)
{
    int ret = SCE_OK;
    if (pthread_mutex_lock (&init_mutex) != 0) {
        SCEE_Log (42);
        SCEE_LogMsg ("failed to lock initialisation mutex of interface");
        return SCE_ERROR;
    }
    init_n++;
    if (init_n == 1) {
        if (SCE_Init_Utils (outlog) < 0 ||
            SCE_Init_Noise () < 0 ||
            SCE_Init_Geometry () < 0 ||
            SCE_Init_Image () < 0 ||
            SCE_Init_BoxGeom () < 0 ||
            SCE_Init_OBJ () < 0 ||
            SCE_Init_AnimGeom () < 0 ||
            SCE_Init_Anim () < 0 ||
            SCE_Init_idTechMD5 () < 0) {
            ret = SCE_ERROR;
        }
    }
    pthread_mutex_unlock (&init_mutex);
    if (ret == SCE_ERROR) {
        SCE_Quit_Core ();
        SCEE_LogSrc ();
        SCEE_LogSrcMsg ("failed to initialize SCEngine interface");
    }
    return ret;
}

/**
 * \brief Quit core
 * This function frees every modules initialised by SCE_Init_Core()
 * \returns SCE_ERROR if any error has occured, SCE_OK otherwise
 */
void SCE_Quit_Core (void)
{
    if (pthread_mutex_lock (&init_mutex) != 0) {
        SCEE_Log (42);
        SCEE_LogMsg ("failed to lock initialization mutex of interface");
    } else {
        init_n--;
        if (init_n < 0) {
            init_n = 0;         /* user made an useless call */
        } else if (init_n == 0) {
            SCE_Quit_idTechMD5 ();
            SCE_Quit_Anim ();
            SCE_Quit_AnimGeom ();
            SCE_Quit_OBJ ();
            SCE_Quit_BoxGeom ();
            SCE_Quit_Image ();
            SCE_Quit_Geometry ();
            SCE_Quit_Utils ();
        }
        pthread_mutex_unlock (&init_mutex);
    }
}

#if 0
/**
 * \brief Gets the version string of the SCEngine
 */
const char* SCE_GetVersionString (void)
{
    return SCE_VERSION_STRING;
}
#endif

/** @} */

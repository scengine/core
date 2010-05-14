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
 
/* created: 08/07/2007
   updated: 02/08/2009 */

#include <stdlib.h>
#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/libwar.h"
#include "SCE/core/SCEOBJLoader.h"

static int is_init = SCE_FALSE;
static int gen_indices = SCE_FALSE;
static unsigned int load_lod_level = 0; /* default LOD level to load */

static void* SCE_OBJ_Load (FILE*, const char*, void*);

int SCE_Init_OBJ (void)
{
    if (is_init)
        return SCE_OK;
    /* register loader */
    if (SCE_Media_Register (SCE_Geometry_GetResourceType(),
                            "."WAR_FILE_EXTENSION, SCE_OBJ_Load, NULL) < 0)
        goto fail;
    is_init = SCE_TRUE;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize .obj loader");
    return SCE_ERROR;
}
void SCE_Quit_OBJ (void)
{
    is_init = SCE_FALSE;
}

/**
 * \brief Generates GL indices on loading?
 */
void SCE_OBJ_ActivateIndicesGeneration (int activated)
{
    gen_indices = activated;
}
/**
 * \brief Considering that one .obj object is a level of detail, this function
 * selects which one load
 */
void SCE_OBJ_LoadLOD (unsigned int level)
{
    load_lod_level = level;
}

static void* SCE_OBJ_Load (FILE *fp, const char *fname, void *unused)
{
    SCE_SGeometry *geom = NULL;
    WarMesh *me = NULL;

    (void)unused;

    me = war_read (fp, gen_indices, load_lod_level);
    if (!me) {
        SCEE_LogSrc ();
        SCEE_LogSrcMsg ("libwar can't load '%s': %s", fname, war_geterror ());
        return NULL;
    }

    if (!(geom = SCE_Geometry_Create ()))
        goto fail;

    SCE_Geometry_SetPrimitiveType (geom, SCE_TRIANGLES);

    if (SCE_Geometry_SetData (geom, me->pos, me->nor, me->tex, NULL,
                              me->vcount, me->icount) < 0)
        goto fail;
    me->pos = me->nor = me->tex = NULL;
    if (me->indices) {
        SCE_SGeometryArray array;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayIndices (&array, SCE_UNSIGNED_INT,
                                      me->indices, SCE_TRUE);
        if (SCE_Geometry_SetIndexArrayDupDup (geom, &array, SCE_FALSE) < 0)
            goto fail;
    }

    war_free (me);
    return geom;
fail:
    war_free (me);
    SCE_Geometry_Delete (geom);
    SCEE_LogSrc ();
    return NULL;
}

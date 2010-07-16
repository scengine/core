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

/* created: 25/07/2009
   updated: 17/04/2010 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

static int is_init = SCE_FALSE;
static int resource_type = 0;

int SCE_Init_Geometry (void)
{
    if (is_init)
        return SCE_OK;
    resource_type = SCE_Resource_RegisterType (SCE_TRUE, NULL, NULL);
    if (resource_type < 0)
        goto fail;
    is_init = SCE_TRUE;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize geometry module");
    return SCE_ERROR;
}
void SCE_Quit_Geometry (void)
{
    resource_type = 0;
    is_init = SCE_FALSE;
}

int SCE_Geometry_GetResourceType (void)
{
    return resource_type;
}


void SCE_Geometry_InitArrayData (SCE_SGeometryArrayData *data)
{
    data->attrib = SCE_POSITION;
    data->type = SCE_FLOAT;
    data->stride = 0;
    data->size = 3;
    data->data = NULL;
}
SCE_SGeometryArrayData* SCE_Geometry_CreateArrayData (void)
{
    SCE_SGeometryArrayData *data = NULL;
    if (!(data = SCE_malloc (sizeof *data)))
        SCEE_LogSrc ();
    else
        SCE_Geometry_InitArrayData (data);
    return data;
}
void SCE_Geometry_DeleteArrayData (SCE_SGeometryArrayData *data)
{
    if (data) {
        SCE_free (data);
    }
}


static void SCE_Geometry_FreeArrayUser (void *auser)
{
    SCE_Geometry_ClearArrayUser (auser);
}
void SCE_Geometry_InitArray (SCE_SGeometryArray *array)
{
    SCE_Geometry_InitArrayData (&array->data);
    array->root = array->child = NULL;
    array->canfree_data = SCE_FALSE;
    SCE_List_InitIt (&array->it);
    SCE_List_SetData (&array->it, array);
#if 0
    SCE_List_InitIt (&array->it2);
    SCE_List_SetData (&array->it2, array);
#endif
    SCE_List_Init (&array->users);
    SCE_List_SetFreeFunc (&array->users, SCE_Geometry_FreeArrayUser);
    array->range[0] = array->range[1] = 0;
    array->rangeptr = NULL;
    array->geom = NULL;
}
SCE_SGeometryArray* SCE_Geometry_CreateArray (void)
{
    SCE_SGeometryArray *array = NULL;
    if (!(array = SCE_malloc (sizeof *array)))
        SCEE_LogSrc ();
    else
        SCE_Geometry_InitArray (array);
    return array;
}
SCE_SGeometryArray*
SCE_Geometry_CreateArrayFrom (SCE_EVertexAttribute attrib, SCE_EType type,
                              size_t stride, int size, void *data, int canfree)
{
    SCE_SGeometryArray *array = NULL;
    if (!(array = SCE_Geometry_CreateArray ()))
        SCEE_LogSrc ();
    else
        SCE_Geometry_SetArrayData (array, attrib, type, stride, size,
                                   data, canfree);
    return array;
}
void SCE_Geometry_DeleteArray (SCE_SGeometryArray *array)
{
    if (array) {
        if (array->child)
            array->child->root = array->root;
        SCE_List_Remove (&array->it);
        /* having a root means our data pointer is just an offset of the main
           pointer which is in and will be freed by the root array */
        if (array->canfree_data && !array->root)
            SCE_free (array->data.data);
        SCE_List_Clear (&array->users);
        SCE_free (array);
    }
}
/**
 * \brief Just copies \p a2->data into \p a1->data
 */
void SCE_Geometry_CopyArray (SCE_SGeometryArray *a1,
                             const SCE_SGeometryArray *a2)
{
    a1->data = a2->data;
}

void SCE_Geometry_InitArrayUser (SCE_SGeometryArrayUser *auser)
{
    auser->array = NULL;
    auser->update = NULL;
    auser->arg = NULL;
    SCE_List_InitIt (&auser->it);
    SCE_List_SetData (&auser->it, auser);
}
SCE_SGeometryArrayUser* SCE_Geometry_CreateArrayUser (void)
{
    SCE_SGeometryArrayUser *auser = NULL;
    if (!(auser = SCE_malloc (sizeof *auser)))
        SCEE_LogSrc ();
    else
        SCE_Geometry_InitArrayUser (auser);
    return auser;
}
void SCE_Geometry_ClearArrayUser (SCE_SGeometryArrayUser *auser)
{
    if (auser->array) {
        SCE_List_Remove (&auser->it);
        auser->array = NULL;
    }
}
void SCE_Geometry_DeleteArrayUser (SCE_SGeometryArrayUser *auser)
{
    if (auser) {
        SCE_Geometry_ClearArrayUser (auser);
        SCE_free (auser);
    }
}

static void SCE_Geometry_FreeArray (void *array)
{
    SCE_Geometry_DeleteArray (array);
}
static void SCE_Geometry_Init (SCE_SGeometry *geom)
{
    SCE_List_Init (&geom->arrays);
    SCE_List_SetFreeFunc (&geom->arrays, SCE_Geometry_FreeArray);
    SCE_List_Init (&geom->modified);
    SCE_List_SetFreeFunc (&geom->modified, SCE_Geometry_FreeArray);
    geom->index_array = NULL;
    geom->canfree_index = SCE_FALSE;
    geom->n_vertices = geom->n_indices = 0;

    geom->pos_array = geom->nor_array = geom->tex_array = NULL;
    geom->pos_data = geom->nor_data = geom->tex_data = NULL;
    geom->index_data = NULL;

    geom->sorted = NULL;
    geom->sorted_length = 0;

    SCE_Box_Init (&geom->box);
    SCE_Sphere_Init (&geom->sphere);
    geom->box_uptodate = geom->sphere_uptodate = SCE_FALSE;
}
SCE_SGeometry* SCE_Geometry_Create (void)
{
    SCE_SGeometry *geom = NULL;
    if (!(geom = SCE_malloc (sizeof *geom)))
        SCEE_LogSrc ();
    else
        SCE_Geometry_Init (geom);
    return geom;
}
static void SCE_Geometry_DeleteIndexArray (SCE_SGeometry *geom)
{
    if (geom->canfree_index)
        SCE_Geometry_DeleteArray (geom->index_array);
}
void SCE_Geometry_Delete (SCE_SGeometry *geom)
{
    if (geom) {
        SCE_List_Clear (&geom->arrays);
        SCE_List_Clear (&geom->modified);
        SCE_Geometry_DeleteIndexArray (geom);
        SCE_free (geom);
    }
}

/**
 * \brief Gets the geometry array that \p auser uses
 */
SCE_SGeometryArray* SCE_Geometry_GetUserArray (SCE_SGeometryArrayUser *auser)
{
    return auser->array;
}
/**
 * \brief Attach an array to another, this means they are interleaved
 *
 * Since \p a1 has an attached array, it is marked as root and must always be
 * the first parameter of this function.
 * \sa SCE_Geometry_GetRoot(), SCE_Geometry_GetChild()
 */
void SCE_Geometry_AttachArray (SCE_SGeometryArray *a1, SCE_SGeometryArray *a2)
{
    a2->child = a1->child;
    a2->root = a1;
    a1->child = a2;
}
/**
 * \brief Gets the root array of an array
 * \sa SCE_Geometry_AttachArray(), SCE_Geometry_GetChild()
 */
SCE_SGeometryArray* SCE_Geometry_GetRoot (SCE_SGeometryArray *array)
{
    return array->root;
}
/**
 * \brief Gets the child of an array
 * \sa SCE_Geometry_AttachArray(), SCE_Geometry_GetRoot()
 */
SCE_SGeometryArray* SCE_Geometry_GetChild (SCE_SGeometryArray *array)
{
    return array->child;
}

/**
 * \brief Adds an user to a geometry array
 * \param u the user to add
 * \brief fun callback called when \p array is updated
 *
 * The added pointer will never be freed by SCE_Geometry_DeleteArray() or
 * any other function of the geometry module.
 * \sa SCE_Geometry_RemoveUser(), SCE_Geometry_Modified(),
 * SCE_Geometry_Update(), SCE_Geometry_UpdateArray()
 */
void SCE_Geometry_AddUser (SCE_SGeometryArray *array, SCE_SGeometryArrayUser *u,
                           SCE_FUpdateGeometryArray fun, void *arg)
{
#ifdef SCE_DEBUG
    if (!fun) {
        SCEE_SendMsg ("you must specify an update function");
    }
#endif
    SCE_List_Appendl (&array->users, &u->it);
    u->update = fun;
    u->arg = arg;
    u->array = array;
}
/**
 * \brief Removes an user from its array
 * \sa SCE_Geometry_AddUser()
 */
void SCE_Geometry_RemoveUser (SCE_SGeometryArrayUser *auser)
{
    SCE_List_Removel (&auser->it);
    auser->array = NULL;
}

/**
 * \brief Defines an array as modified
 * \param range range of modified vertices, [0] is the first modified vertex
 * and [1] the number of modified vertices, if NULL the whole buffer data will
 * be updated.
 *
 * Stores the given geometry array in the modified arrays list of the array's
 * geometry. The \p array user update callback is called when \p array is
 * updated with SCE_Geometry_UpdateArray(). If \p array is a child of a root
 * array, then this functions acts on the root.
 * \sa SCE_Geometry_Update(), SCE_Geometry_UpdateArray(),
 * SCE_Geometry_AddUser()
 */
void SCE_Geometry_Modified (SCE_SGeometryArray *array, const size_t *range)
{
    if (array->root)
        array = array->root;
    if (!range)
        array->rangeptr = NULL;
    else {
        if (array->rangeptr) {
            /* increase range */
            array->range[0] = MIN (range[0], array->range[0]);
            array->range[1] = MAX (range[1], array->range[1]);
        } else {
            array->range[0] = range[0];
            array->range[1] = range[1];
            array->rangeptr = array->range;
        }
    }
    SCE_List_Removel (&array->it);
    SCE_List_Appendl (&array->geom->modified, &array->it);
}
/**
 * \brief Defines an array as unmodified
 * \sa SCE_Geometry_Modified(), SCE_Geometry_UpdateArray()
 */
void SCE_Geometry_Unmodified (SCE_SGeometryArray *array)
{
    SCE_List_Removel (&array->it);
    SCE_List_Appendl (&array->geom->arrays, &array->it);
    array->rangeptr = NULL;
}
/**
 * \brief Updates an array
 *
 * This function calls all the update callback of each \p arrays's user. It is
 * recommanded to use SCE_Geometry_Modified() and then update the whole
 * geometry by calling SCE_Geometry_Update().
 * \sa SCE_Geometry_Modified(), SCE_Geometry_AddUser(), SCE_Geometry_Update(),
 */
void SCE_Geometry_UpdateArray (SCE_SGeometryArray *array)
{
    SCE_SListIterator *it = NULL;
    SCE_List_ForEach (it, &array->users) {
        SCE_SGeometryArrayUser *auser = SCE_List_GetData (it);
        auser->update (auser->arg, array->rangeptr);
    }
    /* move the array back to the main geometry's array list */
    SCE_Geometry_Unmodified (array);
}
/**
 * \brief Updates a geometry
 *
 * Calls SCE_Geometry_UpdateArray() for each array of \p geom.
 * \sa SCE_Geomoetry_UpdateArray()
 */
void SCE_Geometry_Update (SCE_SGeometry *geom)
{
    SCE_SListIterator *it = NULL;
    SCE_List_ForEach (it, &geom->modified)
        SCE_Geometry_UpdateArray (SCE_List_GetData (it));
}


/**
 * \brief Sets data for an array
 * \param attrib vertex attribute of the data
 * \param type data type (SCE_VERTICES_TYPE is recommanded)
 * \param stride stride between two consecutive vertices (in bytes)
 * \param size vertex vector's length (usually 3 for position, 2 for texcoord)
 * \param data pointer to the vertex data
 * \param canfree can \p array free \p data when SCE_Geometry_DeleteArray()
 * or SCE_Geometry_Delete() is called?
 * \sa SCE_Geometry_SetArrayPosition(), SCE_Geometry_SetArrayTexCoord(),
 * SCE_Geometry_SetArrayNormal(), SCE_Geometry_SetArrayTangent(),
 * SCE_Geometry_SetArrayBinormal(), SCE_Geometry_SetArrayIndices(),
 */
void SCE_Geometry_SetArrayData (SCE_SGeometryArray *array,
                                SCE_EVertexAttribute attrib,
                                SCE_EType type, size_t stride, int size,
                                void *data, int canfree)
{
    array->data.attrib = attrib;
    array->data.type = type;
    array->data.stride = stride;
    array->data.size = size;
    array->data.data = data;
    array->canfree_data = canfree;
}
/**
 * \brief Calls SCE_Geometry_SetArrayData (\p array, SCE_POSITION,
 * SCE_VERTICES_TYPE, \p size, \p data, \p canfree).
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayNormal()
 */
void SCE_Geometry_SetArrayPosition (SCE_SGeometryArray *array, size_t stride,
                                    int size, SCEvertices *data, int canfree)
{
    SCE_Geometry_SetArrayData (array, SCE_POSITION, SCE_VERTICES_TYPE, stride,
                               size, data, canfree);
}
/**
 * \brief Calls SCE_Geometry_SetArrayData (\p array, SCE_TEXCOORD0 + \p unit,
 * SCE_VERTICES_TYPE, \p size, \p data, \p canfree).
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayPosition()
 */
void SCE_Geometry_SetArrayTexCoord (SCE_SGeometryArray *array,
                                    unsigned int unit, size_t stride, int size,
                                    SCEvertices *data, int canfree)
{
    SCE_Geometry_SetArrayData (array, SCE_TEXCOORD0 + unit, SCE_VERTICES_TYPE,
                               stride, size, data, canfree);
}
/**
 * \brief Calls SCE_Geometry_SetArrayData (\p array, SCE_NORMAL,
 * SCE_VERTICES_TYPE, 3, \p data, \p canfree).
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayPosition()
 */
void SCE_Geometry_SetArrayNormal (SCE_SGeometryArray *array, size_t stride,
                                  SCEvertices *data, int canfree)
{
    /* it is useless to specify the parameter 'size' (not used by the GL) */
    SCE_Geometry_SetArrayData (array, SCE_NORMAL, SCE_VERTICES_TYPE, stride, 3,
                               data, canfree);
}
/**
 * \brief Calls SCE_Geometry_SetArrayData (\p array, SCE_TANGENT,
 * SCE_VERTICES_TYPE, 3, \p data, \p canfree).
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayNormal()
 */
void SCE_Geometry_SetArrayTangent (SCE_SGeometryArray *array, size_t stride,
                                   SCEvertices *data, int canfree)
{
    /* tangent are like normals, so force size of 3 */
    SCE_Geometry_SetArrayData (array, SCE_TANGENT, SCE_VERTICES_TYPE, stride,
                               3, data, canfree);
}
/**
 * \brief Calls SCE_Geometry_SetArrayData (\p array, SCE_BINORMAL,
 * SCE_VERTICES_TYPE, 3, \p data, \p canfree).
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayNormal()
 */
void SCE_Geometry_SetArrayBinormal (SCE_SGeometryArray *array, size_t stride,
                                    SCEvertices *data, int canfree)
{
    SCE_Geometry_SetArrayData (array, SCE_BINORMAL, SCE_VERTICES_TYPE, stride,
                               3, data, canfree);
}

/**
 * \brief Sets indices to an array
 *
 * I know this function is just weird, but it would be boring to make a new
 * structure type just for "index" arrays, like SCE_SGeometryIndexArray,
 * because it would mean that SCE_SGeometryArray is a wrong type name and
 * SCE_SGeometryVertexArray would be better.
 * \sa SCE_Geometry_SetArrayData(), SCE_Geometry_SetArrayNormal()
 */
void SCE_Geometry_SetArrayIndices (SCE_SGeometryArray *array, SCE_EType type,
                                   void *data, int canfree)
{
    array->data.type = type;
    array->data.data = data;
}

/**
 * \brief Gets the vertex data of an array
 * \sa SCE_Geometry_GetArrayData(), SCE_SGeometryArrayData::data
 */
void* SCE_Geometry_GetData (SCE_SGeometryArray *array)
{
    return array->data.data;
}
/**
 * \brief Gets the attribute type of an array
 * \sa SCE_Geometry_GetArrayData(), SCE_Geometry_GetArrayArray()
 */
SCE_EVertexAttribute
SCE_Geometry_GetArrayVertexAttribute (SCE_SGeometryArray *array)
{
    return array->data.attrib;
}
/**
 * \brief Gets the vertex array of a geometry array
 * \sa SCE_Geometry_GetArrayArray(), SCE_Geometry_GetData(),
 * SCE_SGeometryArrayData
 */
SCE_SGeometryArrayData* SCE_Geometry_GetArrayData (SCE_SGeometryArray *array)
{
    return &array->data;
}

/**
 * \brief Adds a vertex array to a geometry
 *
 * The pointer \p array will be freed when SCE_Geometry_Delete() called under
 * \p geom.
 * \sa SCE_Geometry_AddArrayDup(), SCE_Geometry_RemoveArray(),
 * SCE_Geometry_SetIndexArray()
 */
void SCE_Geometry_AddArray (SCE_SGeometry *geom, SCE_SGeometryArray *array)
{
    SCE_List_Appendl (&geom->arrays, &array->it);
    array->geom = geom;
    switch (SCE_Geometry_GetArrayVertexAttribute (array)) {
    case SCE_POSITION:
        geom->pos_array = array;
        geom->pos_data = SCE_Geometry_GetData (array);
        break;
    case SCE_NORMAL:
        geom->nor_array = array;
        geom->nor_data = SCE_Geometry_GetData (array);
        break;
    case SCE_TEXCOORD0:
        geom->tex_array = array;
        geom->tex_data = SCE_Geometry_GetData (array);
    default:;                   /* kicks compilation warning */
    }
}
/**
 * \brief Adds an array and all its children
 * \note \p array must be a root array, see SCE_Geometry_AttachArray()
 * \sa SCE_Geometry_AttachArray(), SCE_Geometry_AddArray()
 */
void SCE_Geometry_AddArrayRec (SCE_SGeometry *geom, SCE_SGeometryArray *array)
{
    SCE_SGeometryArray *a = array;
    while (a) {
        SCE_Geometry_AddArray (geom, a);
        a = a->child;
    }
}
/**
 * \brief Adds a new array to a geometry
 *
 * This function will be useful, I can feel it.
 * \sa SCE_Geometry_AddArray(), SCE_Geometry_CreateArrayFrom(),
 * SCE_Geometry_AddArrayDup()
 */
SCE_SGeometryArray*
SCE_Geometry_AddNewArray (SCE_SGeometry *geom, SCE_EVertexAttribute attrib,
                          SCE_EType type, size_t stride, int size,
                          void *data, int canfree)
{
    SCE_SGeometryArray *array = NULL;
    if (!(array = SCE_Geometry_CreateArrayFrom (attrib, type, stride, size,
                                                data, canfree)))
        SCEE_LogSrc ();
    else
        SCE_Geometry_AddArray (geom, array);
    return array;
}
/**
 * \brief Duplicates and adds an array to a geometry
 * \param canfree can the data pointer of \p array be freed by
 * SCE_Geometry_Delete() ?
 *
 * Does the same of SCE_Geometry_AddArray() except it duplicates the
 * geometry array before add it. The duplicated array is not authorized
 * to delete the vertex data of \p array (see SCE_Geometry_SetArrayData()).
 * The duplicated array hasn't any user (see SCE_Geometry_AddUser()).
 * \sa SCE_Geometry_AddArray(), SCE_Geometry_SetArrayData()
 */
SCE_SGeometryArray* SCE_Geometry_AddArrayDup (SCE_SGeometry *geom,
                                              SCE_SGeometryArray *array,
                                              int canfree)
{
    SCE_SGeometryArray *new = NULL;
    if (!(new = SCE_Geometry_CreateArray ()))
        SCEE_LogSrc ();
    else {
        SCE_Geometry_CopyArray (new, array);
        new->canfree_data = canfree;
        SCE_Geometry_AddArray (geom, new);
    }
    return new;
}
/**
 * \brief Duplicates and adds an array and all its children
 * \returns the new root array
 *
 * \p canfree counts for all the added array
 * \sa SCE_Geometry_AddArrayRec(), SCE_Geometry_AttachArray()
 */
SCE_SGeometryArray* SCE_Geometry_AddArrayRecDup (SCE_SGeometry *geom,
                                                 SCE_SGeometryArray *array,
                                                 int canfree)
{
    SCE_SGeometryArray *root = NULL, *a = array;
    while (a) {
        SCE_SGeometryArray *new = SCE_Geometry_AddArrayDup (geom, a, canfree);
        if (!new) {
            SCEE_LogSrc ();
            return NULL;
        }
        if (!root)
            root = new;
        else
            SCE_Geometry_AttachArray (root, new);
        a = a->child;
    }
    return root;
}
/**
 * \todo Merge it with SCE_Geometry_SetIndexArrayDupDup() ?
 * \brief Duplicates an array and its vertex data and adds it to a geometry
 * \param array the array to duplicate
 * \param keep keep the data type as in \p array, otherwise they are converted to
 * an optimized data type (SCE_VERTICES_TYPE)
 * \note This function requires that the number of vertices have been yet
 * specified to \p geom (see SCE_Geometry_SetNumVertices()).
 *
 * \sa SCE_Geometry_SetIndexArrayDupDup(), SCE_Geometry_SetData()
 */
SCE_SGeometryArray* SCE_Geometry_AddArrayDupDup (SCE_SGeometry *geom,
                                                 SCE_SGeometryArray *array,
                                                 int keep)
{
    SCE_SGeometryArray *new = NULL;
    void *newdata = NULL;
    SCE_SGeometryArrayData *data;
    SCE_EType type = SCE_VERTICES_TYPE;
    data = SCE_Geometry_GetArrayData (array);
    /* do not convert if the type is the same */
    if (keep || type == data->type) {
        type = data->type;
        newdata = SCE_Mem_Dup (data->data, geom->n_vertices *
                               SCE_Type_Sizeof (data->type) * data->size);
    } else {
        /* TODO: not according to definition of SCEvertices, may be greater */
        newdata = SCE_Type_ConvertDup (type, data->type, data->data,
                                       geom->n_vertices * data->size);
    }
    if (!newdata)
        SCEE_LogSrc ();
    else {
        /* NOTE: it suggests that the vertex array doesn't use 'data' or 'type'
          for any precomputation when set the data (SCE_CSetVertexArrayData())*/
        if (!(new = SCE_Geometry_AddArrayDup (geom, array, SCE_TRUE))) {
            SCEE_LogSrc ();
            SCE_free (newdata);
        } else {
            data = SCE_Geometry_GetArrayData (new);
            /* if the program is interrupted between AddArrayDup() an here,
               a double-free may occur (nocake) */
            data->data = newdata;
            data->type = type;
        }
    }
    return new;
}
/**
 * \brief Duplicates the given geometry arrays and their data and adds them
 * to a geometry
 *
 * \p keep counts for all the children of \p array
 * \sa SCE_Geometry_AddArrayRecDup(), SCE_Geometry_AddArrayRec(),
 * SCE_Geometry_AddArrayDupDup(), SCE_Geometry_AttachArray()
 */
SCE_SGeometryArray* SCE_Geometry_AddArrayRecDupDup (SCE_SGeometry *geom,
                                                    SCE_SGeometryArray *array,
                                                    int keep)
{
    SCE_SGeometryArray *root = NULL, *a = array;
    while (a) {
        SCE_SGeometryArray *new = SCE_Geometry_AddArrayDupDup (geom, a, keep);
        if (!new) {
            SCEE_LogSrc ();
            return NULL;
        }
        if (!root)
            root = new;
        else
            SCE_Geometry_AttachArray (root, new);
    }
    return root;
}
/**
 * \brief Removes an array and all its children from its geometry
 * \sa SCE_Geometry_AddArray(), SCE_Geometry_AddArrayDup(),
 * SCE_Geometry_AddArrayRec()
 */
void SCE_Geometry_RemoveArray (SCE_SGeometryArray *array)
{
    while (array) {
        SCE_List_Remove (&array->it);
        array->geom = NULL;
        array = array->child;
    }
}

/**
 * \brief Sets the index array of a geometry
 * \param array the index array to set, NULL if you want to remove the current
 * one of \p geom
 * \param canfree if SCE_TRUE, SCE_Geometry_Delete() will free \p array
 * \sa SCE_Geometry_AddArray(), SCE_Geometry_SetArrayData()
 */
void SCE_Geometry_SetIndexArray (SCE_SGeometry *geom, SCE_SGeometryArray *array,
                                 int canfree)
{
    SCE_Geometry_DeleteIndexArray (geom);
    geom->index_array = array;
    if (array) {
        geom->index_data = SCE_Geometry_GetData (array);
        geom->canfree_index = canfree;
    } else {
        geom->index_data = NULL;
        geom->canfree_index = SCE_FALSE;
    }
}
/**
 * \brief Duplicates and set an index array
 * \returns the new array, duplicated from \p array
 *
 * Duplicates \p array but not its vertex data. \p canfree indicates if
 * SCE_Geometry_Delete() will free the vertex data of \p array.
 * \sa SCE_Geometry_SetIndexArray(), SCE_Geometry_GetIndexArray()
 */
SCE_SGeometryArray* SCE_Geometry_SetIndexArrayDup (SCE_SGeometry *geom,
                                                   SCE_SGeometryArray *array,
                                                   int canfree)
{
    SCE_SGeometryArray *new = NULL;
    if (!(new = SCE_Geometry_CreateArray ()))
        SCEE_LogSrc ();
    else {
        SCE_Geometry_CopyArray (new, array);
        new->canfree_data = canfree;
        SCE_Geometry_SetIndexArray (geom, new, SCE_TRUE);
    }
    return new;
}
/**
 * \brief Duplicates an index array and its data and set it to a geometry
 * \param array the index array to duplicate
 * \param keep keep data type as in \p array, otherwise they are converted to
 * an optimized data type (SCE_INDICES_TYPE)
 * \note This function requires that the number of indices have been yet
 * specified to \p geom (see SCE_Geometry_SetNumIndices()).
 * \sa SCE_Geomtry_AddArrayDupDup(), SCE_Geometry_SetIndexArrayDup(),
 * SCE_Geometry_SetIndexArray(), SCE_Geometry_SetArraDataDup()
 */
SCE_SGeometryArray* SCE_Geometry_SetIndexArrayDupDup (SCE_SGeometry *geom,
                                                      SCE_SGeometryArray *array,
                                                      int keep)
{
    SCE_SGeometryArray *new = NULL;
    void *newdata = NULL;
    SCE_SGeometryArrayData *data;
    SCEenum type = SCE_INDICES_TYPE;
    data = SCE_Geometry_GetArrayData (array);
    if (keep || type == data->type) {
        type = data->type;
        newdata = SCE_Mem_Dup (data->data, geom->n_indices *
                               SCE_Type_Sizeof (data->type));
    } else {
        /* TODO: not according to definition of SCEindices, may be greater */
        newdata = SCE_Type_ConvertDup (type, data->type, data->data,
                                       geom->n_indices);
    }
    if (!newdata)
        SCEE_LogSrc ();
    else {
        if (!(new = SCE_Geometry_CreateArray ())) {
            SCEE_LogSrc ();
            SCE_free (newdata);
        } else {
            /* vertex array used to contain an index array... very nice. */
            data = SCE_Geometry_GetArrayData (new);
            data->data = newdata;
            data->type = type;
            new->canfree_data = SCE_TRUE;
            SCE_Geometry_SetIndexArray (geom, new, SCE_TRUE);
        }
    }
    return new;
}
/**
 * \brief Gets the index array of a geometry
 * \sa SCE_Geometry_SetIndexArray()
 */
SCE_SGeometryArray* SCE_Geometry_GetIndexArray (SCE_SGeometry *geom)
{
    return geom->index_array;
}


/**
 * \brief User-friendly function to quickly defined data of a geometry
 * \param pos,nor,tex vertex data, can be NULL if non-defined, but \p pos
 * must be given. all these pointer will be freed by SCE_Geometry_Delete().
 * \param index if specified, set as indices of the geometry
 * \returns SCE_ERROR on error, SCE_OK otherwise
 *
 * Considers the size of \p pos, \p nor and \p tex are 3, 3 and 2, respectively.
 * \sa SCE_Geometry_AddArray(), SCE_Geometry_SetIndexArray(),
 * SCE_Geometry_SetArrayData()
 */
int SCE_Geometry_SetData (SCE_SGeometry *geom, SCEvertices *pos,
                          SCEvertices *nor, SCEvertices *tex, SCEindices *index,
                          SCEuint n_vertices, SCEuint n_indices)
{
    int i;
    SCE_SGeometryArray array, *arrays[4];

    for (i = 0; i < 4; i++)
        arrays[i] = NULL;
    i = 0;
    SCE_Geometry_SetNumVertices (geom, n_vertices);
    SCE_Geometry_SetNumIndices (geom, n_indices);

    SCE_Geometry_InitArray (&array);
    SCE_Geometry_SetArrayPosition (&array, 0, 3, pos, SCE_TRUE);
    if (!(arrays[i] = SCE_Geometry_AddArrayDup (geom, &array, SCE_TRUE)))
        goto fail;
    if (nor) {
        i++;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayNormal (&array, 0, nor, SCE_TRUE);
        if (!(arrays[i] = SCE_Geometry_AddArrayDup (geom, &array, SCE_TRUE)))
            goto fail;
    }
    if (tex) {
        i++;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayTexCoord (&array, 0, 0, 2, tex, SCE_TRUE);
        if (!(arrays[i] = SCE_Geometry_AddArrayDup (geom, &array, SCE_TRUE)))
            goto fail;
    }
    if (index) {
        i++;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayIndices (&array, SCE_INDICES_TYPE,
                                      index, SCE_TRUE);
        if (!(arrays[i] = SCE_Geometry_SetIndexArrayDup (geom, &array,
                                                         SCE_TRUE)))
            goto fail;
    }

    return SCE_OK;
fail:
    for (; i >= 0; i--) {
        arrays[i]->canfree_data = SCE_FALSE;
        SCE_Geometry_DeleteArray (arrays[i]);
    }
    SCEE_LogSrc ();
    return SCE_ERROR;
}
/**
 * \brief Does like SCE_Geometry_SetData() but duplicates the data
 */
int SCE_Geometry_SetDataDup (SCE_SGeometry *geom, SCEvertices *pos,
                             SCEvertices *nor, SCEvertices *tex,
                             SCEindices *index, SCEuint n_vertices,
                             SCEuint n_indices)
{
    SCEvertices *newpos = NULL, *newnor = NULL, *newtex = NULL;
    SCEindices *newindex = NULL;
    size_t size = n_vertices * sizeof (SCEvertices);
    if (!(newpos = SCE_Mem_Dup (pos, size * 3)))
        goto fail;
    if (nor) {
        if (!(newnor = SCE_Mem_Dup (nor, size * 3)))
            goto fail;
    }
    if (tex) {
        if (!(newtex = SCE_Mem_Dup (tex, size * 2)))
            goto fail;
    }
    if (index) {
        if (!(newindex = SCE_Mem_Dup (index, n_indices * sizeof (SCEindices))))
            goto fail;
    }
    if (SCE_Geometry_SetData (geom, newpos, newnor, newtex, newindex,
                              n_vertices, n_indices) < 0)
        goto fail;
    return SCE_OK;
fail:
    SCE_free (index);
    SCE_free (tex);
    SCE_free (nor);
    SCE_free (pos);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Gets the array of the vertices' position
 */
SCE_SGeometryArray* SCE_Geometry_GetPositionsArray (SCE_SGeometry *geom)
{
    return geom->pos_array;
}
/**
 * \brief Gets the array of the vertices' normal
 */
SCE_SGeometryArray* SCE_Geometry_GetNormalsArray (SCE_SGeometry *geom)
{
    return geom->nor_array;
}
/**
 * \brief Gets the array of the vertices' texture coordinate
 */
SCE_SGeometryArray* SCE_Geometry_GetTexCoordsArray (SCE_SGeometry *geom)
{
    return geom->tex_array;
}
/**
 * \brief Gets the vertices' position
 */
SCEvertices* SCE_Geometry_GetPositions (SCE_SGeometry *geom)
{
    return geom->pos_data;
}
/**
 * \brief Gets the vertices' normal
 */
SCEvertices* SCE_Geometry_GetNormals (SCE_SGeometry *geom)
{
    return geom->nor_data;
}
/**
 * \brief Gets the vertices' texture coordinates
 */
SCEvertices* SCE_Geometry_GetTexCoords (SCE_SGeometry *geom)
{
    return geom->tex_data;
}
/**
 * \brief Gets the indices
 */
SCEindices* SCE_Geometry_GetIndices (SCE_SGeometry *geom)
{
    return geom->index_data;
}

/**
 * \brief Sets primitive type of a geometry
 * \sa SCE_Geometry_GetPrimitiveType()
 */
void SCE_Geometry_SetPrimitiveType (SCE_SGeometry *geom,
                                    SCE_EPrimitiveType prim)
{
    geom->prim = prim;
}
/**
 * \brief Gets primitive type of a geometry
 * \sa SCE_Geometry_SetPrimitiveType()
 */
SCEenum SCE_Geometry_GetPrimitiveType (SCE_SGeometry *geom)
{
    return geom->prim;
}

/**
 * \brief Sets the number of vertices of a geometry
 * \sa SCE_Geometry_GetNumvertices(), SCE_Geometry_SetNumIndices()
 */
void SCE_Geometry_SetNumVertices (SCE_SGeometry *geom, size_t n_vertices)
{
    geom->n_vertices = n_vertices;
}
/**
 * \brief Sets the number of indices of a geometry
 * \sa SCE_Geometry_GetNumIndices(), SCE_Geometry_SetNumVertices()
 */
void SCE_Geometry_SetNumIndices (SCE_SGeometry *geom, size_t n_indices)
{
    geom->n_indices = n_indices;
}

/**
 * \brief Gets the number of vertices of a geometry
 * \sa SCE_Geometry_SetNumVertices(), SCE_Geometry_GetNumIndices()
 */
size_t SCE_Geometry_GetNumVertices (SCE_SGeometry *geom)
{
    return geom->n_vertices;
}
/**
 * \brief Gets the number of indices of a geometry
 * \sa SCE_Geometry_SetNumIndices(), SCE_Geometry_GetNumVertices()
 */
size_t SCE_Geometry_GetNumIndices (SCE_SGeometry *geom)
{
    return geom->n_indices;
}
/**
 * \brief Gets the number of vertices used for each primitive (or 'face'),
 * 3 for triangles, 2 for lines, etc.
 * \sa SCE_Geometry_GetNumPrimitives()
 */
size_t SCE_Geometry_GetNumVerticesPerPrimitive (SCE_SGeometry *geom)
{
    size_t vpp = 0;
    switch (geom->prim) {
    case SCE_POINTS:         vpp = 1; break;
    case SCE_LINES:          vpp = 2; break;
    case SCE_TRIANGLES:      vpp = 3; break;
#ifdef SCE_DEBUG
        break;
    default:
        SCEE_SendMsg ("unsupported primitive type to compute the "
                      "number of vertices per primitive");
#endif
    }
    return vpp;
}
/**
 * \brief Gets the total number of primitives of a geometry
 * \sa SCE_Geometry_GetNumVerticesPerPrimitive()
 */
size_t SCE_Geometry_GetNumPrimitives (SCE_SGeometry *geom)
{
    if (geom->index_array)
        return geom->n_indices / SCE_Geometry_GetNumVerticesPerPrimitive (geom);
    else
        return geom->n_vertices / SCE_Geometry_GetNumVerticesPerPrimitive(geom);
}

/**
 * \brief Gets the arrays of a geometry (not including those who are modified)
 *
 * To get all the arrays of a geometry, use also those returned by
 * SCE_Geometry_GetModifiedArrays().
 * \sa SCE_Geometry_GetModifiedArrays()
 */
SCE_SList* SCE_Geometry_GetArrays (SCE_SGeometry *geom)
{
    return &geom->arrays;
}
/**
 * \brief Gets the modified arrays of a geometry
 *
 * To get all the arrays of a geometry, use also those returned by
 * SCE_Geometry_GetArrays().
 * \sa SCE_Geometry_GetArrays()
 */
SCE_SList* SCE_Geometry_GetModifiedArrays (SCE_SGeometry *geom)
{
    return &geom->modified;
}

/**
 * \brief Is the given geometry modified since the last update?
 */
int SCE_Geometry_IsModified (SCE_SGeometry *geom)
{
    return SCE_List_HasElements (&geom->modified);
}

/**
 * \brief Loads geometry from a file
 * \param force force a new instance of the resource to be loaded
 * \sa SCE_Geometry_GetResourceType(), SCE_Resource_Load()
 */
SCE_SGeometry* SCE_Geometry_Load (const char *fname, int force)
{
    return SCE_Resource_Load (resource_type, fname, force, NULL);
}

/**
 * \brief Computes a bounding box from a lot of vertices' positions
 * \param v the vertices' positions (must be 3 components vectors)
 * \param vcount the number of vertices into \p v
 * \param box write out the bounding box here
 * \sa SCE_Geometry_GenerateBoundingBox()
 */
void SCE_Geometry_ComputeBoundingBox (SCEvertices *v, size_t vcount,
                                      SCE_SBox *box)
{
    SCE_TVector3 max = {0., 0., 0.};
    SCE_TVector3 min = {0., 0., 0.};
    size_t i, count;

    /* TODO: use a "Rectangle3D" ? */
    count = vcount * 3;
    for (i = 0; i < count; i += 3) {
        SCE_Vector3_GetMin (min, min, &v[i]);
        SCE_Vector3_GetMax (max, max, &v[i]);
    }
    SCE_Box_SetFromMinMax (box, min, max);
}
/**
 * \brief Computes a bounding sphere from a lot of vertices
 * \param v the vertices
 * \param vcount the number of vertices
 * \param box needed to compute the bounding sphere, you can obtain the
 * bounding box using SCE_Geometry_ComputeBoundingBox()
 * \param sphere write out the sphere here
 * \sa SCE_Geometry_ComputeBoundingBox(), SCE_Geometry_GenerateBoundingVolumes()
 */
void SCE_Geometry_ComputeBoundingSphere (SCEvertices *v, size_t vcount,
                                         SCE_SBox *box, SCE_SSphere *sphere)
{
    float d = 0.0f;
    size_t i;
    sphere->radius = 0.0f;
    SCE_Box_GetCenterv (box, sphere->center);
    for (i = 0; i < vcount; i++) {
        d = SCE_Vector3_Distance (sphere->center, &v[i*3]);
        sphere->radius = (sphere->radius < d ? d : sphere->radius);
    }
}

/**
 * \brief Generates the bounding box of a geometry
 * \sa SCE_Geometry_GenerateBoundingVolumes()
 */
void SCE_Geometry_GenerateBoundingBox (SCE_SGeometry *geom)
{
    if (!geom->box_uptodate) {
        SCE_Geometry_ComputeBoundingBox (geom->pos_data, geom->n_vertices,
                                         &geom->box);
        geom->box_uptodate = SCE_TRUE;
    }
}
/**
 * \brief Generates the bounding sphere of a geometry
 * \sa SCE_Geometry_GenerateBoundingVolumes()
 */
void SCE_Geometry_GenerateBoundingSphere (SCE_SGeometry *geom)
{
    if (!geom->sphere_uptodate) {
        SCE_Geometry_GenerateBoundingBox (geom);
        SCE_Geometry_ComputeBoundingSphere (geom->pos_data, geom->n_vertices,
                                            &geom->box, &geom->sphere);
        geom->sphere_uptodate = SCE_TRUE;
    }
}
/**
 * \brief Generates or update the bounding volumes of a geometry
 * \sa SCE_Geometry_GenerateBoundingBox(), SCE_Geometry_GenerateBoundingSphere()
 */
void SCE_Geometry_GenerateBoundingVolumes (SCE_SGeometry *geom)
{
    SCE_Geometry_GenerateBoundingSphere (geom);
    /* TODO: hey what about bounding boxes? */
}

/**
 * \brief Gets the box of a geometry
 * \sa SCE_Geometry_GetSphere(), SCE_Geometry_GenerateBoundingBox()
 */
SCE_SBox* SCE_Geometry_GetBox (SCE_SGeometry *geom)
{
    return &geom->box;
}
/**
 * \brief Gets the sphere of a geometry
 * \sa SCE_Geometry_GetBox(), SCE_Geometry_GenerateBoundingSphere()
 */
SCE_SSphere* SCE_Geometry_GetSphere (SCE_SGeometry *geom)
{
    return &geom->sphere;
}

/**
 * \brief Marks the box of a geometry as up to date
 * \sa SCE_Geometry_SphereUpToDate()
 * \todo what about marking it as non up-to-date?
 */
void SCE_Geometry_BoxUpToDate (SCE_SGeometry *geom)
{
    geom->box_uptodate = SCE_TRUE;
}
/**
 * \brief Marks the sphere of a geometry as up to date
 * \sa SCE_Geometry_BoxUpToDate()
 * \todo what about marking it as non up-to-date?
 */
void SCE_Geometry_SphereUpToDate (SCE_SGeometry *geom)
{
    geom->sphere_uptodate = SCE_TRUE;
}


/* bonus functions */
static int SCE_Geometry_UpdatePrimArray (SCE_SGeometry *geom, size_t vpp,
                                         size_t n_prim)
{
    if (geom->sorted_length != n_prim) {
        /* update the sorted array */
        SCE_free (geom->sorted);
        geom->sorted = SCE_malloc (n_prim * sizeof *geom->sorted);
        if (!geom->sorted) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
        geom->sorted_length = n_prim;
    }
    return SCE_OK;
}
static void SCE_Geometry_GetPrimCenter (SCE_TVector3 v[4], size_t vpp,
                                        SCE_TVector3 c)
{
    size_t i;
    float invvpp = 1.0f / vpp;
    SCE_Vector3_Copy (c, v[0]);
    for (i = 1; i < vpp; i++)
        SCE_Vector3_Operator1v (c, +=, v[i]);
    SCE_Vector3_Operator1 (c, *=, invvpp);
}
static void SCE_Geometry_ComputePrimDistances (SCE_SGeometry *geom,
                                               size_t vpp, SCE_TVector3 from)
{
    size_t i;
    SCE_TVector3 center;
    SCEvertices vprim[4][3];
    SCEindices *indices;
    SCEvertices *positions = geom->pos_data;
    indices = geom->index_data;
    for (i = 0; i < geom->sorted_length; i++) {
        size_t j;
        for (j = 0; j < vpp; j++)
            SCE_Vector3_Copy (&vprim[j], &positions[indices[(i * vpp) + j]]);
        SCE_Geometry_GetPrimCenter (vprim, vpp, center);
        geom->sorted[i].dist = SCE_Vector3_Distance (center, from);
        geom->sorted[i].index = i * vpp;
    }
}
static int SCE_Geometry_FarToNear (float first, float last)
{
    return (first < last);
}
static int SCE_Geometry_NearToFar (float first, float last)
{
    return (first > last);
}
static void SCE_Geometry_SortPrimArray (SCE_SGeometry *geom,
                                        SCE_ESortOrder order)
{
    size_t i;
    int (*xch)(float, float);
    xch = (order == SCE_SORT_FAR_TO_NEAR ?
           SCE_Geometry_FarToNear : SCE_Geometry_NearToFar);
    for (i = 0; i < geom->sorted_length; i++) {
        size_t j;
        for (j = 0; j < i; j++) {
            if (xch (geom->sorted[j].dist, geom->sorted[i].dist)) {
                SCE_SGeometryPrimitiveSort s = geom->sorted[i];
                geom->sorted[i] = geom->sorted[j];
                geom->sorted[j] = s;
            }
        }
    }
}
static void SCE_Geometry_ExchangeIndices (SCEindices *i1, SCEindices *i2,
                                          size_t vpp)
{
    SCEindices tmp[4];
    memcpy (tmp, i1, vpp * sizeof *tmp);
    memcpy (i1, i2, vpp * sizeof *i1);
    memcpy (i2, tmp, vpp * sizeof *i2);
}
/**
 * \brief Sort the primitives of a geometry
 * \param order order of sorting
 * \param from from which sort the distance of the primitives
 * \returns SCE_ERROR on error, SCE_OK otherwise
 */
int SCE_Geometry_SortPrimitives (SCE_SGeometry *geom, SCE_ESortOrder order,
                                 SCE_TVector3 from)
{
    size_t i, vpp, n_prim;
    SCEindices *indices = NULL;

#ifdef SCE_DEBUG
    /* TODO: use auto-generation of pseudo-indices (0, 1, 2, 3, ...) */
    if (!geom->index_array) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("cannot sort a geometry without indices");
        return SCE_ERROR;
    }
#endif
    indices = geom->index_data;
    vpp = SCE_Geometry_GetNumVerticesPerPrimitive (geom);
    n_prim = SCE_Geometry_GetNumPrimitives (geom);

    if (SCE_Geometry_UpdatePrimArray (geom, vpp, n_prim) < 0)
        goto fail;
    SCE_Geometry_ComputePrimDistances (geom, vpp, from);
    SCE_Geometry_SortPrimArray (geom, order);

    /* reset indices */
    for (i = 0; i < geom->sorted_length; i++) {
        SCE_Geometry_ExchangeIndices (&indices[i * vpp],
                                      &indices[geom->sorted[i].index], vpp);
    }
    /* TODO: how to set a good range? */
    SCE_Geometry_Modified (geom->index_array, NULL);
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


/**
 * \brief Compute the tangent, binormal and normal for a triangle
 * \param vertex the vertices' positions of the triangle
 * \param texcoord the texture coordinates of the triangle, can be NULL
 * \param index face's indices to use, can be NULL
 * \param tangents here is stored the computed tangents, can be NULL
 * \param binormals here is stored the computed binormals, can be NULL
 * \param normals here is written the computed normals, can be NULL
 * \warning If \p texcoord is NULL, only normals will be computed
 */
void SCE_Mesh_ComputeTriangleTBN (SCEvertices *vertex, SCEvertices *texcoord,
                                  size_t *index, SCEvertices *tangents,
                                  SCEvertices *binormals, SCEvertices *normals)
{
    SCE_TVector3 side0, side1;
    SCE_TVector3 tmpTangente, tmpNormal, tmpBinormal;
    SCEvertices deltaT0=0., deltaT1=0., deltaB0=0., deltaB1=0., scale=0.;
    size_t default_indices[3] = {0, 1, 2};

    if (!index)
        index = default_indices;

    /* on calcule 2 vecteurs formant les bords du triangle
       le vecteur side0 est celui allant du vertex 0 au vertex 1 */
    SCE_Vector3_Operator2v (side0, =, &vertex[index[1]*3],
                                   -, &vertex[index[0]*3]);
    /* le vecteur side1 est celui allant du vertex 0 au vertex 2 */
    SCE_Vector3_Operator2v (side1, =, &vertex[index[2]*3],
                                   -, &vertex[index[0]*3]);

    if (texcoord) {
        /* ici, +1 pour prendre la composante v (uv) */
        deltaT0 = texcoord[index[1]*2+1] - texcoord[index[0]*2+1];
        deltaT1 = texcoord[index[2]*2+1] - texcoord[index[0]*2+1];
        /* composante u */
        deltaB0 = texcoord[index[1]*2] - texcoord[index[0]*2];
        deltaB1 = texcoord[index[2]*2] - texcoord[index[0]*2];
        scale = 1.0f / ((deltaB0 * deltaT1) - (deltaB1 * deltaT0));
    }
    if (normals) {
        SCE_Vector3_Cross (tmpNormal, side0, side1);
        SCE_Vector3_Normalize (tmpNormal);
        SCE_Vector3_Operator1v (&normals[index[0]*3], +=, tmpNormal);
        SCE_Vector3_Operator1v (&normals[index[1]*3], +=, tmpNormal);
        SCE_Vector3_Operator1v (&normals[index[2]*3], +=, tmpNormal);
    }
    if (texcoord && tangents) {
        SCE_Vector3_Operator1v (tmpTangente, = deltaT1*, side0);
        SCE_Vector3_Operator1v (tmpTangente, -= deltaT0*, side1);
        SCE_Vector3_Operator1 (tmpTangente, *=, scale);
        SCE_Vector3_Normalize (tmpTangente);
        SCE_Vector3_Operator1v (&tangents[index[0]*3], +=, tmpTangente);
        SCE_Vector3_Operator1v (&tangents[index[1]*3], +=, tmpTangente);
        SCE_Vector3_Operator1v (&tangents[index[2]*3], +=, tmpTangente);
    }
    if (texcoord && binormals) {
        SCE_Vector3_Operator1v (tmpBinormal, = -deltaB1*, side0);
        SCE_Vector3_Operator1v (tmpBinormal, += deltaB0*, side1);
        SCE_Vector3_Operator1 (tmpBinormal, *=, scale);
        SCE_Vector3_Normalize (tmpBinormal);
        SCE_Vector3_Operator1v (&binormals[index[0]*3], +=, tmpBinormal);
        SCE_Vector3_Operator1v (&binormals[index[1]*3], +=, tmpBinormal);
        SCE_Vector3_Operator1v (&binormals[index[2]*3], +=, tmpBinormal);
    }
}


/**
 * \brief Compute the tangent, binormal and normal for a lot of polygons
 * \param itype type of \p indices
 * \param icount number of vertices that \indices dereference
 * \param vcount number of vertices that are availables in
 * \p vertex and \p texcoord
 * \returns SCE_ERROR on error, SCE_OK otherwise
 *
 * This function works like SCE_Mesh_ComputeTriangleTBN(), but on a lot of
 * polygons.
 */
int SCE_Geometry_ComputeTBN (SCE_EPrimitiveType prim, SCEvertices *vertex,
                             SCEvertices *texcoord, SCE_EType itype,
                             void *indices, size_t icount, size_t vcount,
                             SCEvertices *tangents, SCEvertices *binormals,
                             SCEvertices *normals)
{
    size_t i;
    size_t t_indices[3];
    SCEindices *index = NULL;
    size_t count;

#if 0
    if (normals)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Set (&normals[i*3], 0.0f, 0.0f, 0.0f);
    if (binormals)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Set (&binormals[i*3], 0.0f, 0.0f, 0.0f);
    if (tangents)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Set (&tangents[i*3], 0.0f, 0.0f, 0.0f);
#endif

    if (!indices) {
        count = vcount;
        itype = SCE_INDICES_TYPE;
        index = SCE_malloc (sizeof *index * count);
        if (!index) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
        for (i = 0; i < count; i++)
            index[i] = i;
    } else {
        index = indices;
        count = icount;
    }

    switch (prim) {
    case SCE_TRIANGLES:
        for (i = 0; i < count; i += 3) {
            SCE_Type_Convert (SCE_SIZE_T, t_indices, itype, &index[i], 3);
            SCE_Mesh_ComputeTriangleTBN (vertex, texcoord, t_indices,
                                         tangents, binormals, normals);
        }
        break;
    case SCE_TRIANGLE_STRIP:
        for (i = 0; i < count - 2; i++) {
            SCE_Type_Convert (SCE_SIZE_T, t_indices, itype, &index[i], 3);
            SCE_Mesh_ComputeTriangleTBN (vertex, texcoord, t_indices,
                                         tangents, binormals, normals);
        }
        break;
    case SCE_TRIANGLE_FAN:
        SCE_Type_Convert (SCE_SIZE_T, t_indices, itype, index, 1);
        for (i = 1; i < count - 1; i++) {
            SCE_Type_Convert (SCE_SIZE_T, &t_indices[1], itype, &index[i], 2);
            SCE_Mesh_ComputeTriangleTBN (vertex, texcoord, t_indices,
                                         tangents, binormals, normals);
        }
        break;
    default:
        if (!indices)
            SCE_free (index);
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("primitive type unsupported, you must choose one"
                     " of the following types: SCE_TRIANGLES, "
                     "SCE_TRIANGLE_STRIP or SCE_TRIANGLE_FAN");
        return SCE_ERROR;
    }

    if (!indices)
        SCE_free (index);

    if (normals)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Normalize (&normals[i*3]);
    if (binormals)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Normalize (&binormals[i*3]);
    if (tangents)
        for (i = 0; i < vcount; i++)
            SCE_Vector3_Normalize (&tangents[i*3]);

    return SCE_OK;
}

/**
 * \brief Generates the tangent, binormal an normal vectors for a geometry
 * \param nor,offset allow to specify one normal... nice.
 */
int SCE_Geometry_GenerateTBN (SCE_SGeometry *geom, SCEvertices **t,
                              SCEvertices **b, SCEvertices **n,
                              unsigned int unit)/*,
                              SCEvertices *nor, size_t offset)*/
{
    SCE_SGeometryArrayData *data;
    SCEvertices *tangent = NULL, *binormal = NULL, *normal = NULL;

#ifdef SCE_DEBUG
    /* TODO: check the type of the data too */
    if (!geom->pos_data) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("the geometry on which generate TBN vectors must have"
                     " position data");
        return SCE_ERROR;
    }
    /* TODO: oh, what if \p unit is != of 0 ? tex_data is only SCE_TEXCOORD0 */
    if (!geom->tex_data && (t || b)) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("generate tangent and/or binormals requires texture "
                     "coordinates");
        return SCE_ERROR;
    }
#endif

    if (t) {
        if (!(tangent = SCE_malloc (geom->n_vertices * sizeof *tangent)))
            goto fail;
    }
    if (b) {
        if (!(binormal = SCE_malloc (geom->n_vertices * sizeof *binormal)))
            goto fail;
    }
    if (n) {
        if (!(normal = SCE_malloc (geom->n_vertices * sizeof *normal)))
            goto fail;
    }

    data = SCE_Geometry_GetArrayData (geom->index_array);
    if (SCE_Geometry_ComputeTBN (geom->prim, geom->pos_data, geom->tex_data,
                                 data->type, data->data, geom->n_indices,
                                 geom->n_vertices, tangent, binormal,
                                 normal) < 0)
        goto fail;

    if (t)
        *t = tangent;
    if (b)
        *b = binormal;
    if (n)
        *n = normal;
    return SCE_OK;
fail:
    SCE_free (normal);
    SCE_free (binormal);
    SCE_free (tangent);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Generates and adds vertex attributes
 * \sa SCE_Geometry_GenerateTBN()
 */
int SCE_Geometry_AddGenerateTBN (SCE_SGeometry *geom, unsigned int unit,
                                 int flags)
{
    SCEvertices **param[3];
    SCEvertices *t = NULL, *b = NULL, *n = NULL;

    param[0] = (flags & SCE_GEN_TANGENTS ? &t : NULL);
    param[1] = (flags & SCE_GEN_BINORMALS ? &b : NULL);
    param[2] = (flags & SCE_GEN_NORMALS ? &n : NULL);
    if (SCE_Geometry_GenerateTBN (geom, param[0], param[1], param[0], unit) < 0)
        goto fail;
    if (t) {
        if (!SCE_Geometry_AddNewArray (geom, SCE_TANGENT, SCE_VERTICES_TYPE,
                                       0, 3, t, SCE_TRUE))
            goto fail;
    }
    if (b) {
        if (!SCE_Geometry_AddNewArray (geom, SCE_BINORMAL, SCE_VERTICES_TYPE,
                                       0, 3, b, SCE_TRUE))
            goto fail;
    }
    if (n) {
        if (!SCE_Geometry_AddNewArray (geom, SCE_NORMAL, SCE_VERTICES_TYPE,
                                       0, 3, n, SCE_TRUE))
            goto fail;
    }
    return SCE_OK;
fail:
    SCE_free (n);
    SCE_free (b);
    SCE_free (t);
    SCEE_LogSrc ();
    return SCE_ERROR;
}

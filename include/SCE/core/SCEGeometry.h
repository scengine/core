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

/* created: 25/07/2009
   updated: 02/02/2012 */

#ifndef SCEGEOMETRY_H
#define SCEGEOMETRY_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEBox.h"
#include "SCE/core/SCESphere.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup geometry
 * @{
 */

  /* TODO: add them to the EVertexAttribute enum */
#define SCE_TANGENT SCE_TEXCOORD1
#define SCE_BINORMAL SCE_TEXCOORD2

#define SCE_GEN_TANGENTS (0x00000001)
#define SCE_GEN_BINORMALS (0x00000002)
#define SCE_GEN_NORMALS (0x00000004)

enum sce_esortorder {
    SCE_SORT_NEAR_TO_FAR,
    SCE_SORT_FAR_TO_NEAR
};
typedef enum sce_esortorder SCE_ESortOrder;

/**
 * \brief Default (and HIGHLY recommanded for compatibility reasons) vertices
 * data type
 */
typedef SCEfloat SCEvertices;
/**
 * \brief Default (and HIGHLY recommanded for compatibility reasons) vertices
 * data type
 */
#define SCE_VERTICES_TYPE SCE_FLOAT

/**
 * \brief Default (and HIGHLY recommanded for compatibility reasons) indices
 * data type
 */
typedef SCEushort SCEindices;
/**
 * \brief Default (and HIGHLY recommanded for compatibility reasons) indices
 * data type
 */
#define SCE_INDICES_TYPE SCE_UNSIGNED_SHORT

/**
 * \brief Primitive types
 */
enum sce_eprimitivetype {
    SCE_POINTS = 0,
    SCE_LINES,
    SCE_LINE_STRIP,
    SCE_TRIANGLES,
    /* those are not recommanded: */
    SCE_TRIANGLE_STRIP,
    SCE_TRIANGLE_FAN,
    SCE_NUM_PRIMITIVE_TYPES
};
typedef enum sce_eprimitivetype SCE_EPrimitiveType;

/**
 * \brief Vertex attributes
 */
enum sce_evertexattribute {
    /* these are used by the geometry manager but shall be re-interpreted
       by the renderer to assign the good vertex attribute */
    SCE_POSITION = 1,
    SCE_COLOR,
    SCE_NORMAL,
    /* integer (only works when using shader attributes mapping) */
    SCE_IPOSITION,
    SCE_ICOLOR,
    SCE_INORMAL,

    /* TODO: add integers for those */
    SCE_TEXCOORD0,
    SCE_TEXCOORD1,
    SCE_TEXCOORD2,
    SCE_TEXCOORD3,
    SCE_TEXCOORD4,
    SCE_TEXCOORD5,
    SCE_TEXCOORD6,
    SCE_TEXCOORD7,

    /* these are the real attributes used by the renderer, but the geometry
       manager won't be able to use them to perform geometric transformations */
    SCE_ATTRIB0,
    SCE_ATTRIB1,
    SCE_ATTRIB2,
    SCE_ATTRIB3,
    SCE_ATTRIB4,
    SCE_ATTRIB5,
    SCE_ATTRIB6,
    SCE_ATTRIB7,
    SCE_ATTRIB8,
    SCE_ATTRIB9,
    SCE_ATTRIB10,
    SCE_ATTRIB11,
    SCE_ATTRIB12,
    SCE_ATTRIB13,
    SCE_ATTRIB14,
    SCE_ATTRIB15,

    /* integer attributes */
    SCE_IATTRIB0,
    SCE_IATTRIB1,
    SCE_IATTRIB2,
    SCE_IATTRIB3,
    SCE_IATTRIB4,
    SCE_IATTRIB5,
    SCE_IATTRIB6,
    SCE_IATTRIB7,
    SCE_IATTRIB8,
    SCE_IATTRIB9,
    SCE_IATTRIB10,
    SCE_IATTRIB11,
    SCE_IATTRIB12,
    SCE_IATTRIB13,
    SCE_IATTRIB14,
    SCE_IATTRIB15,

    SCE_NUM_VERTEX_ATTRIBUTES
};
/** \copydoc sce_evertexattribute */
typedef enum sce_evertexattribute SCE_EVertexAttribute;


/** \copydoc sce_sgeometryarraydata */
typedef struct sce_sgeometryarraydata SCE_SGeometryArrayData;
struct sce_sgeometryarraydata {
    SCE_EVertexAttribute attrib; /**< Vertices' attribute */
    SCE_EType type;       /**< Data type (SCE_FLOAT, SCE_INT, ...) */
    SCEsizei stride;      /**< Stride between two consecutive vertices */
    SCEint size;          /**< Number of dimensions of the vectors */
    void *data;           /**< User is always the owner of the data */
};

/** \copydoc sce_sgeometry */
typedef struct sce_sgeometry SCE_SGeometry;
/** \copydoc sce_sgeometryarray */
typedef struct sce_sgeometryarray SCE_SGeometryArray;
/**
 * \brief A geometry array
 */
struct sce_sgeometryarray {
    SCE_SGeometryArrayData data; /**< Associated data */
    SCE_SGeometryArray *root, *child; /**< Used for interleaved arrays */
    int canfree_data;         /**< Can this structure free \c array.data.data?*/
    SCE_SListIterator it;     /**< Own iterator */
    SCE_SList users;          /**< SCE_SGeometryArrayUser */
    size_t range[2];          /**< Range of modified vertices*/
    size_t *rangeptr;         /**< Pointer to the range to use (can be NULL) */
    SCE_SGeometry *geom;
};

/** \copydoc sce_sgeometryarrayuser */
typedef struct sce_sgeometryarrayuser SCE_SGeometryArrayUser;
/**
 * \brief Prototype of the called callbacks when an array is updated
 * (happens when its geometry is updated)
 * \sa SCE_SGeometryArrayUser, SCE_Geometry_Update()
 */
typedef void (*SCE_FUpdateGeometryArray)(void*, size_t*);
/**
 * \brief User of a geometry
 * \sa SCE_SGeometryArray SCE_FUpdatedGeometryArray
 */
struct sce_sgeometryarrayuser {
    SCE_SGeometryArray *array;       /**< Attached array */
    SCE_FUpdateGeometryArray update; /**< Called on each modified array */
    void *arg;                       /**< Argument of \c update */
    SCE_SListIterator it;
};


typedef struct sce_sgeometryprimitivesort SCE_SGeometryPrimitiveSort;
/**
 * \brief Used to define a primitive in a sort algorithm case
 */
struct sce_sgeometryprimitivesort {
    float dist;
    SCEindices index;
};
/**
 * \brief Contains geometry of a mesh
 * \sa SCE_SGeometryArray, SCE_SGeometryArrayUser, SCE_SMesh
 */
struct sce_sgeometry {
    SCE_EPrimitiveType prim;
    SCE_SList arrays;                 /**< All vertex arrays */
    SCE_SList modified;               /**< Modified vertex arrays */
    SCE_SGeometryArray *index_array;  /**< Index array */
    int canfree_index;
    size_t n_vertices, n_indices;

    SCE_SGeometryArray *pos_array, *nor_array, *tex_array;
    SCEvertices *pos_data, *nor_data, *tex_data;
    SCEindices *index_data;

    SCE_SGeometryPrimitiveSort *sorted;
    size_t sorted_length;

    SCE_SBox box;
    SCE_SSphere sphere;
    int box_uptodate, sphere_uptodate; /* Bounding volumes state */
};

/** @} */

int SCE_Init_Geometry (void);
void SCE_Quit_Geometry (void);

int SCE_Geometry_GetResourceType (void);

void SCE_Geometry_InitArrayData (SCE_SGeometryArrayData*);
SCE_SGeometryArrayData* SCE_Geometry_CreateArrayData (void);
void SCE_Geometry_DeleteArrayData (SCE_SGeometryArrayData*);

void SCE_Geometry_InitArray (SCE_SGeometryArray*);
SCE_SGeometryArray* SCE_Geometry_CreateArray (void);
SCE_SGeometryArray* SCE_Geometry_CreateArrayFrom (SCE_EVertexAttribute,
                                                  SCE_EType, size_t, int,
                                                  void*, int);
void SCE_Geometry_DeleteArray (SCE_SGeometryArray*);
void SCE_Geometry_CopyArray (SCE_SGeometryArray*, const SCE_SGeometryArray*);

void SCE_Geometry_InitArrayUser (SCE_SGeometryArrayUser*);
SCE_SGeometryArrayUser* SCE_Geometry_CreateArrayUser (void);
void SCE_Geometry_ClearArrayUser (SCE_SGeometryArrayUser*);
void SCE_Geometry_DeleteArrayUser (SCE_SGeometryArrayUser*);

void SCE_Geometry_Init (SCE_SGeometry*);
void SCE_Geometry_Clear (SCE_SGeometry*);
SCE_SGeometry* SCE_Geometry_Create (void);
void SCE_Geometry_Delete (SCE_SGeometry*);

SCE_SGeometryArray* SCE_Geometry_GetUserArray (SCE_SGeometryArrayUser*);
void SCE_Geometry_AttachArray (SCE_SGeometryArray*, SCE_SGeometryArray*);
SCE_SGeometryArray* SCE_Geometry_GetRoot (SCE_SGeometryArray*);
SCE_SGeometryArray* SCE_Geometry_GetChild (SCE_SGeometryArray*);

void SCE_Geometry_AddUser (SCE_SGeometryArray*, SCE_SGeometryArrayUser*,
                           SCE_FUpdateGeometryArray, void*);
void SCE_Geometry_RemoveUser (SCE_SGeometryArrayUser*);

void SCE_Geometry_Modified (SCE_SGeometryArray*, const size_t*);
void SCE_Geometry_Unmodified (SCE_SGeometryArray*);
void SCE_Geometry_UpdateArray (SCE_SGeometryArray*);
void SCE_Geometry_Update (SCE_SGeometry*);

void SCE_Geometry_SetArrayData (SCE_SGeometryArray*, SCE_EVertexAttribute,
                                SCE_EType, size_t, int, void*, int);
void SCE_Geometry_SetArrayPosition (SCE_SGeometryArray*, size_t, int,
                                    SCEvertices*, int);
void SCE_Geometry_SetArrayTexCoord (SCE_SGeometryArray*, unsigned int, size_t,
                                    int, SCEvertices*, int);
void SCE_Geometry_SetArrayNormal (SCE_SGeometryArray*, size_t, SCEvertices*,
                                  int);
void SCE_Geometry_SetArrayTangent (SCE_SGeometryArray*, size_t, SCEvertices*,
                                   int);
void SCE_Geometry_SetArrayBinormal (SCE_SGeometryArray*, size_t, SCEvertices*,
                                    int);
void SCE_Geometry_SetArrayIndices (SCE_SGeometryArray*, SCE_EType, void*, int);

void* SCE_Geometry_GetData (SCE_SGeometryArray*);
SCE_EVertexAttribute
SCE_Geometry_GetArrayVertexAttribute (SCE_SGeometryArray*);
SCE_SGeometryArrayData* SCE_Geometry_GetArrayData (SCE_SGeometryArray*);

void SCE_Geometry_AddArray (SCE_SGeometry*, SCE_SGeometryArray*);
void SCE_Geometry_AddArrayRec (SCE_SGeometry*, SCE_SGeometryArray*);
SCE_SGeometryArray*
SCE_Geometry_AddNewArray (SCE_SGeometry*, SCE_EVertexAttribute,
                          SCE_EType, size_t, int, void*, int);
SCE_SGeometryArray* SCE_Geometry_AddArrayDup (SCE_SGeometry*,
                                              SCE_SGeometryArray*, int);
SCE_SGeometryArray* SCE_Geometry_AddArrayRecDup (SCE_SGeometry*,
                                                 SCE_SGeometryArray*,
                                                 int);
SCE_SGeometryArray* SCE_Geometry_AddArrayDupDup (SCE_SGeometry*,
                                                 SCE_SGeometryArray*, int);
SCE_SGeometryArray* SCE_Geometry_AddArrayRecDupDup (SCE_SGeometry*,
                                                    SCE_SGeometryArray*,
                                                    int);
void SCE_Geometry_RemoveArray (SCE_SGeometryArray*);

void SCE_Geometry_SetIndexArray (SCE_SGeometry*, SCE_SGeometryArray*, int);
SCE_SGeometryArray* SCE_Geometry_SetIndexArrayDup (SCE_SGeometry*,
                                                   SCE_SGeometryArray*, int);
SCE_SGeometryArray* SCE_Geometry_SetIndexArrayDupDup (SCE_SGeometry*,
                                                      SCE_SGeometryArray*, int);
SCE_SGeometryArray* SCE_Geometry_GetIndexArray (SCE_SGeometry*);

int SCE_Geometry_SetData (SCE_SGeometry*, SCEvertices*, SCEvertices*,
                          SCEvertices*, SCEindices*, SCEuint, SCEuint);
int SCE_Geometry_SetDataDup (SCE_SGeometry*, SCEvertices*, SCEvertices*,
                             SCEvertices*, SCEindices*, SCEuint, SCEuint);

SCE_SGeometryArray* SCE_Geometry_GetPositionsArray (SCE_SGeometry*);
SCE_SGeometryArray* SCE_Geometry_GetNormalsArray (SCE_SGeometry*);
SCE_SGeometryArray* SCE_Geometry_GetTexCoordsArray (SCE_SGeometry*);
SCEvertices* SCE_Geometry_GetPositions (SCE_SGeometry*);
SCEvertices* SCE_Geometry_GetNormals (SCE_SGeometry*);
SCEvertices* SCE_Geometry_GetTexCoords (SCE_SGeometry*);
SCEindices* SCE_Geometry_GetIndices (SCE_SGeometry*);

void SCE_Geometry_SetPrimitiveType (SCE_SGeometry*, SCE_EPrimitiveType);
SCEenum SCE_Geometry_GetPrimitiveType (SCE_SGeometry*);

void SCE_Geometry_SetNumVertices (SCE_SGeometry*, size_t);
void SCE_Geometry_SetNumIndices (SCE_SGeometry*, size_t);

size_t SCE_Geometry_GetNumVertices (SCE_SGeometry*);
size_t SCE_Geometry_GetNumIndices (SCE_SGeometry*);
size_t SCE_Geometry_GetPrimitiveVertices (SCE_EPrimitiveType);
size_t SCE_Geometry_GetNumVerticesPerPrimitive (SCE_SGeometry*);
size_t SCE_Geometry_GetNumPrimitives (SCE_SGeometry*);
size_t SCE_Geometry_GetTotalStride (SCE_SGeometryArray*);

SCE_SList* SCE_Geometry_GetArrays (SCE_SGeometry*);
SCE_SList* SCE_Geometry_GetModifiedArrays (SCE_SGeometry*);

int SCE_Geometry_IsModified (SCE_SGeometry*);

SCE_SGeometry* SCE_Geometry_Load (const char*, int);

void SCE_Geometry_ComputeBoundingBox (SCEvertices*, size_t, size_t, SCE_SBox*);
void SCE_Geometry_ComputeBoundingSphere (SCEvertices*, size_t, size_t,
                                         SCE_SBox*, SCE_SSphere*);
void SCE_Geometry_GenerateBoundingBox (SCE_SGeometry*);
void SCE_Geometry_GenerateBoundingSphere (SCE_SGeometry*);
void SCE_Geometry_GenerateBoundingVolumes (SCE_SGeometry*);

SCE_SBox* SCE_Geometry_GetBox (SCE_SGeometry*);
SCE_SSphere* SCE_Geometry_GetSphere (SCE_SGeometry*);
void SCE_Geometry_BoxUpToDate (SCE_SGeometry*);
void SCE_Geometry_SphereUpToDate (SCE_SGeometry*);

/* bonus functions */
typedef int (*SCE_FGeometryForEach)(SCE_TVector3, SCE_TVector3, SCE_TVector3,
                                    SCEindices, void*);

void SCE_Geometry_ForEachTriangle (SCE_SGeometry*, SCE_FGeometryForEach, void*);

int SCE_Geometry_SortPrimitives (SCE_SGeometry*, SCE_ESortOrder, SCE_TVector3);

void SCE_Mesh_ComputeTriangleTBN (SCEvertices*, SCEvertices*, size_t*,
                                  SCEvertices*, SCEvertices*, SCEvertices*);
int SCE_Geometry_ComputeTBN (SCE_EPrimitiveType, SCEvertices*, SCEvertices*,
                             SCE_EType, void*, size_t, size_t, SCEvertices*,
                             SCEvertices*, SCEvertices*);
int SCE_Geometry_GenerateTBN (SCE_SGeometry*, SCEvertices**, SCEvertices**,
                              SCEvertices**, unsigned int);
int SCE_Geometry_AddGenerateTBN (SCE_SGeometry*, unsigned int, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

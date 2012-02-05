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

/* created: 23/01/2012
   updated: 05/02/2012 */

#ifndef SCETEXTUREDATA_H
#define SCETEXTUREDATA_H

#include "SCE/core/SCEImage.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \copydoc sce_stexdata */
typedef struct sce_stexdata SCE_STexData;
/**
 * \brief Texture data
 * 
 * Contains all the data of a texture
 */
struct sce_stexdata {
    SCE_SImage *img;  /**< The image of this texture data */
    int canfree;      /**< Do we have rights to delete \c img ? */
    int user;         /**< Is user the owner of this structure ? */
    SCEenum target;   /**< Target of this texture data
                       *   (internal use by the renderer) */
    SCE_EImageType type;     /**< Texture type (dimension) */
    int level;        /**< Mipmap level of this texture data */
    int w, h, d;      /**< Image's dimensions */
    SCE_EPixelFormat pxf; /**< Desired internal (in VRAM) pixel format */
    SCE_EImageFormat data_fmt;  /**< Format of \c data */
    SCE_EType data_type; /**< Type of \c data (SCE_UNSIGNED_BYTE, ...) */
    size_t data_size; /**< Size of \c data in bytes */
    int data_user;    /**< Is user the owner of \c data ? */
    void *data;       /**< Images's raw data */
    int comp;         /**< Is \c pxf a compressed pixel format ? */

    int modified;
    int mod_x, mod_y, mod_z;    /**< Modified zone offset */
    int mod_w, mod_h, mod_d;    /**< Modified zone offset */

    SCE_SListIterator it;
    int doo;
};

void SCE_TexData_Init (SCE_STexData*);
void SCE_TexData_Clear (SCE_STexData*);
SCE_STexData* SCE_TexData_Create (void);
SCE_STexData* SCE_TexData_CreateFromImage (SCE_SImage*, int);
void SCE_TexData_Delete (SCE_STexData*);

SCE_STexData* SCE_TexData_Dup (const SCE_STexData*);

int SCE_TexData_SetImage (SCE_STexData*, SCE_SImage*, int);
void SCE_TexData_SetTarget (SCE_STexData*, SCEenum);
void SCE_TexData_SetType (SCE_STexData*, SCE_EImageType);
void SCE_TexData_SetMipmapLevel (SCE_STexData*, int);
void SCE_TexData_SetDimensions (SCE_STexData*, int, int, int);
void SCE_TexData_SetWidth (SCE_STexData*, int);
void SCE_TexData_SetHeight (SCE_STexData*, int);
void SCE_TexData_SetDepth (SCE_STexData*, int);
void SCE_TexData_SetPixelFormat (SCE_STexData*, SCE_EPixelFormat);
void SCE_TexData_SetDataFormat (SCE_STexData*, SCE_EImageFormat);
void SCE_TexData_SetDataType (SCE_STexData*, SCE_EType);
void SCE_TexData_SetData (SCE_STexData*, void*, int);

SCE_SImage* SCE_TexData_GetImage (SCE_STexData*);
SCEenum SCE_TexData_GetTarget (const SCE_STexData*);
SCE_EImageType SCE_TexData_GetType (const SCE_STexData*);
int SCE_TexData_GetMipmapLevel (const SCE_STexData*);
int SCE_TexData_GetWidth (const SCE_STexData*);
int SCE_TexData_GetHeight (const SCE_STexData*);
int SCE_TexData_GetDepth (const SCE_STexData*);
SCE_EPixelFormat SCE_TexData_GetPixelFormat (const SCE_STexData*);
SCE_EImageFormat SCE_TexData_GetDataFormat (const SCE_STexData*);
SCE_EType SCE_TexData_GetDataType (const SCE_STexData*);
size_t SCE_TexData_GetDataSize (const SCE_STexData*);
void* SCE_TexData_GetData (SCE_STexData*);
int SCE_TexData_IsCompressed (SCE_STexData*);
int SCE_TexData_IsDepthFormat (SCE_STexData*);
SCE_SListIterator* SCE_TexData_GetIterator (SCE_STexData*);

void SCE_TexData_Modified1 (SCE_STexData*, int, int);
void SCE_TexData_Modified2 (SCE_STexData*, int, int, int, int);
void SCE_TexData_Modified3 (SCE_STexData*, int, int, int, int, int, int);
void SCE_TexData_Unmofidied (SCE_STexData*);
void SCE_TexData_GetModified1 (const SCE_STexData*, int*, int*);
void SCE_TexData_GetModified2 (const SCE_STexData*, int*, int*, int*, int*);
void SCE_TexData_GetModified3 (const SCE_STexData*, int*, int*, int*,
                               int*, int*, int*);
int SCE_TexData_IsModified (const SCE_STexData*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

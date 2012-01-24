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
 
/* created: 28/07/2007
   updated: 24/01/2012 */

#ifndef SCERIMAGE_H
#define SCERIMAGE_H

#include <IL/il.h>
#include <IL/ilu.h>
#include <SCE/utils/SCEUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Types of texture
 */
typedef enum {
    SCE_IMAGE_1D = 0,
    SCE_IMAGE_2D,
    SCE_IMAGE_3D
    /* SCE_IMAGE_CUBE */
} SCE_EImageType;

typedef enum {
    SCE_IMAGE_NONE = 0,
    SCE_IMAGE_RED,
    SCE_IMAGE_RG,
    SCE_IMAGE_RGB,
    SCE_IMAGE_BGR,
    SCE_IMAGE_RGBA,
    SCE_IMAGE_BGRA,
    SCE_IMAGE_DEPTH,
    /* SCE_IMAGE_STENCIL */
    SCE_NUM_IMAGE_FORMATS
} SCE_EImageFormat;

/**
 * \brief Pixel formats
 * \warning see SCE_RSCEPxfToGL() before modifying this enum
 */
typedef enum {
    SCE_PXF_NONE = 0,
    SCE_PXF_LUMINANCE,
    SCE_PXF_LUMINANCE_ALPHA,
    SCE_PXF_RGB,
    SCE_PXF_RGBA,
    SCE_PXF_BGR,
    SCE_PXF_BGRA,
    SCE_PXF_DXT1,
    SCE_PXF_DXT3,
    SCE_PXF_DXT5,
    SCE_PXF_3DC,
    SCE_PXF_DEPTH24,
    SCE_PXF_DEPTH32,
    SCE_NUM_PIXEL_FORMATS
} SCE_EPixelFormat;

/**
 * \brief An image data
 * 
 * Contains the data of one image's mipmap level.
 */
typedef struct sce_simagedata SCE_SImageData;
struct sce_simagedata {
    int free_data;    /**< Boolean, true = can free \c data */
    void *data;       /**< Raw data */
    int pxf;          /**< Pixel format */
    int updated;      /**< Boolean, true when \c data is up to date */
    SCE_SListIterator it;
};

/**
 * \brief An SCE image
 * 
 * Contains an image with his own data stored into mipmap levels. It stores
 * too a DevIL identifier to manage all the image's data.
 */
typedef struct sce_simage SCE_SImage;
struct sce_simage {
    ILuint id;            /**< DevIL's identifier */
    SCE_SList mipmaps;    /**< All mipmap levels \sa SCE_SImageData  */
    unsigned int level;   /**< Activated mipmap level */
    SCE_SImageData *data; /**< Activated mipmap level's data */
};


int SCE_Init_Image (void);
void SCE_Quit_Image (void);

int SCE_Image_GetResourceType (void);

SCE_SImage* SCE_Image_Create (void);
void SCE_Image_Delete (SCE_SImage*);

void SCE_Image_ForceResize (int, int, int, int);
void SCE_Image_ForceRescale (int, float, float, float);

unsigned int SCE_Image_GetNumMipmaps (SCE_SImage*);

int SCE_Image_HaveMipmaps (SCE_SImage*);

unsigned int SCE_Image_GetMipmapLevel (SCE_SImage*);

int SCE_Image_SetMipmapLevel (SCE_SImage*, unsigned int);

int SCE_Image_UpdateMipmap (SCE_SImage*);

int SCE_Image_Update (SCE_SImage*);

int SCE_Image_GetWidth (SCE_SImage*);
int SCE_Image_GetHeight (SCE_SImage*);
int SCE_Image_GetDepth (SCE_SImage*);
int SCE_Image_GetPixelSize (SCE_SImage*);
SCE_EImageFormat SCE_Image_GetFormat (SCE_SImage*);
SCE_EType SCE_Image_GetDataType (SCE_SImage*);

SCE_EImageType SCE_Image_GetType (SCE_SImage*);
SCE_EPixelFormat SCE_Image_GetPixelFormat (SCE_SImage*);
size_t SCE_Image_GetDataSize (SCE_SImage*);

int SCE_Image_IsCompressed (SCE_SImage*);

void* SCE_Image_GetData (SCE_SImage*);

/* what define?? */
#define SCE_IMAGE_DO_NOT_CHANGE 0
void SCE_Image_Resize (SCE_SImage*, int, int, int);
void SCE_Image_Rescale (SCE_SImage*, float, float, float);

void SCE_Image_Flip (SCE_SImage*);

void SCE_Image_SetPixelFormat (SCE_SImage*, SCE_EPixelFormat);
void SCE_Image_SetAllPixelFormat (SCE_SImage*, SCE_EPixelFormat);

int SCE_Image_BuildMipmaps (SCE_SImage*);

void* SCE_Image_Load (FILE*, const char*, void*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

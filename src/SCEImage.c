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
   updated: 23/01/2012 */

#include <SCE/utils/SCEUtils.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include "SCE/core/SCEImage.h"

/**
 * \file SCERImage.c
 * \copydoc rimage
 * 
 * \file SCERImage.h
 * \copydoc rimage
 */

/**
 * \defgroup rimage Images managment using DevIL library
 * \ingroup renderer-gl
 * \internal
 * \brief Interfacing the DevIL's API and implement some features to be ready
 * for the core textures manager \c coretexture
 */

/** @{ */

static int resource_type = 0;
/*static SCEenum resize_filter = ILU_NEAREST;*/ /* unused */

#define SCE_IMG_FORCE_PONCTUAL 1
#define SCE_IMG_FORCE_PERSISTENT 2

static int rescaleforced = SCE_FALSE;
static float scale_w, scale_h, scale_d;

static int resizeforced = SCE_FALSE;
static int size_w, size_h, size_d;


/**
 * \brief Initializes the images manager
 * \returns SCE_OK on success, SCE_ERROR on error
 */
int SCE_Init_Image (void)
{
    /* initialisation de DevIL */
    ilInit ();
    iluInit ();

    ilEnable (IL_ORIGIN_SET);
    ilOriginFunc (IL_ORIGIN_LOWER_LEFT);
    /* on sauvegarde les donnees compressees */
    ilSetInteger (IL_KEEP_DXTC_DATA, IL_TRUE);

    resource_type = SCE_Resource_RegisterType (SCE_TRUE, NULL, NULL);
    if (resource_type < 0)
        goto fail;
    if (SCE_Media_Register (resource_type,
                            ".bmp .gif .jpg .dds .png .tga .jpeg .ico .mn"
                            "g .pcx .rgb .rgba .tif", SCE_Image_Load, NULL) < 0)
        goto fail;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize images manager");
    return SCE_ERROR;
}

/**
 * \brief Quit the images manager
 */
void SCE_Quit_Image (void)
{
    ilShutDown ();
}


int SCE_Image_GetResourceType (void)
{
    return resource_type;
}


static void SCE_Image_Bind (SCE_SImage *img)
{
    static SCE_SImage *bound = NULL;
    if (!img)
        ilBindImage (0);
    else if (img != bound) {
        ilBindImage (0);
        ilBindImage (img->id);
        ilActiveMipmap (img->level);
    }
    bound = img;
}


static void SCE_Image_InitData (SCE_SImageData *d)
{
    d->free_data = SCE_FALSE;
    d->data = NULL;
    d->pxf = IL_RGBA;
    d->updated = SCE_FALSE;
    SCE_List_InitIt (&d->it);
    SCE_List_SetData (&d->it, d);
}

static SCE_SImageData* SCE_Image_CreateData (void)
{
    SCE_SImageData *d = NULL;

    d = SCE_malloc (sizeof *d);
    if (!d)
        SCEE_LogSrc ();
    else
        SCE_Image_InitData (d);
    return d;
}

static void SCE_Image_DeleteData (void *data)
{
    if (data) {
        SCE_SImageData *d = data;
        if (d->free_data)
            SCE_free (d->data);
        SCE_free (d);
    }
}


static void SCE_Image_Init (SCE_SImage *img)
{
    img->id = 0;
    SCE_List_Init (&img->mipmaps);
    SCE_List_SetFreeFunc (&img->mipmaps, SCE_Image_DeleteData);
    img->level = 0;
    img->data = NULL;
}

/**
 * \brief Creates a new image
 * \returns a pointer to the new image structure
 */
SCE_SImage* SCE_Image_Create (void)
{
    SCE_SImage *img = NULL;

    img = SCE_malloc (sizeof *img);
    if (!img) {
        SCEE_LogSrc ();
        return NULL;
    }
    SCE_Image_Init (img);
    ilGenImages (1, &img->id);
    return img;
}
/**
 * \brief Deletes an image
 * \param img image to delete
 */
void SCE_Image_Delete (SCE_SImage *img)
{
    if (img) {
        ilBindImage (0);
        SCE_List_Clear (&img->mipmaps);
        ilDeleteImages (1, &img->id);
        SCE_free (img);
    }
}


/**
 * \brief Force image resizing at loading
 * \param persistent defines if the resizing is persistent or not, set SCE_TRUE
 * or SCE_FALSE
 * \param w,h,d news dimensions to set
 * \sa SCE_Image_ForceRescale()
 * 
 * To avoid resizing, so... I don't know how you do :D
 */
void SCE_Image_ForceResize (int persistent, int w, int h, int d)
{
    rescaleforced = SCE_FALSE; /* on desactive l'autre */
    resizeforced = (persistent ? SCE_IMG_FORCE_PERSISTENT :
                    SCE_IMG_FORCE_PONCTUAL);
    size_w = w; size_h = h; size_d = d;
}
/**
 * \brief Force image rescaling at loading
 * \param persistent defines if the rescaling is persistent or not, set SCE_TRUE
 * or SCE_FALSE
 * \param w,h,d news scales to set
 * \sa SCE_Image_ForceResize()
 * 
 * To avoid the rescaling, so... I don't know how you do :D
 */
void SCE_Image_ForceRescale (int persistent, float w, float h, float d)
{
    resizeforced = SCE_FALSE; /* on desactive l'autre */
    rescaleforced = (persistent ? SCE_IMG_FORCE_PERSISTENT :
                     SCE_IMG_FORCE_PONCTUAL);
    scale_w = w; scale_h = h; scale_d = d;
}

/**
 * \brief Get the number of mipmap levels of an image
 * \param img the image
 * \returns the number of mipmap levels
 */
unsigned int SCE_Image_GetNumMipmaps (SCE_SImage *img)
{
    return SCE_List_GetSize (&img->mipmaps);
}

/**
 * \brief Indicates if an image contains mipmap levels
 * \returns a boolean
 */
int SCE_Image_HaveMipmaps (SCE_SImage *img)
{
    return (SCE_List_GetLength (&img->mipmaps) > 1) ? 1 : 0;
}

/**
 * \brief Get the number of the active mipmap level
 */
unsigned int SCE_Image_GetMipmapLevel (SCE_SImage *img)
{
    return img->level;
}


/* retourne les donnees du niveau de mipmap actif */
static SCE_SImageData* SCE_Image_GetCurrentMipmapData (SCE_SImage *img)
{
    SCE_SListIterator *it = NULL;

    it = SCE_List_GetIterator (&img->mipmaps, img->level);
    if (!it) {
        SCEE_LogSrc ();
        return NULL;
    }
    return SCE_List_GetData (it);
}

/**
 * \brief Set the active mipmap level
 * \param level the mipmap level to activate
 * \returns SCE_OK on success, SCE_ERROR on error
 * \todo fix this fucking hack
 */
int SCE_Image_SetMipmapLevel (SCE_SImage *img, unsigned int level)
{
    int max_level;

    SCE_Image_Bind (img);
    /*ilBindImage (0);
      ilBindImage (img->id);*/
    /*max_level = SCE_Image_GetNumMipmaps (img);*/
    ilGetIntegerv (IL_NUM_MIPMAPS, &max_level);
    max_level++;
    if (level >= max_level) {
        SCEE_Log (SCE_INVALID_ARG);
        SCEE_LogMsg ("you can't active this mipmap level (%d), the maximum "
                       "mipmap level for this image is %d", level, max_level);
        return SCE_ERROR;
    }
    img->level = level;
    img->data = SCE_Image_GetCurrentMipmapData (img);
    SCE_Image_Bind (img);
    return SCE_OK;
}



static int SCE_RIsCompressedPixelFormat (SCEenum fmt)
{
    if (fmt == IL_DXT1 || fmt == IL_DXT2 ||
        fmt == IL_DXT3 || fmt == IL_DXT4 ||
        fmt == IL_DXT5 || fmt == IL_3DC)
        return SCE_TRUE;
    else
        return SCE_FALSE;
}

/**
 * \brief Updates the active mipmap level of an image
 * \returns SCE_OK on success, SCE_ERROR on error
 */
int SCE_Image_UpdateMipmap (SCE_SImage *img)
{
    SCE_SImageData *d = img->data;

    if (!d) {
        SCEE_Log (SCE_INVALID_OPERATION);
        return SCE_ERROR;
    }

    if (d->updated)
        return SCE_OK;

    if (d->free_data)
        SCE_free (d->data);
    d->data = NULL;

    if (SCE_Image_GetIsCompressed (img)) {
        size_t data_size = SCE_Image_GetDataSize (img);
        d->free_data = 1;
        d->data = SCE_malloc (data_size);
        if (!d->data) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
        ilGetDXTCData (d->data, data_size, d->pxf);
    } else {
        d->free_data = 0;
        ilConvertImage (d->pxf, SCE_Image_GetDataType (img));
        d->data = ilGetData ();
    }

    d->updated = SCE_TRUE;

    return SCE_OK;
}


/* met a jour la liste chainee des mipmaps */
/**
 * \todo faire en sorte de conserver les formats de pixel precedemment
 *       envoyes pour chaque niveau de mipmap
 */
static int SCE_Image_UpdateMipmapList (SCE_SImage *img)
{
    SCE_SImageData *d = NULL;
    int num_mipmaps, i;

    /* let's activate the first mipmap level
       (I don't trust ilActiveMipmap (0)) */
    ilBindImage (0);
    ilBindImage (img->id);

    SCE_List_Clear (&img->mipmaps);

    ilGetIntegerv (IL_NUM_MIPMAPS, &num_mipmaps);
    num_mipmaps++;  /* parce qu'aucun mipmap = 0 (et moi je veux au moins 1) */
    for (i = 0; i < num_mipmaps; i++) {
        ilBindImage (0);
        ilBindImage (img->id);
        ilActiveMipmap (i);

        d = SCE_Image_CreateData ();
        if (!d) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
        ilGetIntegerv (IL_DXTC_DATA_FORMAT, &d->pxf);
        if (d->pxf == IL_DXT_NO_COMP)
            ilGetIntegerv (IL_IMAGE_FORMAT, &d->pxf);
        SCE_List_Appendl (&img->mipmaps, &d->it);
        d = NULL;
    }

    /* si level est trop grand, on le defini
       au niveau de mipmap le plus petit */
    if (img->level >= num_mipmaps)
        img->level = SCE_List_GetIndex (SCE_List_GetLast (&img->mipmaps));

    /* recuperation du niveau de mipmap */
    if (SCE_Image_SetMipmapLevel (img, img->level) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }

    return SCE_OK;
}

/**
 * \brief Updates the mipmap levels list of \p img
 * 
 * Clears the mipmap levels list and rebuild it from the DevIL's image
 * informations. The informations are the number of mipmaps and the pixel format
 * or each level.
 * It updates only the list, doesn't call SCE_Image_UpdateMipmap() for each
 * level.
 */
int SCE_Image_Update (SCE_SImage *img)
{
    return SCE_Image_UpdateMipmapList (img);
}


#define SCE_RIMGGET(name, ienum)\
int SCE_Image_Get##name (SCE_SImage *img)\
{\
    SCE_Image_Bind (img);\
    return ilGetInteger (ienum);\
}

SCE_RIMGGET (Width,     IL_IMAGE_WIDTH)
SCE_RIMGGET (Height,    IL_IMAGE_HEIGHT)
SCE_RIMGGET (Depth,     IL_IMAGE_DEPTH)
SCE_RIMGGET (PixelSize, IL_IMAGE_BYTES_PER_PIXEL)
/* SCE_RIMGGET (DataType,  IL_IMAGE_TYPE) */
#undef SCE_RIMGGET

static SCE_EImageFormat SCE_Image_FormatFromIL (SCEenum f)
{
    switch (f) {
    case IL_BGR: return SCE_IMAGE_BGR;
    case IL_BGRA: return SCE_IMAGE_BGRA;
    case IL_LUMINANCE: return SCE_IMAGE_RED;
    case IL_LUMINANCE_ALPHA: return SCE_IMAGE_RG;
    case IL_RGB: return SCE_IMAGE_RGB;
    case IL_RGBA: return SCE_IMAGE_RGBA;
    default: return SCE_IMAGE_RED; /* herp derp */
    }
}

SCE_EImageFormat SCE_Image_GetFormat (SCE_SImage *img)
{
    SCE_Image_Bind (img);
    return SCE_Image_FormatFromIL (ilGetInteger (IL_IMAGE_FORMAT));
}

static SCE_EType SCE_Image_ILTypeToSCE (SCEenum t)
{
#define SCE_TYPE_CASE(t)\
    case IL_##t: return SCE_##t

    switch (t) {
        SCE_TYPE_CASE (UNSIGNED_BYTE);
        SCE_TYPE_CASE (SHORT);
        SCE_TYPE_CASE (UNSIGNED_SHORT);
        SCE_TYPE_CASE (INT);
        SCE_TYPE_CASE (UNSIGNED_INT);
        SCE_TYPE_CASE (FLOAT);
        SCE_TYPE_CASE (DOUBLE);
    }
    return 0;
#undef SCE_TYPE_CASE
}

SCE_EType SCE_Image_GetDataType (SCE_SImage *img)
{
    SCE_Image_Bind (img);
    return SCE_Image_ILTypeToSCE (ilGetInteger (IL_IMAGE_TYPE));
}


/**
 * \brief Gets the type on an image
 * \returns the image type
 */
SCE_EImageType SCE_Image_GetType (SCE_SImage *img)
{
    int type = SCE_IMAGE_1D;
    if (SCE_Image_GetDepth (img) > 1)
        type = SCE_IMAGE_3D;
    else if (SCE_Image_GetHeight (img) > 1)
        type = SCE_IMAGE_2D;
    return type;
}

static SCE_EPixelFormat SCE_Image_PxfFromIL (int pxf)
{
    switch (pxf) {
    case IL_LUMINANCE: return SCE_PXF_LUMINANCE;
    case IL_LUMINANCE_ALPHA: return SCE_PXF_LUMINANCE_ALPHA;
    case IL_RGB: return SCE_PXF_RGB;
    case IL_RGBA: return SCE_PXF_RGBA;
    case IL_BGR: return SCE_PXF_BGR;
    case IL_BGRA: return SCE_PXF_BGRA;
    case IL_DXT1: return SCE_PXF_DXT1;
    case IL_DXT2:
    case IL_DXT3: return SCE_PXF_DXT3;
    case IL_DXT4:
    case IL_DXT5: return SCE_PXF_DXT5;
    case IL_3DC: return SCE_PXF_3DC;
    default: return IL_LUMINANCE; /* problem? :trollface */
    }
}
static int SCE_Image_PxfToIL (SCE_EPixelFormat pxf)
{
    int p[SCE_NUM_PIXEL_FORMATS] = {
        IL_LUMINANCE,           /* SCE_PXF_NONE */
        IL_LUMINANCE,
        IL_LUMINANCE_ALPHA,
        IL_RGB,
        IL_RGBA,
        IL_BGR,
        IL_BGRA,
        IL_DXT1,
        IL_DXT3,
        IL_DXT5,
        IL_3DC,
        IL_RGB,                 /* depth 24 */
        IL_RGBA                 /* depth 32 */
    };
    return p[pxf];
}

/**
 * \brief Gets the image pixel format
 * \returns the pixel format \sa defines de merde
 * \todo améliorer cette doc en ajoutant le support des defines
 */
SCE_EPixelFormat SCE_Image_GetPixelFormat (SCE_SImage *img)
{
    return SCE_Image_PxfFromIL (img->data->pxf);
}


/**
 * \brief Gets the size of the data of the active mipmap level
 * \returns the size of the data (bytes)
 */
size_t SCE_Image_GetDataSize (SCE_SImage *img)
{
    SCE_Image_Bind (img);
    if (SCE_Image_GetIsCompressed (img))
        return ilGetDXTCData (NULL, 0, img->data->pxf);
    else
        return ilGetInteger (IL_IMAGE_SIZE_OF_DATA);
}

/**
 * \brief Indicates if the active mipmap level has a compressed pixel format
 * \returns a boolean
 */
int SCE_Image_GetIsCompressed (SCE_SImage *img)
{
    return SCE_RIsCompressedPixelFormat (img->data->pxf);
}

/**
 * \brief Gets the pointer to the data of te active mipmap level
 * \returns the pointer to the data
 */
void* SCE_Image_GetData (SCE_SImage *img)
{
    /* mise a jour du niveau de mipmap actif */
    SCE_Image_UpdateMipmap (img);
    return img->data->data;
}


/**
 * \brief Resizes an image, take its new dimensions
 * \param w the new width
 * \param h the new height
 * \param d the new depth (only for 3D images)
 * \note Dimensions less than 1 are not modified.
 * \sa SCE_Image_Rescale()
 * \todo add filters managment
 */
void SCE_Image_Resize (SCE_SImage *img, int w, int h, int d)
{
    SCE_Image_Bind (img);
    if (w < 1)
        w = SCE_Image_GetWidth (img);
    if (h < 1)
        h = SCE_Image_GetHeight (img);
    if (d < 1)
        d = SCE_Image_GetDepth (img);

    iluScale (w, h, d);
    img->data->updated = SCE_FALSE;
}

/**
 * \brief Rescale an image, take its new scales
 * \param w the new width factor
 * \param h the new height factor
 * \param d the new depth factor (only on 3D images)
 * \note Set parameter at 0 at your own risk
 * \sa SCE_Image_Resize()
 * \todo add filters managment
 */
void SCE_Image_Rescale (SCE_SImage *img, float w, float h, float d)
{
    SCE_Image_Bind (img);
    w = SCE_Image_GetWidth (img) * w;
    h = SCE_Image_GetHeight (img) * h;
    d = SCE_Image_GetDepth (img) * d;

    iluScale (w, h, d);
    img->data->updated = SCE_FALSE;
}

/**
 * \brief Flip an image, inverse it from the y axis
 * \todo this function do not works, DevIL can't flip an image (pd)
 */
void SCE_Image_Flip (SCE_SImage *img)
{
    SCE_Image_Bind (img);
    iluFlipImage (); /* TODO: seg fault §§ */
    img->data->updated = SCE_FALSE;
}


/* repete une operation pour chaque niveau de mipmap */
#define SCE_IMAGE_SETALLMIPMAPS(action)\
{\
    unsigned int level = SCE_Image_GetMipmapLevel (img), i, n;  \
    n = SCE_Image_GetNumMipmaps (img);                          \
    for (i = 0; i < n; i++) {                                   \
        if (SCE_Image_SetMipmapLevel (img, i) < 0) {            \
            SCEE_LogSrc ();                                     \
            break;                                              \
        }                                                       \
        action;                                                 \
        i++;                                                    \
    }                                                           \
    SCE_Image_SetMipmapLevel (img, level);                      \
}

/**
 * \brief Sets the pixel format of the active mipmap level of \p img
 * \param fmt the new pixel format
 * \sa SCE_Image_SetAllPixelFormat()
 */
void SCE_Image_SetPixelFormat (SCE_SImage *img, SCE_EPixelFormat fmt)
{
    img->data->pxf = SCE_Image_PxfToIL (fmt);
    img->data->updated = SCE_FALSE;
}
/**
 * \brief Sets the pixel format of all mipmap levels of \p img
 * \param fmt the new pixel format
 * \sa SCE_Image_SetPixelFormat()
 */
void SCE_Image_SetAllPixelFormat (SCE_SImage *img, SCE_EPixelFormat fmt)
{
    SCE_Image_Bind (img);
    SCE_IMAGE_SETALLMIPMAPS (SCE_Image_SetPixelFormat (img, fmt))
}


/**
 * \brief Makes mipmap levels for an image based on its first image
 * \returns SCE_ERROR on error, SCE_OK otherwise
 */
int SCE_Image_BuildMipmaps (SCE_SImage *img)
{
    SCE_Image_Bind (img);
    /* ilActiveLevel (0) ? */
    if (iluBuildMipmaps () == IL_FALSE) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("DevIL fails to build mipmaps: %s",
                     iluErrorString (ilGetError ()));
        return SCE_ERROR;
    }

    /* update mipmaps list */
    SCE_Image_Update (img);

    return SCE_OK;
}


/**
 * \brief Loads a new image. This function is the callback for the media manager
 * \returns a new SCE_Image_*
 */
void* SCE_Image_Load (FILE *fp, const char *fname, void *unused)
{
    SCE_SImage *img = NULL;
    (void)unused;
    img = SCE_Image_Create ();
    if (!img) {
        SCEE_LogSrc ();
        return NULL;
    }

    SCE_Image_Bind (img);

    if (!ilLoadImage ((char*)fname)) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("DevIL can't load '%s': %s",
                     fname, iluErrorString (ilGetError ()));
        SCE_Image_Delete (img);
        return NULL;
    }

    /* application des redimensions */
    if (resizeforced)
        SCE_Image_Resize (img, size_w, size_h, size_d);
    else if (rescaleforced)
        SCE_Image_Rescale (img, scale_w, scale_h, scale_d);

    if (SCE_Image_Update (img) < 0) {
        SCEE_LogSrc ();
        SCE_Image_Delete (img);
        return NULL;
    }

    /* annulation si la demande etait ponctuelle */
    if (resizeforced == SCE_IMG_FORCE_PONCTUAL)
        resizeforced = SCE_FALSE;
    else if (rescaleforced == SCE_IMG_FORCE_PONCTUAL)
        rescaleforced = SCE_FALSE;

    return img;
}

/** @} */

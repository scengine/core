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

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCETextureData.h"

void SCE_TexData_Init (SCE_STexData *d)
{
    d->img = NULL;
    d->canfree = SCE_FALSE;
    d->user = SCE_TRUE;
    d->target = 0;
    d->type = SCE_IMAGE_1D;
    d->level = 0;
    d->w = d->h = d->d = 1;
    d->pxf = SCE_PXF_RGBA;
    d->data_fmt = SCE_IMAGE_RGBA;
    d->data_type = SCE_UNSIGNED_BYTE;
    d->data_size = 0;
    d->data_user = SCE_TRUE;
    d->data = NULL;
    d->comp = SCE_FALSE;

    d->modified = SCE_FALSE;
    d->mod_x = d->mod_y = d->mod_z = 0;
    d->mod_w = d->mod_h = d->mod_d = 0;

    SCE_List_InitIt (&d->it);
    SCE_List_SetData (&d->it, d);
}
void SCE_TexData_Clear (SCE_STexData *d)
{
    SCE_List_Remove (&d->it);
    if (d->canfree && SCE_Resource_Free (d->img))
        SCE_Image_Delete (d->img);
    if (!d->data_user)
        SCE_free (d->data);
}
SCE_STexData* SCE_TexData_Create (void)
{
    SCE_STexData *d = NULL;
    if (!(d = SCE_malloc (sizeof *d)))
        SCEE_LogSrc ();
    else
        SCE_TexData_Init (d);
    return d;
}
SCE_STexData* SCE_TexData_CreateFromImage (SCE_SImage *img, int canfree)
{
    SCE_STexData *d = NULL;
    if (!(d = SCE_TexData_Create ())) goto fail;
    if (SCE_TexData_SetImage (d, img, canfree) < 0) goto fail;
    return d;
fail:
    SCE_TexData_Delete (d);
    SCEE_LogSrc ();
    return NULL;
}
void SCE_TexData_Delete (SCE_STexData *d)
{
    if (d) {
        SCE_TexData_Clear (d);
        SCE_free (d);
    }
}


void SCE_TexData_Copy (SCE_STexData *dst, const SCE_STexData *src)
{
    dst->img = src->img;
    dst->canfree = SCE_FALSE;
    dst->target = src->target;
    dst->type = src->type;
    dst->level = src->level;    /* asian */
    dst->w = src->w;
    dst->h = src->h;
    dst->d = src->d;
    dst->pxf = src->pxf;
    dst->data_fmt = src->data_fmt;
    dst->data_type = src->data_type;
    dst->data_size = src->data_size;
    dst->data_user = SCE_TRUE;  /* let's say the user is 'src' */
    dst->data = src->data;
    dst->comp = src->comp;
}

/**
 * \brief Duplicates a texture data
 * \param d the texture data to copy
 *
 * This function duplicates the data of the texture but not the image.
 * \returns the copy of \p d
 */
SCE_STexData* SCE_TexData_Dup (const SCE_STexData *d)
{
    SCE_STexData *data = NULL;

    if (!(data = SCE_TexData_Create ())) {
        SCEE_LogSrc ();
        return NULL;
    }

    SCE_TexData_Copy (data, d);
    data->data_user = SCE_FALSE;
    data->data = NULL;

    if (data->data_size > 0) {
        if (!(data->data = SCE_malloc (data->data_size))) {
            SCE_TexData_Delete (data);
            SCEE_LogSrc ();
            return NULL;
        }
        memcpy (data->data, d->data, data->data_size);
    }
    return data;
}


int SCE_TexData_SetImage (SCE_STexData *d, SCE_SImage *img, int canfree)
{
    d->data_size = SCE_Image_GetDataSize (img);
    d->data_user = SCE_FALSE;
    if (!(d->data = SCE_malloc (d->data_size))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    memcpy (d->data, SCE_Image_GetData (img), d->data_size);
    d->img = img;
    d->canfree = canfree;
    d->target = SCE_Image_GetType (img);
    d->level = SCE_Image_GetMipmapLevel (img);
    d->w = SCE_Image_GetWidth (img);
    d->h = SCE_Image_GetHeight (img);
    d->d = SCE_Image_GetDepth (img);
    /* NOTE: warning, not calling SCE_TexData_SetPixelFormat() */
    d->pxf = SCE_Image_GetPixelFormat (img);
    d->data_fmt = SCE_Image_GetFormat (img);
    d->data_type = SCE_Image_GetDataType (img);
    d->comp = SCE_Image_IsCompressed (img);
    return SCE_OK;
}
void SCE_TexData_SetTarget (SCE_STexData *d, SCEenum t)
{
    d->target = t;
}
void SCE_TexData_SetType (SCE_STexData *d, SCE_EImageType t)
{
    d->type = t;
}
void SCE_TexData_SetMipmapLevel (SCE_STexData *d, int l)
{
    d->level = l;
}
void SCE_TexData_SetDimensions (SCE_STexData *td, int w, int h, int d)
{
    td->w = w; td->h = h; td->d = d;
}
void SCE_TexData_SetWidth (SCE_STexData *d, int w)
{
    d->w = w;
}
void SCE_TexData_SetHeight (SCE_STexData *d, int h)
{
    d->h = h;
}
void SCE_TexData_SetDepth (SCE_STexData *td, int d)
{
    td->d = d;
}
static SCE_EImageFormat SCE_TexData_ToIntegerFormat (SCE_EImageFormat fmt)
{
    switch (fmt) {
    case SCE_IMAGE_REDI:
    case SCE_IMAGE_RGI:
    case SCE_IMAGE_RGBI:
    case SCE_IMAGE_BGRI:
    case SCE_IMAGE_RGBAI:
    case SCE_IMAGE_BGRAI:
        return fmt;
    default:
        return fmt + SCE_IMAGE_REDI - 1;
    }
}
void SCE_TexData_SetPixelFormat (SCE_STexData *d, SCE_EPixelFormat p)
{
    d->pxf = p;
    if (SCE_TexData_IsPixelFormatInteger (p))
        d->data_fmt = SCE_TexData_ToIntegerFormat (d->data_fmt);
}
void SCE_TexData_SetDataFormat (SCE_STexData *d, SCE_EImageFormat f)
{
    if (SCE_TexData_IsPixelFormatInteger (d->pxf))
        d->data_fmt = SCE_TexData_ToIntegerFormat (f);
    else
        d->data_fmt = f;
}
void SCE_TexData_SetDataType (SCE_STexData *d, SCE_EType t)
{
    d->data_type = t;
}
void SCE_TexData_SetData (SCE_STexData *d, void *data, int canfree)
{
    d->data = data;
    d->data_user = !canfree;
}

SCE_SImage* SCE_TexData_GetImage (SCE_STexData *d)
{
    return d->img;
}
SCEenum SCE_TexData_GetTarget (const SCE_STexData *d)
{
    return d->target;
}
SCE_EImageType SCE_TexData_GetType (const SCE_STexData *d)
{
    return d->type;
}
int SCE_TexData_GetMipmapLevel (const SCE_STexData *d)
{
    return d->level;
}
int SCE_TexData_GetWidth (const SCE_STexData *d)
{
    return d->w;
}
int SCE_TexData_GetHeight (const SCE_STexData *d)
{
    return d->h;
}
int SCE_TexData_GetDepth (const SCE_STexData *d)
{
    return d->d;
}
SCE_EPixelFormat SCE_TexData_GetPixelFormat (const SCE_STexData *d)
{
    return d->pxf;
}
SCE_EImageFormat SCE_TexData_GetDataFormat (const SCE_STexData *d)
{
    return d->data_fmt;
}
SCE_EType SCE_TexData_GetDataType (const SCE_STexData *d)
{
    return d->data_type;
}
size_t SCE_TexData_GetDataSize (const SCE_STexData *d)
{
    return d->data_size;
}
void* SCE_TexData_GetData (SCE_STexData *d)
{
    return d->data;
}
/* TODO: d->comp is only set when using images */
int SCE_TexData_IsCompressed (SCE_STexData *d)
{
    return d->comp;
}
int SCE_TexData_IsDepthFormat (SCE_STexData *d)
{
    return d->pxf == SCE_PXF_DEPTH24 || d->pxf == SCE_PXF_DEPTH32;
}
SCE_SListIterator* SCE_TexData_GetIterator (SCE_STexData *d)
{
    return &d->it;
}

int SCE_TexData_IsPixelFormatInteger (SCE_EPixelFormat pxf)
{
    switch (pxf) {
    case SCE_PXF_R32UI:
    case SCE_PXF_RGBA8UI:
        return SCE_TRUE;
    default:
        return SCE_FALSE;
    }
}

void SCE_TexData_Modified1 (SCE_STexData *d, int x, int w)
{
    if (d->modified) {
        /* enlarge current modified zone */
        d->mod_x = MIN (x, d->mod_x);
        d->mod_w = MAX (x + w, d->mod_w);
    } else {
        d->mod_x = x;
        d->mod_w = x + w;
    }
    d->modified = SCE_TRUE;
}
void SCE_TexData_Modified2 (SCE_STexData *d, int x, int y, int w, int h)
{
    if (d->modified) {
        /* enlarge current modified zone */
        d->mod_x = MIN (x, d->mod_x);
        d->mod_w = MAX (x + w, d->mod_w);
        d->mod_y = MIN (y, d->mod_y);
        d->mod_h = MAX (y + h, d->mod_h);
    } else {
        d->mod_x = x;
        d->mod_w = x + w;
        d->mod_y = y;
        d->mod_h = y + h;
    }
    d->modified = SCE_TRUE;
}
void SCE_TexData_Modified3 (SCE_STexData *td, int x, int y, int z,
                            int w, int h, int d)
{
    if (td->modified) {
        /* enlarge current modified zone */
        td->mod_x = MIN (x, td->mod_x);
        td->mod_w = MAX (x + w, td->mod_w);
        td->mod_y = MIN (y, td->mod_y);
        td->mod_h = MAX (y + h, td->mod_h);
        td->mod_z = MIN (z, td->mod_z);
        td->mod_d = MAX (z + d, td->mod_d);
    } else {
        td->mod_x = x;
        td->mod_w = x + w;
        td->mod_y = y;
        td->mod_h = y + h;
        td->mod_z = z;
        td->mod_d = z + d;
    }
    td->modified = SCE_TRUE;
}
void SCE_TexData_Unmofidied (SCE_STexData *d)
{
    d->modified = SCE_FALSE;
}


void SCE_TexData_GetModified1 (const SCE_STexData *d, int *x, int *w)
{
    *x = d->mod_x;
    *w = d->mod_w - d->mod_x;
}
void SCE_TexData_GetModified2 (const SCE_STexData *d, int *x, int *y,
                               int *w, int *h)
{
    *x = d->mod_x;
    *w = d->mod_w - d->mod_x;
    *y = d->mod_y;
    *h = d->mod_h - d->mod_y;
}
void SCE_TexData_GetModified3 (const SCE_STexData *td, int *x, int *y, int *z,
                               int *w, int *h, int *d)
{
    *x = td->mod_x;
    *w = td->mod_w - td->mod_x;
    *y = td->mod_y;
    *h = td->mod_h - td->mod_y;
    *z = td->mod_z;
    *d = td->mod_d - td->mod_z;
}

int SCE_TexData_IsModified (const SCE_STexData *d)
{
    return d->modified;
}

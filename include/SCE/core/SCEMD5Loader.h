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

/* created: 10/04/2009
   updated: 08/07/2009 */

#ifndef SCEMD5LOADER_H
#define SCEMD5LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define SCE_MD5MESH_FILE_EXTENSION "md5mesh"
#define SCE_MD5ANIM_FILE_EXTENSION "md5anim"

int SCE_Init_idTechMD5 (void);
void SCE_Quit_idTechMD5 (void);

void* SCE_idTechMD5_LoadMesh (FILE*, const char*, void*);
void* SCE_idTechMD5_LoadAnim (FILE*, const char*, void*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

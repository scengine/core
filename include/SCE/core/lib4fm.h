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
 
#ifndef LIB4FM_H
#define LIB4FM_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef FFM_VERSION_STRING
 #define FFM_VERSION_STRING "(use ffm_getversionstring() function)"
#endif
#define FFM_FILE_EXTENSION "4fm"

#define FFM_MAGIC 0x4fba /* 4f m/2 m/2+1 */

typedef unsigned int FMuint;
/**
 * \todo a reviser : les types doivent etres de taille constante
 */
typedef float FMfloat4;         /* doit faire 4 octets */
typedef unsigned short FMuint2; /* doit faire 2 octets */
typedef FMuint FMuint4;         /* doit faire 4 octets */
typedef int FMint4;             /* doit faire 4 octets */


typedef struct FFMesh {
    int canfree;     /* autorisation de detruire les donnees */

    FMfloat4 *pos;
    FMfloat4 *tex;
    FMfloat4 *nor;
    FMint4 vcount;

    void *indices;
    FMint4 icount;
    FMint4 size;
} FFMesh;

const char* ffm_geterror (void);
const char* ffm_getversionstring (void);

FFMesh* ffm_new (void);
void ffm_canfree (FFMesh*, int);
void ffm_clear (FFMesh*);
void ffm_free (FFMesh*);

FFMesh** ffm_read (FILE*, int*);
int ffm_write (FILE*, FFMesh**, FMuint, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */

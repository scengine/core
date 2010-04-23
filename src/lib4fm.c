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
 
#include <stdlib.h>
#include <stdio.h>

#include "SCE/core/lib4fm.h"

/*** fonctions de gestion des erreurs ***/
static const char* ffm_error (const char *e)
{
    static const char *error = NULL;
    if (e)
        error = e;
    return error;
}

const char* ffm_geterror (void)
{
    return ffm_error (NULL);
}

const char* ffm_getversionstring (void)
{
    return FFM_VERSION_STRING;
}

/*** fonction de gestion des allocations memoire ***/
static void* my_alloc (size_t size)
{
    void *p = malloc (size);
    if (!p)
        ffm_error ("can't allocate memory!\n");
    return p;
}

#define malloc(s) my_alloc (s)

/*** fonctions de manipulation d'une structure FFMesh ***/
static void ffm_init (FFMesh *m)
{
    m->pos = m->tex = m->nor = NULL;
    m->vcount = m->icount = 0;
    m->indices = NULL;
    m->size = 0;
    m->canfree  = 0;
}
FFMesh* ffm_new (void)
{
    FFMesh *m = malloc (sizeof *m);
    ffm_init (m);
    return m;
}
void ffm_canfree (FFMesh *m, int c)
{
    m->canfree = c;
}
void ffm_clear (FFMesh *m)
{
    if (m)
    {
        if (m->canfree) {
            free (m->pos);
            free (m->tex);
            free (m->nor);
            free (m->indices);
        }
        m->pos = NULL;
        m->tex = NULL;
        m->nor = NULL;
        m->indices = NULL;
    }
}
void ffm_free (FFMesh *m)
{
    ffm_clear (m);
    free (m);
}


FFMesh** ffm_read (FILE *fp, int *nb_meshs)
{
    int i, unused, magic;
    int *n_meshs = nb_meshs;  /* affectation importante ! */
    FFMesh **meshs = NULL;
    char a, b;

    /* verification de l'argument */
    if (!n_meshs)
        n_meshs = &unused;

#define fread(a, b, c) fread (a, b, c, fp)

    fread (&magic, sizeof magic, 1);
    if (magic != FFM_MAGIC) {
        ffm_error ("invalid file format, bad magic number!");
        return NULL;
    }
    fread (n_meshs, sizeof *n_meshs, 1);

    meshs = malloc (*n_meshs * sizeof *meshs);
    if (!meshs)
        return NULL;

    for (i=0; i<*n_meshs; i++) {
#define m meshs[i]

#define FFM_ASSERT(c)\
        if ((c)) {\
            for (i++; i>0; i--)\
                ffm_free (meshs[i-1]);\
            free (meshs);\
            return NULL;\
        }

        FFM_ASSERT (!(m = ffm_new ()))
        /* lecture des donnees */
        fread (&m->vcount, sizeof m->vcount, 1);
        fread (&a, 1, 1);
        fread (&b, 1, 1);

        FFM_ASSERT (!(m->pos = malloc (m->vcount * 3 * sizeof *m->pos)))
        if (a) FFM_ASSERT (!(m->tex = malloc (m->vcount * 2 * sizeof *m->tex)))
        if (b) FFM_ASSERT (!(m->nor = malloc (m->vcount * 3 * sizeof *m->nor)))

        fread (m->pos, sizeof *m->pos, m->vcount * 3);
        if (a) fread (m->tex, sizeof *m->tex, m->vcount * 2);
        if (b) fread (m->nor, sizeof *m->nor, m->vcount * 3);

        fread (&m->size, sizeof m->size, 1);
        fread (&m->icount, sizeof m->icount, 1);
        if (m->size) {
            FFM_ASSERT (!(m->indices = malloc (m->icount * m->size)))
            fread (m->indices, m->size, m->icount);
        }
#undef m
    }

#undef fread
#undef FFM_ASSERT

    return meshs;
}

static FMuint2* convert (FMint4 *indices, FMuint n)
{
    FMuint i;
    FMuint2 *new = malloc (n * sizeof *new);
    for (i=0; i<n; i++)
        new[i] = indices[i];
    return new;
}

int ffm_write (FILE *fp, FFMesh **meshs, FMuint n_meshs, int si)
{
    FMint4 i;
    const FMint4 magic = FFM_MAGIC;
    FMint4 n = n_meshs;
    char a, b;

#define fwrite(a, b, c) fwrite (a, b, c, fp)
    /* nombre "magique" */
    fwrite (&magic, sizeof magic, 1);
    /* nombre de maillages */
    fwrite (&n, sizeof n, 1);

    for (i=0; i<n; i++) {
#define m meshs[i]
        /* informations */
        fwrite (&m->vcount, sizeof m->vcount, 1);
        a = ((m->tex) ? 1 : 0);
        b = ((m->nor) ? 1 : 0);
        fwrite (&a, sizeof a, 1);
        fwrite (&b, sizeof b, 1);

        /* donnees */
        fwrite (m->pos, sizeof *m->pos, m->vcount*3);
        if (a) fwrite (m->tex, sizeof *m->tex, m->vcount*2);
        if (b) fwrite (m->nor, sizeof *m->nor, m->vcount*3);

        /* conversion eventuelle des indices */
        if (m->indices) {
            if (si && m->icount < 256*256 && m->size == sizeof (FMint4)) {
                /*! cette action modifie la structure !*/
                void *tmp = convert (m->indices, m->icount);
                if (!tmp)
                    return -1;
                if (m->canfree)
                    free (m->indices);
                m->indices = tmp;
                m->size = sizeof (FMuint2);
            }
        }
        fwrite (&m->size, sizeof m->size, 1);
        fwrite (&m->icount, sizeof m->icount, 1);
        fwrite (m->indices, m->size, m->icount);
#undef m
    }
#undef fwrite

    return 0;
}

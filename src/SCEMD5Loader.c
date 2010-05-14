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
   updated: 04/06/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEJoint.h"
#include "SCE/core/SCEAnimatedGeometry.h"
#include "SCE/core/SCEAnimation.h"
#include "SCE/core/SCEMD5Loader.h"

static int is_init = SCE_FALSE;

int SCE_Init_idTechMD5 (void)
{
    if (is_init)
        return SCE_OK;
    if (SCE_Media_Register (SCE_AnimGeom_GetResourceType (),
                            "."SCE_MD5MESH_FILE_EXTENSION,
                            SCE_idTechMD5_LoadMesh, NULL) < 0)
        goto fail;
    if (SCE_Media_Register (SCE_Anim_GetResourceType (),
                            "."SCE_MD5ANIM_FILE_EXTENSION,
                            SCE_idTechMD5_LoadAnim, NULL) < 0)
        goto fail;
    is_init = SCE_TRUE;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize id Tech .md5 loader");
    return SCE_ERROR;
}
void SCE_Quit_idTechMD5 (void)
{
    is_init = SCE_FALSE;
}

void* SCE_idTechMD5_LoadMesh (FILE *fp, const char *fname, void *unused)
{
    char buff[512];
    int version;
    int i;
    SCE_SAnimatedGeometry *ageom = NULL;
    SCE_SSkeleton *baseskel = NULL;
    int n_joints = 0, n_meshes = 0;
    int n_verts = 0, n_weights = 0, n_tris = 0;
    int max_tris = 0, max_verts = 0;
    SCEindices *indices = NULL;

    (void)unused;
    if (!(baseskel = SCE_Skeleton_Create ()))
        goto failure;

    while (!feof (fp)) {
        fgets (buff, sizeof (buff), fp);
        if (sscanf (buff, " MD5Version %d", &version) == 1) {
            if (version != 10)
                goto failure;
        }
        else if (sscanf (buff, " numJoints %d", &n_joints) == 1) {
            if (n_joints > 0) {
                if (SCE_Skeleton_AllocateJoints (baseskel, n_joints) < 0)
                    goto failure;
                if (SCE_Skeleton_AllocateMatrices (baseskel, 0) < 0)
                    goto failure;
            }
        } else if (sscanf (buff, " numMeshes %d", &n_meshes) == 1) {
            if (n_meshes > 0) {
                /* ok cool */
            }
        } else if (strncmp (buff, "joints {", 8) == 0) {
            SCE_SJoint *joints = SCE_Skeleton_GetJoints (baseskel);
            for (i = 0; i < n_joints; ++i) {
                SCE_SJoint *joint = &joints[i];
                char useless[64];
                fgets (buff, sizeof (buff), fp);

                if (sscanf (buff, "%s %d ( %f %f %f ) ( %f %f %f )",
                            useless, &joint->parent, &joint->position[0],
                            &joint->position[1], &joint->position[2],
                            &joint->orientation[0], &joint->orientation[1],
                            &joint->orientation[2]) == 8)
                    SCE_Quaternion_ComputeW (joint->orientation);
            }
        } else if (strncmp (buff, "mesh {", 6) == 0 && !ageom) {
            int vert_index = 0;
            int tri_index = 0;
            int w_index = 0;
            float fdata[4];
            int idata[3];
            char shader[42];

            if (!(ageom = SCE_AnimGeom_Create()))
                goto failure;

            while ((buff[0] != '}') && !feof (fp)) {
                fgets (buff, sizeof (buff), fp);

                if (sscanf (buff, " numverts %d", &n_verts) == 1) {
                    if (n_verts > max_verts) {
                        max_verts = n_verts;
                        if (SCE_AnimGeom_AllocateVertices (ageom, n_verts) < 0)
                            goto failure;
                    }
                } else if (sscanf (buff, " numtris %d", &n_tris) == 1) {
                    if (n_tris > max_tris) {
                        max_tris = n_tris;
                        SCE_free (indices);
                        if (!(indices = SCE_malloc(max_tris*3*sizeof *indices)))
                            goto failure;
                    }
                } else if (sscanf (buff, " numweights %d", &n_weights) == 1) {
                    if (n_weights > 0) {
                        /* allocate memory for vertex weights */
                        if (SCE_AnimGeom_AllocateWeights (ageom, n_weights) < 0)
                            goto failure;
                        if (SCE_AnimGeom_AllocateBaseVertices (ageom,
                                                               SCE_POSITION,
                                                               SCE_TRUE) < 0)
                            goto failure;
                    }
                } else if (sscanf (buff, " vert %d ( %f %f ) %d %d",
                                   &vert_index, &fdata[0], &fdata[1],
                                   &idata[0], &idata[1]) == 5) {
                    /* copy vertex data */
                    SCE_SVertex *verts = SCE_AnimGeom_GetVertices (ageom);
                    verts[vert_index].weight_id = idata[0];
                    verts[vert_index].weight_count = idata[1];
                } else if (sscanf (buff, " tri %d %d %d %d", &tri_index,
                                   &idata[0], &idata[1], &idata[2]) == 4) {
                    /* copy triangle data */
                    indices[tri_index * 3 + 0] = idata[0];
                    indices[tri_index * 3 + 1] = idata[1];
                    indices[tri_index * 3 + 2] = idata[2];
                } else if (sscanf (buff, " weight %d %d %f ( %f %f %f )",
                                   &w_index, &idata[0], &fdata[3],
                                   &fdata[0], &fdata[1], &fdata[2]) == 6) {
                    /* copy vertex data */
                    SCE_SVertexWeight *weights = SCE_AnimGeom_GetWeights (ageom);
                    SCEvertices *base =
                        SCE_AnimGeom_GetBaseVertices (ageom, SCE_POSITION);
                    weights[w_index].joint_id  = idata[0];
                    weights[w_index].weight    = fdata[3];
                    base[w_index * 4 + 0] = fdata[0] * fdata[3];
                    base[w_index * 4 + 1] = fdata[1] * fdata[3];
                    base[w_index * 4 + 2] = fdata[2] * fdata[3];
                    base[w_index * 4 + 3] = fdata[3];
                }
            }
        }
    }

    SCE_AnimGeom_SetIndices (ageom, n_tris * 3, indices, SCE_TRUE);

    SCE_Skeleton_ComputeMatrices (baseskel, 0);
    SCE_AnimGeom_SetBaseSkeleton (ageom, baseskel, SCE_TRUE);

    goto success;
failure:
    SCE_Skeleton_Delete (baseskel);
    SCE_AnimGeom_Delete (ageom), ageom = NULL;
    SCEE_LogSrc ();
success:
    return ageom;
}


typedef struct
{
    char name[64];
    int parent;
    int flags;
    int startIndex;
} SCE_SMD5JointInfo;

static void SCE_idTechMD5_BuildSkel (SCE_SMD5JointInfo *joint_infos,
                                     SCE_SJoint *base_joints,
                                     const float *anim_frame_data,
                                     SCE_SSkeleton *skel,
                                     int n_joints)
{
    int i;

    for (i = 0; i < n_joints; i++) {
        SCE_TVector3 apos;
        SCE_TQuaternion aorient;
        size_t offset = 0;
        SCE_SJoint *base_joint = &base_joints[i];
        /* NOTE: we assume that this joint's parent has
            already been calculated, i.e. joint's ID should
            never be smaller than its parent ID. */
        SCE_SJoint *this_joint = &skel->joints[i];
        int parent = joint_infos[i].parent;

#if 1
        SCE_Vector3_Copy (apos, base_joint->position);
        SCE_Quaternion_Copy (aorient, base_joint->orientation);
#else
        SCE_Vector3_Set (apos, 0.0, 0.0, 0.0);
        SCE_Quaternion_Identity (aorient);
#endif

        if (joint_infos[i].flags & 1) { /* Tx */
            apos[0] = anim_frame_data[joint_infos[i].startIndex + offset];
            offset++;
        }
        if (joint_infos[i].flags & 2) { /* Ty */
            apos[1] = anim_frame_data[joint_infos[i].startIndex + offset];
            offset++;
        }
        if (joint_infos[i].flags & 4) { /* Tz */
            apos[2] = anim_frame_data[joint_infos[i].startIndex + offset];
            offset++;
        }
        if (joint_infos[i].flags & 8) { /* Qx */
            aorient[0] = anim_frame_data[joint_infos[i].startIndex + offset];
            offset++;
        }
        if (joint_infos[i].flags & 16) { /* Qy */
            aorient[1] = anim_frame_data[joint_infos[i].startIndex + offset];
            offset++;
        }
        if (joint_infos[i].flags & 32) /* Qz */
            aorient[2] = anim_frame_data[joint_infos[i].startIndex + offset];

        SCE_Quaternion_ComputeW (aorient);

        this_joint->parent = parent;

        SCE_Vector3_Copy (this_joint->position, apos);
        SCE_Quaternion_Copy (this_joint->orientation, aorient);
    }
    SCE_Skeleton_ComputeAbsoluteJoints (skel);
    SCE_Skeleton_ComputeMatrices (skel, 0);
}

void* SCE_idTechMD5_LoadAnim (FILE *fp, const char *fname, void *un)
{
    SCE_SAnimation *anim = NULL;
    char buff[512] = {0};
    SCE_SMD5JointInfo *joint_infos = NULL;
    SCE_SJoint *base_joints = NULL;
    float *anim_frame_data = NULL;
    int version;
    int n_animated_components = 0;
    int frame_index = 0;
    int i, frame_rate = 1;
    int n_frames = 0, n_joints = 0;
    SCE_SSkeleton *baseskel = NULL;

    (void)un;
    if (!(anim = SCE_Anim_Create ()))
        goto failure;
    if (!(baseskel = SCE_Skeleton_Create ()))
        goto failure;

    while (!feof (fp)) {
        fgets (buff, sizeof (buff), fp);

        if (sscanf (buff, " MD5Version %d", &version) == 1) {
            if (version != 10)
                goto failure;
        } else if (sscanf (buff, " numFrames %d", &n_frames) == 1) {
/*            if (n_joints > 0) {
                if (SCE_Anim_AllocateKeys (anim, n_frames, n_joints) < 0)
                    goto failure;
            }
*/
        } else if (sscanf (buff, " numJoints %d", &n_joints) == 1) {
            if (n_joints > 0) {
                if (n_frames > 0) {
                    if (SCE_Anim_AllocateKeys (anim, n_frames, n_joints) < 0)
                        goto failure;
                }

                /* allocate temporary memory for building skeleton frames */
                joint_infos = SCE_malloc (n_joints * sizeof *joint_infos);
                base_joints = SCE_malloc (n_joints * sizeof *base_joints);
            }
        } else if (sscanf (buff, " frameRate %d", &frame_rate) == 1) {
        } else if (sscanf (buff, " numAnimatedComponents %d",
                           &n_animated_components) == 1) {
            if (n_animated_components > 0) {
                int j;
                anim_frame_data = SCE_malloc (n_animated_components *
                                              sizeof *anim_frame_data);
                for (j = 0; j < n_animated_components; j++)
                    anim_frame_data[j] = 0.0;
            }
        } else if (strncmp (buff, "hierarchy {", 11) == 0) {
            for (i = 0; i < n_joints; ++i) {
                fgets (buff, sizeof (buff), fp);
                sscanf (buff, " %s %d %d %d", joint_infos[i].name,
                        &joint_infos[i].parent, &joint_infos[i].flags,
                        &joint_infos[i].startIndex);
            }
        } else if (strncmp (buff, "bounds {", 8) == 0) {
            for (i = 0; i < n_frames; ++i) {
                fgets (buff, sizeof (buff), fp);
/*                sscanf (buff, " ( %f %f %f ) ( %f %f %f )",
                        &anim->bboxes[i].min[0], &anim->bboxes[i].min[1],
                        &anim->bboxes[i].min[2], &anim->bboxes[i].max[0],
                        &anim->bboxes[i].max[1], &anim->bboxes[i].max[2]);
*/
            }
        } else if (strncmp (buff, "baseframe {", 11) == 0) {
            for (i = 0; i < n_joints; ++i) {
                fgets (buff, sizeof (buff), fp);
                if (sscanf (buff, " ( %f %f %f ) ( %f %f %f )",
                     &base_joints[i].position[0], &base_joints[i].position[1],
                     &base_joints[i].position[2],&base_joints[i].orientation[0],
                     &base_joints[i].orientation[1],
                     &base_joints[i].orientation[2]) == 6)
                    SCE_Quaternion_ComputeW (base_joints[i].orientation);
            }
        } else if (sscanf (buff, " frame %d", &frame_index) == 1) {
            SCE_SSkeleton **keys = SCE_Anim_GetKeys (anim);
            for (i = 0; i < n_animated_components; ++i)
                fscanf (fp, "%f", &anim_frame_data[i]);
            /* build frame skeleton from the collected data */
            SCE_idTechMD5_BuildSkel (joint_infos, base_joints, anim_frame_data,
                                     keys[frame_index], n_joints);
        }
    }

    SCE_Anim_SetFrequency (anim, frame_rate);
    SCE_Skeleton_SetJoints (baseskel, base_joints, n_joints);
    SCE_Anim_SetBaseSkeleton (anim, baseskel, SCE_TRUE);
    goto success;
failure:
    SCE_Skeleton_Delete (baseskel);
    SCE_Anim_Delete (anim), anim = NULL;
    SCEE_LogSrc ();
success:
    SCE_free (joint_infos);
    SCE_free (anim_frame_data);
    return anim;
}

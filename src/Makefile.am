lib_LTLIBRARIES = libscecore.la

libscecore_la_CPPFLAGS  = -I$(srcdir)/../include
libscecore_la_CFLAGS    = @SCE_UTILS_CFLAGS@ \
                          @PTHREAD_CFLAGS@ \
                          @IL_CFLAGS@ \
                          @ILU_CFLAGS@ \
                          @SCE_DEBUG_CFLAGS@ \
                          @SCE_DEBUG_CFLAGS_EXPORT@
libscecore_la_LIBADD    = @SCE_UTILS_LIBS@ \
                          @PTHREAD_LIBS@ \
                          @IL_LIBS@ \
                          @ILU_LIBS@
libscecore_la_LDFLAGS   = -version-info @SCE_CORE_LTVERSION@
libscecore_la_SOURCES   = libwar.c \
                          lib4fm.c \
                          SCEBox.c \
                          SCESphere.c \
                          SCECone.c \
                          SCEBoundingBox.c \
                          SCEBoundingSphere.c \
                          SCECollide.c \
                          SCEFrustum.c \
                          SCEOctree.c \
                          SCENode.c \
                          SCECamera.c \
                          SCELevelOfDetail.c \
                          SCEImage.c \
                          SCETextureData.c \
                          SCEGeometry.c \
                          SCEGrid.c \
                          SCEVoxelStore.c \
                          SCEMarchingTetrahedra.c \
                          SCEForestTree.c \
                          SCEOBJLoader.c \
                          SCESphereGeometry.c \
                          SCEBoxGeometry.c \
                          SCEConeGeometry.c \
                          SCEParticle.c \
                          SCEParticleEmitter.c \
                          SCEParticleModifier.c \
                          SCEParticleProcessor.c \
                          SCEParticleSystem.c \
                          SCEJoint.c \
                          SCESkeleton.c \
                          SCEAnimatedGeometry.c \
                          SCEAnimation.c \
                          SCEMD5Loader.c \
                          SCECore.c

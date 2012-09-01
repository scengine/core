dnl Process this file with autoconf to produce a configure script.

dnl prelude
AC_PREREQ([2.60])
AC_INIT([SCEngine core library],
        [0.1.0],
        [martin.antony@yahoo.fr],
        [scecore])
AC_CONFIG_SRCDIR([src/SCECore.c])
AC_CONFIG_AUX_DIR([build/aux])
AC_CONFIG_MACRO_DIR([build/m4])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl check for programs
m4_ifdef([AM_PROG_AR], [AM_PROG_AR]) dnl I hate you Automake 1.12
LT_PREREQ([2.2.0])
LT_INIT
AC_PROG_CC

SCE_CHECK_DOXYGEN

dnl basic setup

dnl
dnl revision of the library
dnl
dnl CURRENT:REVISION:AGE
dnl
dnl Please remeber to bump library version before releases
dnl   Remeberer:
dnl =====================================================================
dnl |   If you have...                              |   Bump version to |
dnl =====================================================================
dnl | Not changed the interface (bug fixes)         | CURRENT:REV+1:AGE |
dnl ---------------------------------------------------------------------
dnl | Augmented the interface (new functions)       | CURRENT+1:0:AGE+1 |
dnl ---------------------------------------------------------------------
dnl | Broken old interface (e.g. removed functions) | CURRENT+1:0:0     |
dnl ---------------------------------------------------------------------
SCE_CORE_LTVERSION="0:0:0"
AC_SUBST([SCE_CORE_LTVERSION])

pkgconfigdir="$libdir/pkgconfig"
sce_includedir="$includedir/SCE"
sce_include_coredir="$sce_includedir/core"
AC_SUBST(pkgconfigdir)
AC_SUBST(sce_includedir)
AC_SUBST(sce_include_coredir)

dnl legacy version names
AC_DEFINE([SCE_CORE_VERSION_STRING], [VERSION], [Version of the package])
AC_DEFINE([FFM_VERSION_STRING], ["0.1"], [Version of the FFM sublib])

dnl check for libs & headers

SCE_CHECK_PTHREAD
SCE_CHECK_IL
SCE_CHECK_ILU
PKG_CHECK_MODULES([SCE_UTILS], [sceutils])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
SCE_REQUIRE_FUNCS([memset strtol strtoul])

SCE_CHECK_DEBUG

dnl output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 Doxyfile
                 doc/Makefile
                 src/Makefile
                 include/Makefile
                 include/SCE/Makefile
                 include/SCE/core/Makefile
                 scecore.pc
                 ])
AC_OUTPUT

echo "------------------------------------------"
echo "SCECore version                : $VERSION ($SCE_CORE_LTVERSION)"
echo "Debugging enabled              : $enable_debug"
echo "Base installation directory    : $prefix"
echo ""
echo "Configuration succeed."
echo ""

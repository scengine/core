dnl SCE_CHECK_ILU
dnl -----------------
dnl Checks for OpenIL utils and fails if not found
AC_DEFUN([SCE_CHECK_ILU],
[
    SCE_REQUIRE_HEADER([IL/ilu.h])
    SCE_REQUIRE_LIB([ILU], [iluInit])
    dnl those are fake flags in case they will become useful, but currently
    dnl AC_CHECK_LIB (called through SCE_REQUIRE_LIB) fills LIBS as needed
    AC_SUBST([ILU_CFLAGS], [])
    AC_SUBST([ILU_LIBS], [])
])

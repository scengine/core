dnl SCE_CHECK_IL
dnl -----------------
dnl Checks for OpenIL and fails if not found
AC_DEFUN([SCE_CHECK_IL],
[
    SCE_REQUIRE_HEADER([IL/il.h])
    SCE_REQUIRE_LIB([IL], [ilInit])
    dnl those are fake flags in case they will become useful, but currently
    dnl AC_CHECK_LIB (called through SCE_REQUIRE_LIB) fills LIBS as needed
    AC_SUBST([IL_CFLAGS], [])
    AC_SUBST([IL_LIBS], [])
])

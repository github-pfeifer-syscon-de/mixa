dnl Process this file with autoconf to produce a configure script.

AC_CONFIG_MACRO_DIR([m4])

AC_INIT([mixa],[0.1],[mixa@pfeifer-syscon.de])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CXX
AC_PROG_CC
AX_CXX_COMPILE_STDCXX([17], [noext], [optional])

LT_INIT

PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])

PKG_CHECK_MODULES(GLIBMM, [glibmm-2.4])

PKG_CHECK_MODULES(GTKMM, [gtkmm-3.0])

PKG_CHECK_MODULES(ALSA, [alsa], [AC_DEFINE([ALSA], [1], [Using alsa])])

AC_ARG_WITH([keybinder], AS_HELP_STRING([--with-keybinder], [Build with multimedia key support]))

AM_CONDITIONAL([KEYBINDER], [test "x$with_keybinder" = "xyes"])

AS_IF([test "x$with_keybinder" = "xyes"],
   [
    PKG_CHECK_MODULES(KEYBINDER, [keybinder-3.0], [AC_DEFINE([KEYBINDER], [1], [Using keybinder])])
   ]
)

AX_CHECK_SED()

AC_CONFIG_FILES([
Makefile
src/Makefile
res/Makefile
])
AC_OUTPUT

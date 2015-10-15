AC_PREREQ([2.69])
AC_INIT([bbbm], [0.8], [bbbm@icemanx.nl])
AC_COPYRIGHT([Copyright (C) 2004-2015 Rob Spoor])
AC_DEFINE([BBBM_AUTHOR], "Rob Spoor", [Author])
AC_DEFINE([BBBM_COPYRIGHT], "Copyright (C) 2004-2015 Rob Spoor", [Copyright notice])
AC_DEFINE([BBBM_HOME_DIR], ".bbbm", [BBBM home directory relative to ~])
AC_DEFINE([BBBM_CONFIG_FILE], "bbbm.xml", [BBBM config file relative to BBBM_HOME_DIR])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/bbbm.h])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

PKG_CHECK_MODULES([GTK], [gtk+-2.0])

AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h sys/stat.h sys/wait.h getopt.h])

AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_FUNC_FORK
AC_FUNC_STRTOLD
AC_FUNC_STRTOD
AC_CHECK_FUNCS([mkdir getopt_long memset strstr strcasecmp])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

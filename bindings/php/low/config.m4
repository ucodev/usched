PHP_ARG_ENABLE(usc, whether to enable uSched client support, [ --enable-usc   Enable uSched client support ])

if test "$PHP_USC" = "yes"; then
	AC_DEFINE(HAVE_USC, 1, [Whether you have uSched client])
	PHP_NEW_EXTENSION(usc, usc.c, $ext_shared)
fi

LIBNAME=usc
LIBSYMBOL=usched_init

PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
	[
		PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $USC_DIR/lib, USC_SHARED_LIBADD)
		AC_DEFINE(HAVE_USCLIB, 1, [ ])
	], [
		AC_MSG_ERROR([wrong $LIBNAME lib version or lib not found])
	], [
		-L$USC_DIR/lib -ldl -lusc
	]
)



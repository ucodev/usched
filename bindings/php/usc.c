#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_usc.h"

#include <usched/lib.h>

static zend_function_entry usc_functions[] = {
	PHP_FE(usc_test, NULL)
	{ NULL, NULL, NULL}
};

zend_module_entry usc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_USC_EXTNAME, /* mod name */
	usc_functions, /* mod funcs */
	PHP_MINIT(usc_init), /* init */
	PHP_MSHUTDOWN(usc_shutdown), /* shutdown */
	NULL, /* req init */
	NULL, /* req shutdown */
	NULL, /* mod info */
#if ZEND_MODULE_API_NO >= 20010901
	PHP_USC_VERSION, /* version */
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_USC
ZEND_GET_MODULE(usc)
#endif

PHP_MINIT_FUNCTION(usc_init) {
	usched_init();

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(usc_shutdown) {
	usched_destroy();

	return SUCCESS;
}

PHP_FUNCTION(usc_test) {
	RETURN_STRING("Testing uSched interface...", 1);
}


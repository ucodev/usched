#ifndef PHP_USC_H
#define PHP_USC_H 1

#define PHP_USC_VERSION "1.0"
#define PHP_USC_EXTNAME "usc"

PHP_MINIT_FUNCTION(usc_init);
PHP_MSHUTDOWN_FUNCTION(usc_shutdown);
PHP_FUNCTION(usc_test);

extern zend_module_entry usc_module_entry;
#define phpext_usc_ptr &usc_module_entry

#endif


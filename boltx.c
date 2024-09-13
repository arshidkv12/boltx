/* boltx extension for PHP */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_boltx.h"
#include "boltx_arginfo.h"
#include "hook.h"


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "boltx");
}
/* }}} */

/* {{{ string _boltx_unique_id( [ string $var ] ) */
PHP_FUNCTION(_boltx_unique_id)
{
	zval *callable;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_STR( _boltx_unique_id( callable ) );
}
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(boltx)
{
#if defined(ZTS) && defined(COMPILE_DL_BOLTX)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(boltx)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "boltx support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINIT_FUNCTION(boltx) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Hook", hook_methods);
    hook_ce = zend_register_internal_class(&ce);

    zend_declare_property_null(hook_ce, "actions", sizeof("actions") - 1, ZEND_ACC_PROTECTED);
    zend_declare_property_null(hook_ce, "filters", sizeof("filters") - 1, ZEND_ACC_PUBLIC);

    return SUCCESS;
}
/* }}} */

/* {{{ boltx_module_entry */
zend_module_entry boltx_module_entry = {
	STANDARD_MODULE_HEADER,
	"boltx",					/* Extension name */
	ext_functions,					/* zend_function_entry */
	PHP_MINIT(boltx),							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(boltx),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(boltx),			/* PHP_MINFO - Module info */
	PHP_BOLTX_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BOLTX
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(boltx)
#endif

/* boltx extension for PHP */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_boltx.h"
#include "boltx_arginfo.h"


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
	zval arr;  
	HashTable *arr_hash;
	HashPosition pos; 
	// zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	if( Z_TYPE_P(callable) == IS_STRING ){
		RETURN_STR( Z_STR_P(callable) );
	}

	if( Z_TYPE_P(callable) == IS_OBJECT ){
		array_init(&arr);  
		GC_ADDREF(Z_OBJ_P(callable));
		add_next_index_object(&arr,  Z_OBJ_P(callable) );
		add_next_index_string(&arr, "");
		arr_hash = Z_ARRVAL_P(&arr);
	}else{
		convert_to_array( callable );
		arr_hash = Z_ARRVAL_P(callable);
	}

	zend_hash_internal_pointer_reset_ex(arr_hash, &pos);
	zval *first_el  = zend_hash_get_current_data_ex(arr_hash, &pos);
	zend_hash_move_forward_ex(arr_hash, &pos);
	zval *second_el = zend_hash_get_current_data_ex(arr_hash, &pos); 
		
	if( Z_TYPE_P(first_el) == IS_OBJECT ){
		RETVAL_STR( strpprintf(0, "%016zx0000000000000000%s", (intptr_t)Z_OBJ_P(first_el)->handle, Z_STR_P(second_el)->val) );
	}else if( Z_TYPE_P(first_el) == IS_STRING ){
		RETVAL_STR( strpprintf(0, "%s::%s", Z_STR_P(first_el)->val, Z_STR_P(second_el)->val ) );
	}
	zend_hash_destroy( arr_hash );
    FREE_HASHTABLE(arr_hash);

	return;
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

/* {{{ boltx_module_entry */
zend_module_entry boltx_module_entry = {
	STANDARD_MODULE_HEADER,
	"boltx",					/* Extension name */
	ext_functions,					/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
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

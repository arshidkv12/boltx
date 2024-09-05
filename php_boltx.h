/* boltx extension for PHP */

#ifndef PHP_BOLTX_H
# define PHP_BOLTX_H

extern zend_module_entry boltx_module_entry;
# define phpext_boltx_ptr &boltx_module_entry

# define PHP_BOLTX_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_BOLTX)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_BOLTX_H */

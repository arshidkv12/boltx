
#ifndef BOLTX_ARRAY
#define BOLTX_ARRAY

bool boltx_ksort(zval *array, zend_long sort_type);
zend_long boltx_count(zval *array, zend_long mode);
zval *boltx_array_keys(zval *input, zval *search_value, bool strict);

#endif /* BOLTX_ARRAY */


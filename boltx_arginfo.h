/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 54b0ffc3af871b189435266df516f7575c1b9675 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_test1, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

// ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo__boltx_unique_id, 0, 0, IS_STRING, 0)
// 	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, str, IS_STRING, 0, "\"\"")
// ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo__boltx_unique_id, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(test1);
ZEND_FUNCTION(_boltx_unique_id);

static const zend_function_entry ext_functions[] = {
	ZEND_FE(test1, arginfo_test1)
	ZEND_FE(_boltx_unique_id, arginfo__boltx_unique_id)
	ZEND_FE_END
};

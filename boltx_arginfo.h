/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 51130d8547a2c7e47ec5db6b87015458ff09ac8f */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_test1, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo__boltx_unique_id, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(test1);
ZEND_FUNCTION(_boltx_unique_id);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(test1, arginfo_test1)
	ZEND_FE(_boltx_unique_id, arginfo__boltx_unique_id)
	ZEND_FE_END
};


PHP_METHOD(Hook, __construct);
PHP_METHOD(Hook, __destruct);
PHP_METHOD(Hook, add_filter);
// PHP_METHOD(Hook, setMake);
PHP_METHOD(Hook, getData);
PHP_METHOD(Hook, call_function);

ZEND_BEGIN_ARG_INFO_EX(arginfo_Hook__construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Hook_getData, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Hook_setMake, 0, 0, 1)
    ZEND_ARG_INFO(0, make)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_Hook__destruct, 0, 0, 0)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Hook_add_filter, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, hook_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_LONG, 0, "10")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, accepted_args, IS_LONG, 0, "1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_call_function, 0, 0, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, args, IS_MIXED, 0)
	// ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, args, IS_MIXED, 0, NULL)
ZEND_END_ARG_INFO()

static const zend_function_entry hook_methods[] = {
    // PHP_ME(Hook, __construct, arginfo_Hook__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Hook, __destruct, arginfo_Hook__destruct, ZEND_ACC_PUBLIC)
    PHP_ME(Hook, add_filter, arginfo_Hook_add_filter, ZEND_ACC_PUBLIC)
    // PHP_ME(Hook, setMake, arginfo_Hook_setMake, ZEND_ACC_PUBLIC)
    PHP_ME(Hook, getData, arginfo_Hook_getData, ZEND_ACC_PUBLIC)
    PHP_ME(Hook, call_function, arginfo_call_function, ZEND_ACC_PUBLIC)
    PHP_FE_END
};





#include "php.h"
#include "hook.h"
#include "boltx_arginfo.h"


// Define the __construct method
PHP_METHOD(Hook, __construct) {
    
    zval callbacks;
    array_init(&callbacks);  
    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, &callbacks);
    zval_ptr_dtor(&callbacks);
}

PHP_METHOD(Hook, __destruct) {
    
    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks") - 1, 1, NULL);
    if (!Z_ISUNDEF_P(callbacks)) {
		zval_ptr_dtor(callbacks);
		ZVAL_UNDEF(callbacks);
	}
}

// Define the displayDetails method
PHP_METHOD(Hook, add_action) {

    zval *callback, arr_1, arr_2, _callbacks;
    zend_string *hook_name;
    zend_long priority = 10, accepted_args =1;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_STR(hook_name)
		Z_PARAM_ZVAL(callback)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(priority)
        Z_PARAM_LONG(accepted_args)
	ZEND_PARSE_PARAMETERS_END();
     
    zend_string *idx = _boltx_unique_id(callback);

    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks") - 1, 1, NULL);

    array_init(&arr_1);
    array_init(&arr_2);
    array_init(&_callbacks);

    Z_ARRVAL(_callbacks) = zend_array_dup(Z_ARRVAL_P(callbacks));

    add_assoc_zval(&arr_1, "function", callback);
    add_assoc_long(&arr_1, "accepted_args", accepted_args);

    add_assoc_zval(&arr_2, idx->val, &arr_1);
    add_index_zval(&_callbacks, priority, &arr_2);

    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, &_callbacks);
    zval_ptr_dtor(&_callbacks);
}

// Define the setMake method
PHP_METHOD(Hook, setMake) {
    char *make;
    size_t make_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &make, &make_len) == FAILURE) {
        RETURN_NULL();
    }

    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks") - 1, 1, NULL);
    RETURN_ZVAL(callbacks, 0,0);
}


zend_string *_boltx_unique_id(zval *callable){
	zval arr;  
	HashTable *arr_hash;
	HashPosition pos; 

	if( Z_TYPE_P(callable) == IS_STRING ){
		return( Z_STR_P(callable) );
	}

	if(  ! i_zend_is_true(callable) ){
		return zend_string_init("", 0, 0);
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
	
    zend_string *retval;
	if( Z_TYPE_P(first_el) == IS_OBJECT ){
		retval = strpprintf(0, "%016zx0000000000000000%s", (intptr_t)Z_OBJ_P(first_el)->handle, Z_STR_P(second_el)->val);
	}else if( Z_TYPE_P(first_el) == IS_STRING ){
		retval = strpprintf(0, "%s::%s", Z_STR_P(first_el)->val, Z_STR_P(second_el)->val );
	}
	
	if( Z_TYPE_P(callable) == IS_OBJECT ){ 
		zend_hash_destroy(arr_hash);
		FREE_HASHTABLE(arr_hash);
	}
    return retval;
}

	

 
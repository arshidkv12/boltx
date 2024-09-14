#include "php.h"
#include "hook.h"
#include "boltx_arginfo.h"
#include "array.h"
#include "ext/standard/php_array.h"


// Define the __construct method
PHP_METHOD(Hook, __construct) {
    
    zval callbacks, priorities, iterations, current_priority;

    array_init(&callbacks);  
    array_init(&priorities);  
    array_init(&iterations);  
    array_init(&current_priority);  

    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, &callbacks);
    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "priorities", sizeof("priorities")-1, &priorities);
    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "iterations", sizeof("iterations")-1, &iterations);
    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "current_priority", sizeof("current_priority")-1, &current_priority);
    zend_update_property_long(hook_ce, Z_OBJ_P(getThis()), "nesting_level", sizeof("nesting_level")-1, 0);
    zend_update_property_bool(hook_ce, Z_OBJ_P(getThis()), "doing_action", sizeof("doing_action")-1, false);
    
    zval_ptr_dtor(&callbacks);
    zval_ptr_dtor(&priorities);
    zval_ptr_dtor(&iterations);
    zval_ptr_dtor(&current_priority);
}

PHP_METHOD(Hook, __destruct) {
    
    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks") - 1, 1, NULL);
    // if (Z_REFCOUNTED_P(callbacks) && Z_REFCOUNT_P(callbacks) > 0) {
	// 	zval_ptr_dtor(callbacks);
	// }
	
}

// Define the displayDetails method
PHP_METHOD(Hook, add_filter) {

    zval *callback, *tmp, arr_1, arr_2, _callbacks;
    zend_string *hook_name, *arr_key;
    zend_long priority = 10, accepted_args =1;
    HashTable *ht_callback;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_STR(hook_name)
		Z_PARAM_ZVAL(callback)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(priority)
        Z_PARAM_LONG(accepted_args)
	ZEND_PARSE_PARAMETERS_END();
     
    zend_string *idx = _boltx_unique_id(callback);

    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks") - 1, 1, NULL);
    
    ht_callback = Z_ARRVAL_P(callbacks);
    bool priority_existed = zend_hash_index_exists(ht_callback, priority);
    
    array_init(&arr_1);

    ZVAL_ARR(&_callbacks, zend_array_dup(Z_ARRVAL_P(callbacks)));

    if ((tmp = zend_hash_index_find(Z_ARRVAL_P(callbacks), priority)) != NULL) {
        ZVAL_ARR(&arr_2, zend_array_dup(Z_ARRVAL_P(tmp)));
    }else{
        array_init(&arr_2);
    }

    add_assoc_zval(&arr_1, "function", callback);
    add_assoc_long(&arr_1, "accepted_args", accepted_args);

    add_assoc_zval(&arr_2, idx->val, &arr_1);
    add_index_zval(&_callbacks, priority, &arr_2);

    if ( ! priority_existed && boltx_count( &_callbacks, PHP_COUNT_NORMAL ) > 1 ) {
        boltx_ksort( &_callbacks, PHP_SORT_REGULAR );
    }

    // $this->priorities = array_keys( $this->callbacks );
    zval priorities;
    boltx_array_keys( &priorities, &_callbacks, NULL, 0 );

    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, &_callbacks);
    zval_ptr_dtor(&_callbacks);
    zend_string_release(arr_key);
    zval_ptr_dtor(&priorities);
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

	

 
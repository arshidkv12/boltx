#include "php.h"
#include "hook.h"
#include "boltx_arginfo.h"
#include "array.h"
#include "ext/standard/php_array.h"
#include "ext/standard/php_var.h"


typedef struct {
    zval *function;
    zend_long accepted_args;
} callbacksData;

void callbacks_data_dtor(zval *zv) {
    callbacksData *data = (callbacksData *) Z_PTR_P(zv);
                
    if (data->function) {
        // Decrement the reference count of the function if necessary
        zval_ptr_dtor(data->function);
        efree(data->function);
    }
    efree(data);
}

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


void iterate_two_dimensional_array(zval *callbacks)
{
    zval *first_level_value;
    zend_string *first_level_key;
    zend_long first_level_index;
    
    // Ensure the zval is an array
    if (Z_TYPE_P(callbacks) != IS_ARRAY) {
        php_error_docref(NULL, E_WARNING, "Expected an array");
        return;
    }


    // Iterate over the first-level array
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(callbacks), first_level_index, first_level_key, first_level_value) {

        if (Z_TYPE_P(first_level_value) == IS_ARRAY) {
            // Inner loop to iterate over the second-level array
            zval *second_level_value;
            zend_string *second_level_key;
            zend_long second_level_index;

            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(first_level_value), second_level_index, second_level_key, second_level_value) {

            callbacksData *data;

            data = (callbacksData *) Z_PTR_P(second_level_value);
            php_printf("+++%s---", Z_STR_P(data->function)->val);
            php_printf("soop%lld---", data->accepted_args);
                
                
            } ZEND_HASH_FOREACH_END();
        }
    } ZEND_HASH_FOREACH_END();
}


PHP_METHOD(Hook, call_function) {
    zval *hook_data, *args, *val, retval;
    callbacksData *data;
    zend_long  priority, index_val;
    zend_string *key_val;

    ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(priority)
		Z_PARAM_ZVAL(args)
	ZEND_PARSE_PARAMETERS_END();  

    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, 1, NULL);

    zval *the_ = zend_hash_index_find(Z_ARRVAL_P(callbacks), priority);
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(the_), index_val, key_val, val) {

    data = (callbacksData *) Z_PTR_P(val);
        // php_printf("+++%s---", Z_STR_P(data->function)->val);
        call_user_function(EG(function_table), NULL, (data->function), &retval, 0, args);
    }ZEND_HASH_FOREACH_END();
    
//  iterate_two_dimensional_array(callbacks);
}

PHP_METHOD(Hook, getData) {
    
    zval *callbacks = zend_read_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, 1, NULL);
    // if (Z_REFCOUNTED_P(callbacks) && Z_REFCOUNT_P(callbacks) > 0) {
	// 	zval_ptr_dtor(callbacks);
	// }

    iterate_two_dimensional_array(callbacks);

    // php_var_dump(callbacks, 1);
    // FREE_HASHTABLE(Z_ARRVAL_P(callbacks));
	
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

    if(Z_TYPE_P(callbacks) == IS_NULL){
        array_init(callbacks);
    }

    ht_callback = Z_ARRVAL_P(callbacks);
    bool priority_existed = zend_hash_index_exists(ht_callback, priority);
    
    // array_init(&arr_1);


    ZVAL_ARR(&_callbacks, zend_array_dup(Z_ARRVAL_P(callbacks)));

    if ((tmp = zend_hash_index_find(Z_ARRVAL_P(callbacks), priority)) != NULL) {
        ZVAL_ARR(&arr_2, zend_array_dup(Z_ARRVAL_P(tmp)));
    }else{
        array_init(&arr_2);
    }

    Z_TRY_ADDREF_P(callback);
    callbacksData *data = emalloc(sizeof(callbacksData));
    data->function = emalloc(sizeof(zval)); // Allocate memory for zval
    ZVAL_COPY(data->function, callback);
    // data->function = callback;   
    data->accepted_args = accepted_args;

    // zval data_zval;
    // ZVAL_PTR(&data_zval, data);    
    // zend_hash_add_mem(Z_ARRVAL_P(&arr_1), idx, data, sizeof(callbacksData));
    // add_assoc_zval(&arr_1, "function", callback);
    // add_assoc_long(&arr_1, "accepted_args", accepted_args);

    // add_assoc_zval(&arr_2, idx->val, &arr_1);
    zend_hash_add_mem(Z_ARRVAL_P(&arr_2), idx, data, sizeof(callbacksData));
    efree(data);
    Z_ARRVAL_P(&arr_2)->pDestructor = callbacks_data_dtor;
    add_index_zval(&_callbacks, priority, &arr_2);

    if ( ! priority_existed && boltx_count( &_callbacks, PHP_COUNT_NORMAL ) > 1 ) {
        boltx_ksort( &_callbacks, PHP_SORT_REGULAR );
    }

    zval priorities;
    boltx_array_keys( &priorities, &_callbacks, NULL, 0 );
    //php_var_dump(&priorities, 1);

    zend_update_property(hook_ce, Z_OBJ_P(getThis()), "callbacks", sizeof("callbacks")-1, &_callbacks);
    zval_ptr_dtor(&_callbacks);
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

	

 
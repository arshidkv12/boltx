/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@php.net>                                 |
   |          Zeev Suraski <zeev@php.net>                                 |
   |          Rasmus Lerdorf <rasmus@php.net>                             |
   |          Andrei Zmievski <andrei@php.net>                            |
   |          Stig Venaas <venaas@php.net>                                |
   |          Jason Greene <jason@php.net>                                |
   +----------------------------------------------------------------------+
*/

#include "php.h"
#include "php_ini.h"
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#ifdef PHP_WIN32
#include "win32/unistd.h"
#endif
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "php_globals.h"
#include "ext/standard/php_array.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_math.h"
#include "zend_smart_str.h"
#include "zend_bitset.h"
#include "zend_exceptions.h"
#include "ext/spl/spl_array.h"
#include "ext/random/php_random.h"

/* {{{ defines */

#define DIFF_NORMAL			1
#define DIFF_KEY			2
#define DIFF_ASSOC			6
#define DIFF_COMP_DATA_NONE    -1
#define DIFF_COMP_DATA_INTERNAL 0
#define DIFF_COMP_DATA_USER     1
#define DIFF_COMP_KEY_INTERNAL  0
#define DIFF_COMP_KEY_USER      1

#define INTERSECT_NORMAL		1
#define INTERSECT_KEY			2
#define INTERSECT_ASSOC			6
#define INTERSECT_COMP_DATA_NONE    -1
#define INTERSECT_COMP_DATA_INTERNAL 0
#define INTERSECT_COMP_DATA_USER     1
#define INTERSECT_COMP_KEY_INTERNAL  0
#define INTERSECT_COMP_KEY_USER      1
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(array)

/* {{{ php_array_init_globals */
static void php_array_init_globals(zend_array_globals *array_globals)
{
	memset(array_globals, 0, sizeof(zend_array_globals));
}
/* }}} */


static zend_never_inline ZEND_COLD int stable_sort_fallback(Bucket *a, Bucket *b) {
	if (Z_EXTRA(a->val) > Z_EXTRA(b->val)) {
		return 1;
	} else if (Z_EXTRA(a->val) < Z_EXTRA(b->val)) {
		return -1;
	} else {
		return 0;
	}
}

#define RETURN_STABLE_SORT(a, b, result) do { \
	int _result = (result); \
	if (EXPECTED(_result)) { \
		return _result; \
	} \
	return stable_sort_fallback((a), (b)); \
} while (0)

/* Generate inlined unstable and stable variants, and non-inlined reversed variants. */
#define DEFINE_SORT_VARIANTS(name) \
	static zend_never_inline int php_array_##name##_unstable(Bucket *a, Bucket *b) { \
		return php_array_##name##_unstable_i(a, b); \
	} \
	static zend_never_inline int php_array_##name(Bucket *a, Bucket *b) { \
		RETURN_STABLE_SORT(a, b, php_array_##name##_unstable_i(a, b)); \
	} \
	static zend_never_inline int php_array_reverse_##name##_unstable(Bucket *a, Bucket *b) { \
		return php_array_##name##_unstable(a, b) * -1; \
	} \
	static zend_never_inline int php_array_reverse_##name(Bucket *a, Bucket *b) { \
		RETURN_STABLE_SORT(a, b, php_array_reverse_##name##_unstable(a, b)); \
	} \

static zend_always_inline int php_array_key_compare_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	zval first;
	zval second;

	if (f->key == NULL && s->key == NULL) {
		return (zend_long)f->h > (zend_long)s->h ? 1 : -1;
	} else if (f->key && s->key) {
		return zendi_smart_strcmp(f->key, s->key);
	}
	if (f->key) {
		ZVAL_STR(&first, f->key);
	} else {
		ZVAL_LONG(&first, f->h);
	}
	if (s->key) {
		ZVAL_STR(&second, s->key);
	} else {
		ZVAL_LONG(&second, s->h);
	}
	return zend_compare(&first, &second);
}
/* }}} */

static zend_always_inline int php_array_key_compare_numeric_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	if (f->key == NULL && s->key == NULL) {
		return (zend_long)f->h > (zend_long)s->h ? 1 : -1;
	} else {
		double d1, d2;
		if (f->key) {
			d1 = zend_strtod(f->key->val, NULL);
		} else {
			d1 = (double)(zend_long)f->h;
		}
		if (s->key) {
			d2 = zend_strtod(s->key->val, NULL);
		} else {
			d2 = (double)(zend_long)s->h;
		}
		return ZEND_THREEWAY_COMPARE(d1, d2);
	}
}
/* }}} */

static zend_always_inline int php_array_key_compare_string_case_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	const char *s1, *s2;
	size_t l1, l2;
	char buf1[MAX_LENGTH_OF_LONG + 1];
	char buf2[MAX_LENGTH_OF_LONG + 1];

	if (f->key) {
		s1 = f->key->val;
		l1 = f->key->len;
	} else {
		s1 = zend_print_long_to_buf(buf1 + sizeof(buf1) - 1, f->h);
		l1 = buf1 + sizeof(buf1) - 1 - s1;
	}
	if (s->key) {
		s2 = s->key->val;
		l2 = s->key->len;
	} else {
		s2 = zend_print_long_to_buf(buf2 + sizeof(buf2) - 1, s->h);
		l2 = buf2 + sizeof(buf2) - 1 - s2;
	}
	return zend_binary_strcasecmp_l(s1, l1, s2, l2);
}
/* }}} */

static zend_always_inline int php_array_key_compare_string_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	const char *s1, *s2;
	size_t l1, l2;
	char buf1[MAX_LENGTH_OF_LONG + 1];
	char buf2[MAX_LENGTH_OF_LONG + 1];

	if (f->key) {
		s1 = f->key->val;
		l1 = f->key->len;
	} else {
		s1 = zend_print_long_to_buf(buf1 + sizeof(buf1) - 1, f->h);
		l1 = buf1 + sizeof(buf1) - 1 - s1;
	}
	if (s->key) {
		s2 = s->key->val;
		l2 = s->key->len;
	} else {
		s2 = zend_print_long_to_buf(buf2 + sizeof(buf2) - 1, s->h);
		l2 = buf2 + sizeof(buf2) - 1 - s2;
	}
	return zend_binary_strcmp(s1, l1, s2, l2);
}
/* }}} */

static int php_array_key_compare_string_natural_general(Bucket *f, Bucket *s, int fold_case) /* {{{ */
{
	const char *s1, *s2;
	size_t l1, l2;
	char buf1[MAX_LENGTH_OF_LONG + 1];
	char buf2[MAX_LENGTH_OF_LONG + 1];

	if (f->key) {
		s1 = f->key->val;
		l1 = f->key->len;
	} else {
		s1 = zend_print_long_to_buf(buf1 + sizeof(buf1) - 1, f->h);
		l1 = buf1 + sizeof(buf1) - 1 - s1;
	}
	if (s->key) {
		s2 = s->key->val;
		l2 = s->key->len;
	} else {
		s2 = zend_print_long_to_buf(buf2 + sizeof(buf2) - 1, s->h);
		l2 = buf2 + sizeof(buf2) - 1 - s2;
	}
	return strnatcmp_ex(s1, l1, s2, l2, fold_case);
}
/* }}} */

static int php_array_key_compare_string_natural_case(Bucket *a, Bucket *b) /* {{{ */
{
	RETURN_STABLE_SORT(a, b, php_array_key_compare_string_natural_general(a, b, 1));
}
/* }}} */

static int php_array_reverse_key_compare_string_natural_case(Bucket *a, Bucket *b) /* {{{ */
{
	RETURN_STABLE_SORT(a, b, php_array_key_compare_string_natural_general(b, a, 1));
}
/* }}} */

static int php_array_key_compare_string_natural(Bucket *a, Bucket *b) /* {{{ */
{
	RETURN_STABLE_SORT(a, b, php_array_key_compare_string_natural_general(a, b, 0));
}
/* }}} */

static int php_array_reverse_key_compare_string_natural(Bucket *a, Bucket *b) /* {{{ */
{
	RETURN_STABLE_SORT(a, b, php_array_key_compare_string_natural_general(b, a, 0));
}
/* }}} */

static zend_always_inline int php_array_key_compare_string_locale_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	const char *s1, *s2;
	char buf1[MAX_LENGTH_OF_LONG + 1];
	char buf2[MAX_LENGTH_OF_LONG + 1];

	if (f->key) {
		s1 = f->key->val;
	} else {
		s1 = zend_print_long_to_buf(buf1 + sizeof(buf1) - 1, f->h);
	}
	if (s->key) {
		s2 = s->key->val;
	} else {
		s2 = zend_print_long_to_buf(buf2 + sizeof(buf2) - 1, s->h);
	}
	return strcoll(s1, s2);
}
/* }}} */

static zend_always_inline int php_array_data_compare_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	int result = zend_compare(&f->val, &s->val);
	/* Special enums handling for array_unique. We don't want to add this logic to zend_compare as
	 * that would be observable via comparison operators. */
	zval *rhs = &s->val;
	ZVAL_DEREF(rhs);
	if (UNEXPECTED(Z_TYPE_P(rhs) == IS_OBJECT)
	 && result == ZEND_UNCOMPARABLE
	 && (Z_OBJCE_P(rhs)->ce_flags & ZEND_ACC_ENUM)) {
		zval *lhs = &f->val;
		ZVAL_DEREF(lhs);
		if (Z_TYPE_P(lhs) == IS_OBJECT && (Z_OBJCE_P(lhs)->ce_flags & ZEND_ACC_ENUM)) {
			// Order doesn't matter, we just need to group the same enum values
			uintptr_t lhs_uintptr = (uintptr_t)Z_OBJ_P(lhs);
			uintptr_t rhs_uintptr = (uintptr_t)Z_OBJ_P(rhs);
			return lhs_uintptr == rhs_uintptr ? 0 : (lhs_uintptr < rhs_uintptr ? -1 : 1);
		} else {
			// Shift enums to the end of the array
			return -1;
		}
	}
	return result;
}
/* }}} */

static zend_always_inline int php_array_data_compare_numeric_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	return numeric_compare_function(&f->val, &s->val);
}
/* }}} */

static zend_always_inline int php_array_data_compare_string_case_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	return string_case_compare_function(&f->val, &s->val);
}
/* }}} */

static zend_always_inline int php_array_data_compare_string_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	return string_compare_function(&f->val, &s->val);
}
/* }}} */

static int php_array_natural_general_compare(Bucket *f, Bucket *s, int fold_case) /* {{{ */
{
	zend_string *tmp_str1, *tmp_str2;
	zend_string *str1 = zval_get_tmp_string(&f->val, &tmp_str1);
	zend_string *str2 = zval_get_tmp_string(&s->val, &tmp_str2);

	int result = strnatcmp_ex(ZSTR_VAL(str1), ZSTR_LEN(str1), ZSTR_VAL(str2), ZSTR_LEN(str2), fold_case);

	zend_tmp_string_release(tmp_str1);
	zend_tmp_string_release(tmp_str2);
	return result;
}
/* }}} */

static zend_always_inline int php_array_natural_compare_unstable_i(Bucket *a, Bucket *b) /* {{{ */
{
	return php_array_natural_general_compare(a, b, 0);
}
/* }}} */

static zend_always_inline int php_array_natural_case_compare_unstable_i(Bucket *a, Bucket *b) /* {{{ */
{
	return php_array_natural_general_compare(a, b, 1);
}
/* }}} */

static int php_array_data_compare_string_locale_unstable_i(Bucket *f, Bucket *s) /* {{{ */
{
	return string_locale_compare_function(&f->val, &s->val);
}
/* }}} */

DEFINE_SORT_VARIANTS(key_compare);
DEFINE_SORT_VARIANTS(key_compare_numeric);
DEFINE_SORT_VARIANTS(key_compare_string_case);
DEFINE_SORT_VARIANTS(key_compare_string);
DEFINE_SORT_VARIANTS(key_compare_string_locale);
DEFINE_SORT_VARIANTS(data_compare);
DEFINE_SORT_VARIANTS(data_compare_numeric);
DEFINE_SORT_VARIANTS(data_compare_string_case);
DEFINE_SORT_VARIANTS(data_compare_string);
DEFINE_SORT_VARIANTS(data_compare_string_locale);
DEFINE_SORT_VARIANTS(natural_compare);
DEFINE_SORT_VARIANTS(natural_case_compare);

static bucket_compare_func_t php_get_key_compare_func(zend_long sort_type, int reverse) /* {{{ */
{
	switch (sort_type & ~PHP_SORT_FLAG_CASE) {
		case PHP_SORT_NUMERIC:
			if (reverse) {
				return php_array_reverse_key_compare_numeric;
			} else {
				return php_array_key_compare_numeric;
			}
			break;

		case PHP_SORT_STRING:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_key_compare_string_case;
				} else {
					return php_array_key_compare_string_case;
				}
			} else {
				if (reverse) {
					return php_array_reverse_key_compare_string;
				} else {
					return php_array_key_compare_string;
				}
			}
			break;

		case PHP_SORT_NATURAL:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_key_compare_string_natural_case;
				} else {
					return php_array_key_compare_string_natural_case;
				}
			} else {
				if (reverse) {
					return php_array_reverse_key_compare_string_natural;
				} else {
					return php_array_key_compare_string_natural;
				}
			}
			break;

		case PHP_SORT_LOCALE_STRING:
			if (reverse) {
				return php_array_reverse_key_compare_string_locale;
			} else {
				return php_array_key_compare_string_locale;
			}
			break;

		case PHP_SORT_REGULAR:
		default:
			if (reverse) {
				return php_array_reverse_key_compare;
			} else {
				return php_array_key_compare;
			}
			break;
	}
	return NULL;
}
/* }}} */

static bucket_compare_func_t php_get_data_compare_func(zend_long sort_type, int reverse) /* {{{ */
{
	switch (sort_type & ~PHP_SORT_FLAG_CASE) {
		case PHP_SORT_NUMERIC:
			if (reverse) {
				return php_array_reverse_data_compare_numeric;
			} else {
				return php_array_data_compare_numeric;
			}
			break;

		case PHP_SORT_STRING:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_data_compare_string_case;
				} else {
					return php_array_data_compare_string_case;
				}
			} else {
				if (reverse) {
					return php_array_reverse_data_compare_string;
				} else {
					return php_array_data_compare_string;
				}
			}
			break;

		case PHP_SORT_NATURAL:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_natural_case_compare;
				} else {
					return php_array_natural_case_compare;
				}
			} else {
				if (reverse) {
					return php_array_reverse_natural_compare;
				} else {
					return php_array_natural_compare;
				}
			}
			break;

		case PHP_SORT_LOCALE_STRING:
			if (reverse) {
				return php_array_reverse_data_compare_string_locale;
			} else {
				return php_array_data_compare_string_locale;
			}
			break;

		case PHP_SORT_REGULAR:
		default:
			if (reverse) {
				return php_array_reverse_data_compare;
			} else {
				return php_array_data_compare;
			}
			break;
	}
	return NULL;
}
/* }}} */

static bucket_compare_func_t php_get_data_compare_func_unstable(zend_long sort_type, int reverse) /* {{{ */
{
	switch (sort_type & ~PHP_SORT_FLAG_CASE) {
		case PHP_SORT_NUMERIC:
			if (reverse) {
				return php_array_reverse_data_compare_numeric_unstable;
			} else {
				return php_array_data_compare_numeric_unstable;
			}
			break;

		case PHP_SORT_STRING:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_data_compare_string_case_unstable;
				} else {
					return php_array_data_compare_string_case_unstable;
				}
			} else {
				if (reverse) {
					return php_array_reverse_data_compare_string_unstable;
				} else {
					return php_array_data_compare_string_unstable;
				}
			}
			break;

		case PHP_SORT_NATURAL:
			if (sort_type & PHP_SORT_FLAG_CASE) {
				if (reverse) {
					return php_array_reverse_natural_case_compare_unstable;
				} else {
					return php_array_natural_case_compare_unstable;
				}
			} else {
				if (reverse) {
					return php_array_reverse_natural_compare_unstable;
				} else {
					return php_array_natural_compare_unstable;
				}
			}
			break;

		case PHP_SORT_LOCALE_STRING:
			if (reverse) {
				return php_array_reverse_data_compare_string_locale_unstable;
			} else {
				return php_array_data_compare_string_locale_unstable;
			}
			break;

		case PHP_SORT_REGULAR:
		default:
			if (reverse) {
				return php_array_reverse_data_compare_unstable;
			} else {
				return php_array_data_compare_unstable;
			}
			break;
	}
	return NULL;
}
/* }}} */

/* {{{ Sort an array by key value in reverse order */
bool boltx_krsort(zval *array, zend_long sort_type){

	bucket_compare_func_t cmp;

	cmp = php_get_key_compare_func(sort_type, 1);

	zend_hash_sort(Z_ARRVAL_P(array), cmp, 0);

	return true;
}
/* }}} */

/* {{{ Sort an array by key */
bool boltx_ksort(zval *array, zend_long sort_type){

	bucket_compare_func_t cmp;

	cmp = php_get_key_compare_func(sort_type, 0);

	zend_hash_sort(Z_ARRVAL_P(array), cmp, 0);

	return true;
}
/* }}} */

PHPAPI zend_long php_count_recursive(HashTable *ht) /* {{{ */
{
	zend_long cnt = 0;
	zval *element;

	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		if (GC_IS_RECURSIVE(ht)) {
			php_error_docref(NULL, E_WARNING, "Recursion detected");
			return 0;
		}
		GC_PROTECT_RECURSION(ht);
	}

	cnt = zend_hash_num_elements(ht);
	ZEND_HASH_FOREACH_VAL(ht, element) {
		ZVAL_DEREF(element);
		if (Z_TYPE_P(element) == IS_ARRAY) {
			cnt += php_count_recursive(Z_ARRVAL_P(element));
		}
	} ZEND_HASH_FOREACH_END();

	GC_TRY_UNPROTECT_RECURSION(ht);
	return cnt;
}
/* }}} */

/* {{{ Count the number of elements in a variable (usually an array) */
zend_long boltx_count(zval *array, zend_long mode){
	
    zend_long cnt;

	if (mode != PHP_COUNT_NORMAL && mode != PHP_COUNT_RECURSIVE) {
		zend_argument_value_error(2, "must be either COUNT_NORMAL or COUNT_RECURSIVE");
		ZEND_ASSERT(EG(exception));
	}

	switch (Z_TYPE_P(array)) {
		case IS_ARRAY:
			if (mode != PHP_COUNT_RECURSIVE) {
				cnt = zend_hash_num_elements(Z_ARRVAL_P(array));
			} else {
				cnt = php_count_recursive(Z_ARRVAL_P(array));
			}
			return cnt;
			break;
		case IS_OBJECT: {
			zval retval;
			/* first, we check if the handler is defined */
			zend_object *zobj = Z_OBJ_P(array);
			if (zobj->handlers->count_elements) {
				zval *return_value;
                ZVAL_LONG(return_value, 1);
				if (SUCCESS == zobj->handlers->count_elements(zobj, &Z_LVAL_P(return_value))) {
					return Z_LVAL_P(return_value);
				}
				if (EG(exception)) {
					ZEND_ASSERT(EG(exception));
				}
			}
			/* if not and the object implements Countable we call its count() method */
			if (instanceof_function(zobj->ce, zend_ce_countable)) {
				zend_function *count_fn = zend_hash_find_ptr(&zobj->ce->function_table, ZSTR_KNOWN(ZEND_STR_COUNT));
				zend_call_known_instance_method_with_0_params(count_fn, zobj, &retval);
				if (Z_TYPE(retval) != IS_UNDEF) {
					return(zval_get_long(&retval));
					zval_ptr_dtor(&retval);
				}
				return 0;
			}
		}
		ZEND_FALLTHROUGH;
		default:
			zend_argument_type_error(1, "must be of type Countable|array, %s given", zend_zval_value_name(array));
			ZEND_ASSERT(EG(exception));
	}
    return 0;
}
/* }}} */



/* {{{ Return just the keys from the input array, optionally only for the specified search_value */
zval *boltx_array_keys(zval *input, zval *search_value, bool strict){

	zval *entry,
           *return_value,				/* An entry in the input array */
	       new_val;				/* New value */
	zend_ulong num_idx;
	zend_string *str_idx;
	zend_array *arrval;
	zend_ulong elem_count;

	arrval = Z_ARRVAL_P(input);
	elem_count = zend_hash_num_elements(arrval);

	/* Base case: empty input */
	if (!elem_count) {
		return input;
	}

	/* Initialize return array */
	if (search_value != NULL) {
		array_init(return_value);

		if (strict) {
			ZEND_HASH_FOREACH_KEY_VAL(arrval, num_idx, str_idx, entry) {
				ZVAL_DEREF(entry);
				if (fast_is_identical_function(search_value, entry)) {
					if (str_idx) {
						ZVAL_STR_COPY(&new_val, str_idx);
					} else {
						ZVAL_LONG(&new_val, num_idx);
					}
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &new_val);
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			ZEND_HASH_FOREACH_KEY_VAL(arrval, num_idx, str_idx, entry) {
				if (fast_equal_check_function(search_value, entry)) {
					if (str_idx) {
						ZVAL_STR_COPY(&new_val, str_idx);
					} else {
						ZVAL_LONG(&new_val, num_idx);
					}
					zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &new_val);
				}
			} ZEND_HASH_FOREACH_END();
		}
	} else {
		array_init_size(return_value, elem_count);
		zend_hash_real_init_packed(Z_ARRVAL_P(return_value));
		ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(return_value)) {
			if (HT_IS_PACKED(arrval) && HT_IS_WITHOUT_HOLES(arrval)) {
				/* Optimistic case: range(0..n-1) for vector-like packed array */
				zend_ulong lval = 0;

				for (; lval < elem_count; ++lval) {
					ZEND_HASH_FILL_SET_LONG(lval);
					ZEND_HASH_FILL_NEXT();
				}
			} else {
				/* Go through input array and add keys to the return array */
				ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(input), num_idx, str_idx, entry) {
					if (str_idx) {
						ZEND_HASH_FILL_SET_STR_COPY(str_idx);
					} else {
						ZEND_HASH_FILL_SET_LONG(num_idx);
					}
					ZEND_HASH_FILL_NEXT();
				} ZEND_HASH_FOREACH_END();
			}
		} ZEND_HASH_FILL_END();
	}

    return return_value;
}
/* }}} */
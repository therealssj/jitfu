/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2014 Joe Watkins <krakjoe@php.net>                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe@php.net                                              |
  +----------------------------------------------------------------------+
*/
#ifndef HAVE_BITS_LABEL_H
#define HAVE_BITS_LABEL_H

typedef struct _php_jit_label_t {
	zval                zfunc;
	jit_label_t         label;
	zend_object         std;
} php_jit_label_t;

zend_class_entry *jit_label_ce;

#define PHP_JIT_FETCH_LABEL_O(o) ((php_jit_label_t*) ((char*) o - XtOffsetOf(php_jit_label_t, std)))
#define PHP_JIT_FETCH_LABEL(from) PHP_JIT_FETCH_LABEL_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_LABEL_I(from) (PHP_JIT_FETCH_LABEL(from))->type

void php_jit_minit_label(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_label_methods[];
extern zend_object_handlers php_jit_label_handlers;

#else
#ifndef HAVE_BITS_LABEL
#define HAVE_BITS_LABEL
zend_object_handlers php_jit_label_handlers;

static inline void php_jit_label_free(zend_object *zobject TSRMLS_DC) {
	php_jit_label_t *plabel = 
		(php_jit_label_t *) zobject;

	zval_ptr_dtor(&plabel->zfunc);	
	
	zend_object_std_dtor(&plabel->std TSRMLS_CC);
}

static inline zend_object* php_jit_label_create(zend_class_entry *ce TSRMLS_DC) {
	php_jit_label_t *plabel = 
		(php_jit_label_t*) ecalloc(1, sizeof(php_jit_label_t) + zend_object_properties_size(ce));
	
	zend_object_std_init(&plabel->std, ce TSRMLS_CC);
	object_properties_init(&plabel->std, ce);

	plabel->label = jit_label_undefined;
	
	ZVAL_NULL(&plabel->zfunc);
	
	plabel->std.handlers = &php_jit_label_handlers;
	
	return &plabel->std;
}

void php_jit_minit_label(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Label", php_jit_label_methods);
	jit_label_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_label_ce->create_object = php_jit_label_create;
	
	memcpy(
		&php_jit_label_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_label_handlers));

	php_jit_label_handlers.offset = XtOffsetOf(php_jit_label_t, std);
	php_jit_label_handlers.free_obj = php_jit_label_free;
}

PHP_METHOD(Label, __construct) {
	zval *zfunction = NULL;
	php_jit_function_t *pfunc = NULL;
	php_jit_label_t *plabel = NULL;
	
	if (php_jit_parameters("O", &zfunction, jit_function_ce) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (Func function)");
		return;
	}
	
	plabel = PHP_JIT_FETCH_LABEL(getThis());
	plabel->zfunc = *zfunction;
	zval_copy_ctor(&plabel->zfunc);
	
	pfunc = 
	    PHP_JIT_FETCH_FUNCTION(&plabel->zfunc);
	
	jit_insn_label(pfunc->func, &plabel->label);
}

PHP_METHOD(Label, equal) {
	php_jit_label_t *plabels[2];
	php_jit_function_t *pfuncs[2];
	
	zval *zlabel;
	
	if (php_jit_parameters("O", &zlabel, jit_label_ce) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (Label label)");
		return;
	}
	
	plabels[0] = PHP_JIT_FETCH_LABEL(getThis());
	plabels[1] = PHP_JIT_FETCH_LABEL(zlabel);
	
	pfuncs[0]  = PHP_JIT_FETCH_FUNCTION(&plabels[0]->zfunc);
	pfuncs[1]  = PHP_JIT_FETCH_FUNCTION(&plabels[1]->zfunc);
	
	if (pfuncs[0]->func != pfuncs[1]->func) {
		RETURN_FALSE;
	}
	
	RETURN_BOOL(jit_function_labels_equal(pfuncs[0]->func, plabels[0]->label, plabels[1]->label));
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_label_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_label_equal_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, other)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_label_methods[] = {
	PHP_ME(Label, __construct,     php_jit_label_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Label, equal,           php_jit_label_equal_arginfo,     ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif

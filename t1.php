<?php 

gc_disable();

require 'wp_hook.php';
require 'wp_hook1.php';


add_action('init', 'check_title', 9); 
// add_filter1('init', 'check_title', 9);
// add_action('init', 'check_title', 11);
// add_filter1('init', 'check_title', 11);

// add_action('init', 'check_title', 91, 5);
// add_filter1('init', 'check_title', 91, 5);
// add_action('init', 'check_title', 111);
// add_filter1('init', 'check_title', 111);

function check_title(){
	echo "oooooooo";
}

// add_filter1('init', 'dfdf', 7);
// add_filter1('wp_footer', 'footer_check');

// do_action('init');
do_action('init');

die;
 

function add_action( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	return _add_filter( $hook_name, $callback, $priority, $accepted_args );
}

function _wp_call_all_hook( $args ) {
	global $wp_filter;

	$wp_filter['all']->do_all_hook( $args );
}

function do_action( $hook_name, ...$arg ) {
	global $wp_filter, $wp_actions, $wp_current_filter;

	if ( ! isset( $wp_actions[ $hook_name ] ) ) {
		$wp_actions[ $hook_name ] = 1;
	} else {
		++$wp_actions[ $hook_name ];
	}

	// Do 'all' actions first.
	if ( isset( $wp_filter['all'] ) ) {
		$wp_current_filter[] = $hook_name;
		$all_args            = func_get_args(); // phpcs:ignore PHPCompatibility.FunctionUse.ArgumentFunctionsReportCurrentValue.NeedsInspection
		_wp_call_all_hook( $all_args );
	}

	if ( ! isset( $wp_filter[ $hook_name ] ) ) {
		if ( isset( $wp_filter['all'] ) ) {
			array_pop( $wp_current_filter );
		}

		return;
	}

	if ( ! isset( $wp_filter['all'] ) ) {
		$wp_current_filter[] = $hook_name;
	}

	if ( empty( $arg ) ) {
		$arg[] = '';
	} elseif ( is_array( $arg[0] ) && 1 === count( $arg[0] ) && isset( $arg[0][0] ) && is_object( $arg[0][0] ) ) {
		// Backward compatibility for PHP4-style passing of `array( &$this )` as action `$arg`.
		$arg[0] = $arg[0][0];
	}

	$wp_filter[ $hook_name ]->do_action( $arg );

	array_pop( $wp_current_filter );
}
function _wp_call_all_hook1( $args ) {
	global $wp_filter1;

	$wp_filter1['all']->do_all_hook( $args );
}

function do_action1( $hook_name, ...$arg ) {
	global $wp_filter1, $wp_actions1, $wp_current_filter1;

	if ( ! isset( $wp_actions1[ $hook_name ] ) ) {
		$wp_actions1[ $hook_name ] = 1;
	} else {
		++$wp_actions1[ $hook_name ];
	}

	// Do 'all' actions first.
	if ( isset( $wp_filter1['all'] ) ) {
		$wp_current_filter1[] = $hook_name;
		$all_args            = func_get_args(); // phpcs:ignore PHPCompatibility.FunctionUse.ArgumentFunctionsReportCurrentValue.NeedsInspection
		_wp_call_all_hook( $all_args );
	}

	if ( ! isset( $wp_filter1[ $hook_name ] ) ) {
		if ( isset( $wp_filter1['all'] ) ) {
			array_pop( $wp_current_filter1 );
		}

		return;
	}

	if ( ! isset( $wp_filter1['all'] ) ) {
		$wp_current_filter1[] = $hook_name;
	}

	if ( empty( $arg ) ) {
		$arg[] = '';
	} elseif ( is_array( $arg[0] ) && 1 === count( $arg[0] ) && isset( $arg[0][0] ) && is_object( $arg[0][0] ) ) {
		// Backward compatibility for PHP4-style passing of `array( &$this )` as action `$arg`.
		$arg[0] = $arg[0][0];
	}

	$wp_filter1[ $hook_name ]->do_action( $arg );

	array_pop( $wp_current_filter1 );
}


// _add_filter('init', 'check_title');
// _add_filter('wp_footer', 'footer_check');

// add_filter1('init', 'check_title');
// add_filter1('wp_footer', 'footer_check');



// add_filter1('init', 'check_title');
// _add_filter('wp_footer', 'footer_check');

 



function _add_filter( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	global $wp_filter;

	if ( ! isset( $wp_filter[ $hook_name ] ) ) {
		$wp_filter[ $hook_name ] = new WP_Hook();
	}

	$wp_filter[ $hook_name ]->_add_filter( $hook_name, $callback, $priority, $accepted_args );
	return true;
}

function add_filter1( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	global $wp_filter1;

	if ( ! isset( $wp_filter1[ $hook_name ] ) ) {
		$wp_filter1[ $hook_name ] = new WP_Hook1();
	}

	$wp_filter1[ $hook_name ]->add_filter( $hook_name, $callback, $priority, $accepted_args );

	return true;
}



function _wp_filter_build_unique_id(  $callback  ) {
	if ( is_string( $callback ) ) {
		return $callback;
	}

	if ( is_object( $callback ) ) {
		// Closures are currently implemented as objects.
		$callback = array( $callback, '' );
	} else {
		$callback = (array) $callback;
	}

	if ( is_object( $callback[0] ) ) { 
		// Object class calling.
		return spl_object_hash( $callback[0] ) . $callback[1];
	} elseif ( is_string( $callback[0] ) ) {
		// Static calling.
		return $callback[0] . '::' . $callback[1];
	}
}



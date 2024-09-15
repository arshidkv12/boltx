<?php 

gc_disable();

require 'wp_hook.php';
require 'wp_hook1.php';


_add_filter('init', 'check_title');
_add_filter('wp_footer', 'footer_check');

add_filter1('init', 'check_title');
add_filter1('wp_footer', 'footer_check');


// _add_filter('init', 'check_title');
// _add_filter('wp_footer', 'footer_check');

// add_filter1('init', 'check_title');
// add_filter1('wp_footer', 'footer_check');



// add_filter1('init', 'check_title');
// _add_filter('wp_footer', 'footer_check');

 



function _add_filter( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	global $_wp_filter;

	if ( ! isset( $_wp_filter[ $hook_name ] ) ) {
		$_wp_filter[ $hook_name ] = new WP_Hook();
	}

	// $_wp_filter[ $hook_name ]->add_filter( $hook_name, $callback, $priority, $accepted_args );

	return true;
}

function add_filter1( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	global $wp_filter1;

	if ( ! isset( $wp_filter1[ $hook_name ] ) ) {
		$wp_filter1[ $hook_name ] = new WP_Hook1();
	}

	// $wp_filter1[ $hook_name ]->add_filter( $hook_name, $callback, $priority, $accepted_args );

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

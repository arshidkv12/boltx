<?php 

require 'wp_hook.php';
require 'wp_hook1.php';


_add_filter('init', 'check_title');
_add_filter('wp_footer', 'footer_check');

add_filter1('init', 'check_title');
add_filter1('wp_footer', 'footer_check');


_add_filter('init', 'check_title');
_add_filter('wp_footer', 'footer_check');

add_filter1('init', 'check_title');
add_filter1('wp_footer', 'footer_check');




function _add_filter( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	// global $_wp_filter;

	if ( ! isset( $_wp_filter[ $hook_name ] ) ) {
		$_wp_filter[ $hook_name ] = new WP_Hook1();
	}

	$_wp_filter[ $hook_name ]->add_filter( $hook_name, $callback, $priority, $accepted_args );

	return true;
}

function add_filter1( $hook_name, $callback, $priority = 10, $accepted_args = 1 ) {
	// global $wp_filter1;

	if ( ! isset( $wp_filter1[ $hook_name ] ) ) {
		$wp_filter1[ $hook_name ] = new WP_Hook1();
	}

	$wp_filter1[ $hook_name ]->add_filter( $hook_name, $callback, $priority, $accepted_args );

	return true;
}
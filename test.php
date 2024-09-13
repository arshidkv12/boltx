<?php 

$a = new stdClass;
$a->foo = "abc";


// for ($i=0; $i < 100000; $i++) { 
//       _boltx_unique_id( $a );
//       _wp_filter_build_unique_id( 1, $a, 4);
// }

class MyPlugin {
    public function __construct() {
        echo strlen( _wp_filter_build_unique_id([$this, 'display_custom_message']) ) >= 32 ? 'Valid in class': 'Not valid';
    }
}
new  MyPlugin;


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

<?php 

$a = new stdClass;
$a->foo = "abc";

// _boltx_unique_id( $a );
// _wp_filter_build_unique_id( $a );
 
// $id = _boltx_unique_id( 'String Test' );
// $id = _wp_filter_build_unique_id( 'String Test' );
 

class MyPlugin {
    public function __construct() {
		// $a = memory_get_usage();
		// _wp_filter_build_unique_id([$this, 'display_custom_message']);
		// $b =  memory_get_usage();
		// _boltx_unique_id([$this, 'display_custom_message']);
		// $c = memory_get_usage();
		// echo $b - $a;
		// echo PHP_EOL;
		// echo $c - $b;
        // echo  _boltx_unique_id([$this, 'display_custom_message']);
        // echo  _boltx_unique_id([$this, 'display_custom_message']);
        // echo  _boltx_unique_id([$this, 'display_custom_message']);
        // echo  _wp_filter_build_unique_id([$this, 'display_custom_message']);
        // echo  _wp_filter_build_unique_id([$this, 'display_custom_message']);
        // echo  _wp_filter_build_unique_id([$this, 'display_custom_message']);
		//  echo  _boltx_unique_id([$this, 'display_custom_message']);

		// for ($i=0; $i < 100000; $i++) { 
			_boltx_unique_id([$this, 'display_custom_message']); 
			_wp_filter_build_unique_id([$this, 'display_custom_message']);
$b = 1;
			echo _boltx_unique_id(function()use($b){});
			echo _wp_filter_build_unique_id(function()use($b){});
		// }
		
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

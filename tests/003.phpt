--TEST--
_boltx_unique_id() Basic test
--FILE--
<?php
$a = new stdClass;
$a->foo = "abc";

$id = _boltx_unique_id( $a );
echo strlen( $id ) == 32 ? 'Valid': 'Not valid';
echo PHP_EOL;

$id = _boltx_unique_id( 'String Test' );
echo $id.PHP_EOL;


class MyPlugin {
    public function __construct() {
        echo strlen( _boltx_unique_id([$this, 'display_custom_message']) ) >= 32 ? 'Valid in class': 'Not valid';
    }
}
new  MyPlugin;

?>
--EXPECT--
Valid
String Test
Valid in class


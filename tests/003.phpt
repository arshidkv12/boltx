--TEST--
_boltx_unique_id() Basic test
--EXTENSIONS--
boltx
--FILE--
<?php
var_dump(_boltx_unique_id());
var_dump(_boltx_unique_id('PHP'));
?>
--EXPECT--
string(11) "Hello World"
string(9) "Hello PHP"

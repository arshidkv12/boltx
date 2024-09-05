--TEST--
test1() Basic test
--EXTENSIONS--
boltx
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension boltx is loaded and working!
NULL

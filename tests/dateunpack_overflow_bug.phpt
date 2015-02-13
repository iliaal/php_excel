--TEST--
Excel date pack/unpack overflow tests
--INI--
date.timezone=America/Toronto
--SKIPIF--
<?php if (!extension_loaded("excel")) die("skip - Excel extension not found"); ?>
--FILE--
<?php 
	$x = new ExcelBook();

    $t = mktime(0, 0, 0, 2100, 1, 1);
    $packed = $x->packDate($t);
    $unpacked = $x->unpackDate($packed);

    if ($unpacked != $t)
    {
      echo "source: {$t} <> res: " . $unpacked . " >> diff: ".($unpacked - $t)." packed: '".$packed."'\n";
    }
    else
    {
      echo "OK\n";
    }
?>
--EXPECT--
OK

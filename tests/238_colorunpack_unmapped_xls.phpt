--TEST--
ExcelBook::colorUnpack() rejects XLS colors outside the palette instead of returning undefined values
--EXTENSIONS--
excel
--FILE--
<?php
$xls = new ExcelBook();
$xls->addSheet('s');

$unmapped = [
    ExcelFormat::COLOR_DEFAULT_FOREGROUND,
    ExcelFormat::COLOR_DEFAULT_BACKGROUND,
    0x7FFF,
    0, 1, 2, 3, 4, 5, 6, 7,
];
foreach ($unmapped as $c) {
    $r = @$xls->colorUnpack($c);
    printf("0x%x: %s\n", $c, var_export($r === false, true));
}

$r = $xls->colorUnpack(ExcelFormat::COLOR_DEFAULT_FOREGROUND);
var_dump($r === false);

var_dump($xls->colorUnpack(ExcelFormat::COLOR_BLACK));
var_dump($xls->colorUnpack(ExcelFormat::COLOR_RED));
var_dump($xls->colorUnpack(63));

$xlsx = new ExcelBook(null, null, true);
$xlsx->addSheet('s');
var_dump($xlsx->colorUnpack($xlsx->colorPack(255, 128, 1)));
var_dump($xlsx->colorUnpack($xlsx->colorPack(0, 0, 0)));
?>
--EXPECTF--
0x40: true
0x41: true
0x7fff: true
0x0: true
0x1: true
0x2: true
0x3: true
0x4: true
0x5: true
0x6: true
0x7: true

Warning: ExcelBook::colorUnpack(): Color code '64' has no RGB value in this book in %s on line %d
bool(true)
array(3) {
  ["red"]=>
  int(0)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
}
array(3) {
  ["red"]=>
  int(255)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
}
array(3) {
  ["red"]=>
  int(51)
  ["green"]=>
  int(51)
  ["blue"]=>
  int(51)
}
array(3) {
  ["red"]=>
  int(255)
  ["green"]=>
  int(128)
  ["blue"]=>
  int(1)
}
array(3) {
  ["red"]=>
  int(0)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
}

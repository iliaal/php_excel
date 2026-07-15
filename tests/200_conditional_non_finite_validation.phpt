--TEST--
Conditional-formatting numeric rules reject every non-finite value slot
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists('ExcelBook', 'conditionalFormatSize')) print 'skip libxl 5.0.0+ required';
if ((new ReflectionMethod('ExcelSheet', 'addConditionalFormatting'))->getNumberOfParameters() < 4) print 'skip libxl 5.1.0+ required';
?>
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Sheet1');
$formatting = $sheet->addConditionalFormatting(1, 3, 0, 0);
$format = $book->addConditionalFormat();

$cases = [
    'op value1' => fn() => $formatting->addOpNumRule(0, $format, INF, 1.0),
    'op value2' => fn() => $formatting->addOpNumRule(0, $format, 1.0, NAN),
    '2-color minimum' => fn() => $formatting->add2ColorScaleRule(1, 2, 0, -INF, 0, 1.0),
    '2-color maximum' => fn() => $formatting->add2ColorScaleRule(1, 2, 0, 0.0, 0, INF),
    '3-color minimum' => fn() => $formatting->add3ColorScaleRule(1, 2, 3, 0, NAN, 0, 0.5, 0, 1.0),
    '3-color midpoint' => fn() => $formatting->add3ColorScaleRule(1, 2, 3, 0, 0.0, 0, INF, 0, 1.0),
    '3-color maximum' => fn() => $formatting->add3ColorScaleRule(1, 2, 3, 0, 0.0, 0, 0.5, 0, -INF),
];

$accepted = [];
foreach ($cases as $name => $case) {
    if (@$case() !== false) {
        $accepted[] = $name;
    }
}
var_dump($accepted);
?>
--EXPECT--
array(0) {
}

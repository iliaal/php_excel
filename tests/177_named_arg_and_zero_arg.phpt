--TEST--
fillPattern() accepts its documented named argument; zero-arg methods parse args
--EXTENSIONS--
excel
--DESCRIPTION--
CR-007: the fillPattern() parameter shipped as $patern in arginfo, so a call
using the documented name (pattern:) failed. CR-002: zero-arg methods that
omitted ZEND_PARSE_PARAMETERS_NONE() are now well-formed and callable.
--FILE--
<?php
$fmt = new ExcelFormat(new ExcelBook(null, null, true));
try {
    $fmt->fillPattern(pattern: ExcelFormat::FILLPATTERN_SOLID);
    echo "named-arg: ok\n";
} catch (\Throwable $e) {
    echo "named-arg: " . get_class($e) . "\n";
}
$b = new ExcelBook(null, null, true);
$b->addSheet("Z");
var_dump(is_int($b->sheetCount()));
var_dump(is_string($b->getLibXlVersion()));

// A surplus argument to a zero-arg method must not crash the process (it
// throws ArgumentCountError on a debug build, is ignored on a release build);
// either way the token below must print, proving no arginfo/zpp fatal.
foreach ([
    fn() => $b->getPhpExcelVersion(1),
    fn() => $b->coreProperties()->title(1),
    fn() => $b->getLibXlVersion(1, 2),
] as $call) {
    try { $call(); } catch (\ArgumentCountError $e) { /* debug build */ }
    echo "no-fatal\n";
}
?>
--EXPECT--
named-arg: ok
bool(true)
bool(true)
no-fatal
no-fatal
no-fatal

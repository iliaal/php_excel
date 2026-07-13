--TEST--
Native child wrappers reject a second constructor call
--EXTENSIONS--
excel
--FILE--
<?php
function rejects_reconstruction(string $name, object $object, callable $reconstruct): void
{
    try {
        $reconstruct();
        echo "$name: accepted\n";
    } catch (Throwable $e) {
        echo "$name: rejected\n";
    }
}

$book = new ExcelBook(null, null, true);
$sheet = new ExcelSheet($book, 'S');
$format = new ExcelFormat($book);
$font = new ExcelFont($book);
$autofilter = new ExcelAutoFilter($sheet);
$autofilter->setRef(1, 2, 0, 0);
$filtercolumn = new ExcelFilterColumn($autofilter, 0);
$richstring = new ExcelRichString($book);
$conditionalformat = new ExcelConditionalFormat($book);
$conditionalFormattingHasRange = (new ReflectionMethod(ExcelConditionalFormatting::class, '__construct'))->getNumberOfParameters() > 1;
$conditionalformatting = $conditionalFormattingHasRange
    ? new ExcelConditionalFormatting($sheet, 1, 2, 0, 0)
    : new ExcelConditionalFormatting($sheet);
$coreproperties = new ExcelCoreProperties($book);
$table = new ExcelTable($sheet, 'T', 1, 2, 0, 0);
$fixtureBook = new ExcelBook(null, null, true);
$fixtureBook->loadFile(__DIR__ . '/formcontrols.xlsx');
$fixtureSheet = $fixtureBook->getSheet(0);
$formcontrol = new ExcelFormControl($fixtureSheet, 0);

rejects_reconstruction('filtercolumn', $filtercolumn, fn() => $filtercolumn->__construct($autofilter, 0));
rejects_reconstruction('autofilter', $autofilter, fn() => $autofilter->__construct($sheet));
rejects_reconstruction('sheet', $sheet, fn() => $sheet->__construct($book, 'S2'));
rejects_reconstruction('format', $format, fn() => $format->__construct($book));
rejects_reconstruction('font', $font, fn() => $font->__construct($book));
rejects_reconstruction('richstring', $richstring, fn() => $richstring->__construct($book));
rejects_reconstruction('formcontrol', $formcontrol, fn() => $formcontrol->__construct($fixtureSheet, 0));
rejects_reconstruction('conditionalformat', $conditionalformat, fn() => $conditionalformat->__construct($book));
rejects_reconstruction(
    'conditionalformatting',
    $conditionalformatting,
    $conditionalFormattingHasRange
        ? fn() => $conditionalformatting->__construct($sheet, 1, 2, 0, 0)
        : fn() => $conditionalformatting->__construct($sheet),
);
rejects_reconstruction('coreproperties', $coreproperties, fn() => $coreproperties->__construct($book));
rejects_reconstruction('table', $table, fn() => $table->__construct($sheet, 'T2', 1, 2, 0, 0));
echo "OK\n";
?>
--EXPECT--
filtercolumn: rejected
autofilter: rejected
sheet: rejected
format: rejected
font: rejected
richstring: rejected
formcontrol: rejected
conditionalformat: rejected
conditionalformatting: rejected
coreproperties: rejected
table: rejected
OK

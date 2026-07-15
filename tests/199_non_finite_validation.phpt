--TEST--
Public double parameters reject NAN and infinities before entering LibXL
--EXTENSIONS--
excel
--FILE--
<?php
$book = new ExcelBook(null, null, true);
$sheet = $book->addSheet('Sheet1');
$sheet->write(1, 0, 'Name');
$sheet->write(2, 0, 'Alice');
$filter = $sheet->autoFilter();
$filter->setRef(1, 2, 0, 0);
$column = $filter->column(0);
$properties = $book->coreProperties();

$cases = [
    'unpackDate INF' => fn() => $book->unpackDate(INF),
    'unpackDate NAN' => fn() => $book->unpackDate(NAN),
    'setColWidth' => fn() => $sheet->setColWidth(0, 0, INF),
    'setRowHeight' => fn() => $sheet->setRowHeight(1, NAN),
    'addPictureScaled' => fn() => $sheet->addPictureScaled(1, 0, 0, -INF),
    'setHeader' => fn() => $sheet->setHeader('header', INF),
    'setFooter' => fn() => $sheet->setFooter('footer', NAN),
    'setMarginLeft' => fn() => $sheet->setMarginLeft(INF),
    'setMarginRight' => fn() => $sheet->setMarginRight(-INF),
    'setMarginTop' => fn() => $sheet->setMarginTop(NAN),
    'setMarginBottom' => fn() => $sheet->setMarginBottom(INF),
    'validation value1' => fn() => $sheet->addDataValidationDouble(0, 0, 1, 1, 0, 0, NAN),
    'validation value2' => fn() => $sheet->addDataValidationDouble(0, 0, 1, 1, 0, 0, 1.0, INF),
    'setTop10' => fn() => $column->setTop10(NAN),
    'setCreatedAsDouble' => fn() => $properties->setCreatedAsDouble(INF),
    'setModifiedAsDouble' => fn() => $properties->setModifiedAsDouble(NAN),
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

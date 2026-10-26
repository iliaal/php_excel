--TEST--
moveSheet invalidates all sheet wrappers only on success, not workbook fonts and formats
--EXTENSIONS--
excel
--FILE--
<?php
function check($actual, $expected, $label) {
    if ($actual !== $expected) {
        echo "FAIL $label\n";
        var_dump($actual, $expected);
    }
}
foreach ([false, true] as $xlsx) {
    $book = new ExcelBook(null, null, $xlsx);
    $a = $book->addSheet('A');
    $b = $book->addSheet('B');
    $c = $book->addSheet('C');
    $a->write(1, 0, 'alpha');
    $b->write(1, 0, 'beta');
    $c->write(1, 0, 'gamma');
    $font = $book->addFont();
    $font->size(14);
    $format = $book->addFormat();
    $format->numberFormat(ExcelFormat::NUMFORMAT_TEXT);
    if ($xlsx) {
        $filter = $b->autoFilter();
        $filter->setRef(1, 2, 0, 1);
        $column = $filter->column(0);
        $column->addFilter('beta');
    }

    check($book->moveSheet(0, 99), false, 'failed move');
    check([$a->name(), $b->name(), $c->name()], ['A', 'B', 'C'], 'names after failure');
    if ($xlsx) { check($filter->getRef()['row_first'], 1, 'filter after failure'); }
    if ($xlsx) { check($column->filter(0), 'beta', 'column after failure'); }

    check($book->moveSheet(1, 0), true, 'successful move');
    foreach ([$a, $b, $c] as $old) {
        check(@$old->name(), false, 'stale sheet, including unchanged C');
    }
    if ($xlsx) { check(@$filter->getRef(), false, 'stale filter'); }
    if ($xlsx) { check(@$column->filter(0), false, 'stale column'); }
    check($font->size(), 14, 'font remains usable');
    check($format->numberFormat(), ExcelFormat::NUMFORMAT_TEXT, 'format remains usable');
    check([$book->getSheet(0)->name(), $book->getSheet(1)->name(), $book->getSheet(2)->name()],
          ['B', 'A', 'C'], 'new order');
    check($book->getSheetByName('B')->read(1, 0), 'beta', 'refetched data');
    if ($xlsx) { check($book->getSheetByName('B')->autoFilter()->column(0)->filter(0), 'beta', 'refetched filter'); }
    echo ($xlsx ? 'XLSX' : 'XLS'), " OK\n";
}
?>
--EXPECT--
XLS OK
XLSX OK

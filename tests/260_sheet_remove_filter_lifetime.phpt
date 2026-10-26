--TEST--
Sheet removeFilter invalidates workbook-wide filter wrappers, including no-op removals
--EXTENSIONS--
excel
--FILE--
<?php
function check(string $label, bool $ok): void {
    echo $label, ": ", $ok ? "OK" : "FAIL", "\n";
}
$book = new ExcelBook(null, null, true);
$a = $book->addSheet("A");
$z = $book->addSheet("Z");
foreach ([$a, $z] as $sheet) {
    $sheet->write(1, 0, "Name");
    $sheet->write(2, 0, "Value");
}
$owners = [$a, $z];
$filters = $columns = [];
foreach ($owners as $owner) {
    $filter = $owner->autoFilter();
    $filter->setRef(1, 2, 0, 0);
    $filters[] = $filter;
    $columns[] = $filter->column(0);
}
$otherBook = new ExcelBook(null, null, true);
$otherSheet = $otherBook->addSheet("Other");
$otherFilter = $otherSheet->autoFilter();
$otherFilter->setRef(1, 2, 0, 0);
$otherColumn = $otherFilter->column(0);
$otherRef = $otherFilter->getRef();
$survivingRef = $z->autoFilter()->getRef();
$target = $a;
check("remove", $target->removeFilter());
foreach ($filters as $i => $filter) {
    check("stale filter $i", @$filter->getRef() === false);
    check("stale column $i", @$columns[$i]->index() === false);
}
check("sheets survive", $a->name() === "A" && $z->name() === "Z");
$fresh = $z->autoFilter();
$freshColumn = $fresh->column(0);
check("surviving filter", $fresh->getRef() === $survivingRef && $freshColumn->index() === 0);
check("no-op remove", $target->removeFilter());
check("no-op invalidates filter", @$fresh->getRef() === false);
check("no-op invalidates column", @$freshColumn->index() === false);
check("re-fetch after no-op", $z->autoFilter()->getRef() === $survivingRef);
check("other workbook survives", $otherFilter->getRef() === $otherRef && $otherColumn->index() === 0);
?>
--EXPECT--
remove: OK
stale filter 0: OK
stale column 0: OK
stale filter 1: OK
stale column 1: OK
sheets survive: OK
surviving filter: OK
no-op remove: OK
no-op invalidates filter: OK
no-op invalidates column: OK
re-fetch after no-op: OK
other workbook survives: OK

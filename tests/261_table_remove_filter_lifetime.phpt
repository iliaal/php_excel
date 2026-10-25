--TEST--
Sheet and table removeFilter invalidate both kinds of filter wrappers
--EXTENSIONS--
excel
--SKIPIF--
<?php
if (!method_exists("ExcelTable", "removeFilter")) die("skip libxl 5.2.0+ required");
?>
--FILE--
<?php
function check(string $label, bool $ok): void {
    echo $label, ": ", $ok ? "OK" : "FAIL", "\n";
}
foreach (["sheet", "table"] as $kind) {
    echo $kind, "\n";
    $book = new ExcelBook(null, null, true);
    $a = $book->addSheet("A");
    $z = $book->addSheet("Z");
    foreach ([$a, $z] as $sheet) {
        $sheet->write(1, 0, "Name");
        $sheet->write(2, 0, "Value");
    }
    $t = new ExcelTable($a, "One", 1, 2, 0, 0, true, 0);
    $u = new ExcelTable($z, "Two", 1, 2, 0, 0, true, 0);
    $owners = [$a, $z, $t, $u];
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
    $survivingTableRef = $u->autoFilter()->getRef();
    $target = $kind === "sheet" ? $a : $t;
    check("remove", $target->removeFilter());
    foreach ($filters as $i => $filter) {
        check("stale filter $i", @$filter->getRef() === false);
        check("stale column $i", @$columns[$i]->index() === false);
    }
    check("sheets survive", $a->name() === "A" && $z->name() === "Z");
    check("tables survive", $t->name() === "One" && $u->name() === "Two");
    $fresh = $z->autoFilter();
    $freshColumn = $fresh->column(0);
    check("surviving filter", $fresh->getRef() === $survivingRef && $freshColumn->index() === 0);
    check("surviving table filter", $u->autoFilter()->getRef() === $survivingTableRef);
    check("no-op remove", $target->removeFilter());
    check("no-op invalidates filter", @$fresh->getRef() === false);
    check("no-op invalidates column", @$freshColumn->index() === false);
    check("re-fetch after no-op", $z->autoFilter()->getRef() === $survivingRef);
    check("other workbook survives", $otherFilter->getRef() === $otherRef && $otherColumn->index() === 0);
}
?>
--EXPECT--
sheet
remove: OK
stale filter 0: OK
stale column 0: OK
stale filter 1: OK
stale column 1: OK
stale filter 2: OK
stale column 2: OK
stale filter 3: OK
stale column 3: OK
sheets survive: OK
tables survive: OK
surviving filter: OK
surviving table filter: OK
no-op remove: OK
no-op invalidates filter: OK
no-op invalidates column: OK
re-fetch after no-op: OK
other workbook survives: OK
table
remove: OK
stale filter 0: OK
stale column 0: OK
stale filter 1: OK
stale column 1: OK
stale filter 2: OK
stale column 2: OK
stale filter 3: OK
stale column 3: OK
sheets survive: OK
tables survive: OK
surviving filter: OK
surviving table filter: OK
no-op remove: OK
no-op invalidates filter: OK
no-op invalidates column: OK
re-fetch after no-op: OK
other workbook survives: OK

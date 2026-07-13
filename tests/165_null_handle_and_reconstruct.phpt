--TEST--
Child factory methods return false on a NULL libxl handle
--EXTENSIONS--
excel
--DESCRIPTION--
CR-004: AutoFilter::column / columnByIndex / Sheet::autoFilter wrapped a NULL
libxl handle in a usable-looking object instead of honoring the stub's
`|false` contract. A zombie wrapper only fails on the *next* method call.
--FILE--
<?php
// CR-004: Sheet::autoFilter on XLS (libxl returns a NULL autofilter handle)
// must be false, not a zombie ExcelAutoFilter.
$xls = new ExcelBook();
$s = $xls->addSheet("s");
$s->write(1, 0, "a");
echo "xls autoFilter():            "; var_dump($s->autoFilter());

// CR-004: columnByIndex() past the end of the column list is a NULL handle.
$b = new ExcelBook(null, null, true);
$sx = $b->addSheet("S");
for ($c = 0; $c < 3; $c++) { $sx->write(1, $c, "h$c"); $sx->write(2, $c, "v$c"); }
$af = $sx->autoFilter();
$af->setRef(1, 2, 0, 2);
$af->column(0);                     // create exactly one column
echo "columnByIndex(0) (valid):    "; var_dump(is_object($af->columnByIndex(0)));
echo "columnByIndex(50) (oob):     "; var_dump($af->columnByIndex(50));

// CR-004: column() on a degenerate autofilter ref -> NULL handle -> false.
$b2 = new ExcelBook(null, null, true);
$s2 = $b2->addSheet("S");
$s2->write(1, 0, "h");
$af2 = $s2->autoFilter();
@$af2->setRef(0, 0, 1, 0);          // colFirst(1) > colLast(0): rejected, ref unset
echo "column(0) (no valid ref):    "; var_dump($af2->column(0));

echo "OK\n";
?>
--EXPECT--
xls autoFilter():            bool(false)
columnByIndex(0) (valid):    bool(true)
columnByIndex(50) (oob):     bool(false)
column(0) (no valid ref):    bool(false)
OK

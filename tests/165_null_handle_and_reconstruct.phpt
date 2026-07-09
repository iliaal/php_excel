--TEST--
Child factory methods return false on a NULL libxl handle; re-invoking a child __construct rebinds the parent without leaking
--EXTENSIONS--
excel
--DESCRIPTION--
CR-004: AutoFilter::column / columnByIndex / Sheet::autoFilter wrapped a NULL
libxl handle in a usable-looking object instead of honoring the stub's
`|false` contract. A zombie wrapper only fails on the *next* method call.
CR-002: re-invoking a child __construct() copied the new parent over the old
one without releasing it, leaking a parent refcount for the rest of the
request. Observable via destructor timing: with the leak, the first parent
is not freed when the last userland reference is dropped, only at shutdown.
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

// CR-002: re-invoking a child __construct rebinds onto the new parent and
// releases the old one. A subclassed book with a destructor makes the timing
// observable: the first book must be freed the instant its last userland
// reference is dropped, not held alive by a leaked child reference.
class TrackedBook extends ExcelBook {
    public string $tag = "";
    public function __destruct() { echo "freed {$this->tag}\n"; }
}
$b1 = new TrackedBook(null, null, true); $b1->tag = "b1";
$b2 = new TrackedBook(null, null, true); $b2->tag = "b2";
$font = new ExcelFont($b1);         // font->parent = b1
$font->__construct($b2);            // rebind onto b2; b1's ref must be dropped
echo "unset b1:\n";   unset($b1);   // freed here unless leaked
echo "unset font:\n"; unset($font); // last holder of b2
echo "unset b2:\n";   unset($b2);   // freed here
echo "OK\n";
?>
--EXPECT--
xls autoFilter():            bool(false)
columnByIndex(0) (valid):    bool(true)
columnByIndex(50) (oob):     bool(false)
column(0) (no valid ref):    bool(false)
unset b1:
freed b1
unset font:
unset b2:
freed b2
OK

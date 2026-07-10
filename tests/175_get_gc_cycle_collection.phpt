--TEST--
Cycle through a child wrapper's hidden parent is reclaimable via get_gc
--EXTENSIONS--
excel
--DESCRIPTION--
Child wrappers hold a strong zval to their parent book/sheet that the std
object handlers did not expose. Without a get_gc reporting it, a user-formed
cycle (book -> sheet property, sheet -> hidden parent book) was uncollectable
and leaked. The get_gc handlers now surface the parent so gc_collect_cycles()
reclaims it.
--FILE--
<?php
class BookX extends ExcelBook { public $ref; }
$b = new BookX(null, null, true);
$s = $b->addSheet("S");
$b->ref = $s;                      // book -> sheet; sheet -> hidden parent book
$w = WeakReference::create($b);
unset($b, $s);
var_dump(gc_collect_cycles() > 0); // the cycle was reclaimed
var_dump($w->get() === null);      // book freed, not leaked
?>
--EXPECT--
bool(true)
bool(true)

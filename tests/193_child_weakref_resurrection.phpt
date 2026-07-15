--TEST--
Child destruction invalidates weak references before releasing its hidden parent
--EXTENSIONS--
excel
--FILE--
<?php
class DestructibleBook extends ExcelBook
{
    public static WeakReference $child;
    public function __destruct()
    {
        var_dump(self::$child->get());
    }
}

$book = new DestructibleBook();
$sheet = $book->addSheet('S');
DestructibleBook::$child = WeakReference::create($sheet);
unset($book, $sheet);
echo "alive\n";
?>
--EXPECT--
NULL
alive

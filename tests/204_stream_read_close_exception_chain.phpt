--TEST--
Stream-backed reads preserve the read exception and chain a close exception
--EXTENSIONS--
excel
--FILE--
<?php
class ReadCloseExceptionStream
{
    public $context;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_read(int $count): string { throw new RuntimeException('read failed'); }
    public function stream_eof(): bool { return false; }
    public function stream_close(): void { throw new RuntimeException('close failed'); }
}

stream_wrapper_register('read-close-exception', ReadCloseExceptionStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('Before');
try {
    @$book->loadFile('read-close-exception://book.xlsx');
    echo "missing exception\n";
} catch (Throwable $e) {
    echo $e->getMessage(), "\n";
    echo $e->getPrevious()?->getMessage(), "\n";
}
var_dump($book->getSheetName(0));
?>
--EXPECT--
read failed
close failed
string(6) "Before"

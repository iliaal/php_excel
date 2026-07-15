--TEST--
ExcelBook::save() does not retain a wrapper pointer after stream_write unregisters it
--EXTENSIONS--
excel
--FILE--
<?php
class UnregisterDuringWrite
{
    public $context;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_write(string $data): int
    {
        stream_wrapper_unregister('unregister-write');
        return strlen($data);
    }
    public function stream_flush(): bool { return true; }
    public function stream_close(): void {}
    public function rename(string $from, string $to): bool { return true; }
    public function unlink(string $path): bool { return true; }
}

stream_wrapper_register('unregister-write', UnregisterDuringWrite::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump(@$book->save('unregister-write://book.xlsx'));
echo "alive\n";
?>
--EXPECT--
bool(false)
alive

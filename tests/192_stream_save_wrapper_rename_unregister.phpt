--TEST--
ExcelBook::save() does not retain a wrapper pointer after rename unregisters it
--EXTENSIONS--
excel
--FILE--
<?php
class UnregisterDuringRename
{
    public $context;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_write(string $data): int { return strlen($data); }
    public function stream_flush(): bool { return true; }
    public function stream_close(): void {}
    public function rename(string $from, string $to): bool
    {
        stream_wrapper_unregister('unregister-rename');
        return false;
    }
    public function unlink(string $path): bool { return true; }
}

stream_wrapper_register('unregister-rename', UnregisterDuringRename::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump(@$book->save('unregister-rename://book.xlsx'));
echo "alive\n";
?>
--EXPECT--
bool(false)
alive

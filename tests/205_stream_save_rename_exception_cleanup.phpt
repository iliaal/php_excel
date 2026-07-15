--TEST--
ExcelBook::save() preserves a rename exception while removing the staging URL
--EXTENSIONS--
excel
--FILE--
<?php
class RenameExceptionStream
{
    public $context;
    public static int $closes = 0;
    public static int $renames = 0;
    public static int $unlinks = 0;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_write(string $data): int { return strlen($data); }
    public function stream_flush(): bool { return true; }
    public function stream_close(): void { self::$closes++; }
    public function rename(string $from, string $to): bool
    {
        self::$renames++;
        throw new RuntimeException('rename failed');
    }
    public function unlink(string $path): bool { self::$unlinks++; return true; }
}

stream_wrapper_register('rename-exception', RenameExceptionStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
$warnings = [];
set_error_handler(static function (int $level, string $message) use (&$warnings): bool {
    $warnings[] = $message;
    return true;
});
try {
    $book->save('rename-exception://book.xlsx');
    echo "missing exception\n";
} catch (Throwable $e) {
    echo $e->getMessage(), "\n";
}
restore_error_handler();
printf(
    "warnings=%d closes=%d renames=%d unlinks=%d\n",
    count($warnings),
    RenameExceptionStream::$closes,
    RenameExceptionStream::$renames,
    RenameExceptionStream::$unlinks,
);
?>
--EXPECT--
rename failed
warnings=0 closes=1 renames=1 unlinks=1

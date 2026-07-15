--TEST--
ExcelBook::save() rejects a completed write when stream_flush fails
--EXTENSIONS--
excel
--FILE--
<?php
class FlushFailureStream
{
    public $context;
    public static int $flushes = 0;
    public static int $closes = 0;
    public static int $renames = 0;
    public static int $unlinks = 0;
    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool { return true; }
    public function stream_write(string $data): int { return strlen($data); }
    public function stream_flush(): bool { self::$flushes++; return false; }
    public function stream_close(): void { self::$closes++; }
    public function rename(string $from, string $to): bool { self::$renames++; return true; }
    public function unlink(string $path): bool { self::$unlinks++; return true; }
}

stream_wrapper_register('flush-failure', FlushFailureStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'x');
var_dump(@$book->save('flush-failure://book.xlsx'));
printf(
    "flushes=%d closes=%d renames=%d unlinks=%d\n",
    FlushFailureStream::$flushes,
    FlushFailureStream::$closes,
    FlushFailureStream::$renames,
    FlushFailureStream::$unlinks,
);
?>
--EXPECT--
bool(false)
flushes=1 closes=1 renames=0 unlinks=1

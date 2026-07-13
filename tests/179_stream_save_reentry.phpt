--TEST--
Stream-backed save owns its bytes across book reconstruction callbacks
--EXTENSIONS--
excel
--FILE--
<?php
class ReentrantWriteStream
{
    public $context;
    public static $onOpen;
    public static string $written = '';
    private static bool $called = false;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        if (!self::$called) {
            self::$called = true;
            (self::$onOpen)();
        }
        return true;
    }

    public function stream_write(string $data): int
    {
        self::$written .= $data;
        return strlen($data);
    }

    public function stream_flush(): bool { return true; }
    public function unlink(string $path): bool { return true; }
    public function rename(string $from, string $to): bool { return true; }
}

stream_wrapper_register('reentrant-write', ReentrantWriteStream::class);

$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, str_repeat('x', 10000));
ReentrantWriteStream::$onOpen = static function () use ($book): void {
    $book->__construct(null, null, true);
};

var_dump($book->save('reentrant-write://book'));
var_dump(str_starts_with(ReentrantWriteStream::$written, "PK\x03\x04"));
echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
OK

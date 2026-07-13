--TEST--
Stream-backed load and picture operations survive book reconstruction callbacks
--EXTENSIONS--
excel
--FILE--
<?php
class ReentrantReadStream
{
    public $context;
    public static string $data = '';
    public static $onOpen;
    private int $offset = 0;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        (self::$onOpen)();
        return true;
    }

    public function stream_read(int $length): string
    {
        $result = substr(self::$data, $this->offset, $length);
        $this->offset += strlen($result);
        return $result;
    }

    public function stream_eof(): bool
    {
        return $this->offset >= strlen(self::$data);
    }

    public function stream_stat(): array
    {
        return ['size' => strlen(self::$data)];
    }
}

stream_wrapper_register('reentrant-read', ReentrantReadStream::class);

ReentrantReadStream::$data = file_get_contents(__DIR__ . '/formcontrols.xlsx');

$book = new ExcelBook(null, null, true);
ReentrantReadStream::$onOpen = static function () use ($book): void {
    $book->__construct(null, null, true);
};
var_dump($book->loadFile('reentrant-read://book'));
var_dump($book->getSheet() instanceof ExcelSheet);

$partialOk = true;
if (method_exists($book, 'loadFilePartially')) {
    $partialOk = $book->loadFilePartially('reentrant-read://partial', 0, 0, 10, true)
        && $book->getSheet() instanceof ExcelSheet;
}
var_dump($partialOk);

ReentrantReadStream::$data = file_get_contents(__DIR__ . '/phplogo.jpg');
var_dump(is_int($book->addPictureFromFile('reentrant-read://picture')));

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
OK

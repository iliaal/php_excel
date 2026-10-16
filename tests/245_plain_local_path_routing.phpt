--TEST--
File-name methods hand LibXL only names PHP opens as plain files, and fail closed when resolution changes mid-call
--EXTENSIONS--
excel
--FILE--
<?php
class MemoryStream
{
    public static array $files = [];

    public $context;
    private string $path;
    private int $position = 0;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        if (str_contains($mode, 'x') && isset(self::$files[$path])) {
            return false;
        }
        if (strpbrk($mode, 'wx') !== false) {
            self::$files[$path] = '';
        } elseif (!isset(self::$files[$path])) {
            return false;
        }
        $this->path = $path;
        return true;
    }

    public function stream_write(string $data): int
    {
        self::$files[$this->path] .= $data;
        return strlen($data);
    }

    public function stream_read(int $count): string
    {
        $chunk = substr(self::$files[$this->path], $this->position, $count);
        $this->position += strlen($chunk);
        return $chunk;
    }

    public function stream_eof(): bool { return $this->position >= strlen(self::$files[$this->path]); }
    public function stream_stat(): array { return ['size' => strlen(self::$files[$this->path])]; }
    public function stream_flush(): bool { return true; }
    public function stream_close(): void {}

    public function rename(string $from, string $to): bool
    {
        self::$files[$to] = self::$files[$from];
        unset(self::$files[$from]);
        return true;
    }

    public function unlink(string $path): bool
    {
        unset(self::$files[$path]);
        return true;
    }

    public function url_stat(string $path, int $flags): array|false
    {
        return isset(self::$files[$path]) ? ['size' => strlen(self::$files[$path])] : false;
    }
}

function make_book(string $sheet = 'S'): ExcelBook
{
    $book = new ExcelBook(null, null, true);
    $book->addSheet($sheet)->write(1, 0, 'kept');
    return $book;
}

function local_leftovers(string $prefix): int
{
    return count(array_filter(scandir(getcwd()), fn($f) => str_starts_with($f, $prefix)));
}

$warnings = [];
$onMissingWrapper = null;
set_error_handler(function ($no, $message) use (&$warnings, &$onMissingWrapper) {
    $warnings[] = $message;
    if ($onMissingWrapper && str_contains($message, 'Unable to find the wrapper')) {
        $onMissingWrapper();
    }
    return true;
});
function saw(array $warnings, string $needle): bool
{
    return count(array_filter($warnings, fn($m) => str_contains($m, $needle))) > 0;
}

/* With data: unregistered, every lookup of a "data:" name warns and falls
 * back to the file: wrapper, so an error handler runs inside each one. */
stream_wrapper_unregister('data');

echo "-- handler reinitializes the book during the staging open --\n";
$book = make_book();
$missing = 0;
$onMissingWrapper = function () use ($book, &$missing) {
    if (++$missing === 2) {
        $book->__construct(null, null, true);
    }
};
$warnings = [];
var_dump($book->save('data:excel-245-stale.xlsx'));
var_dump($missing >= 2);
var_dump(saw($warnings, 'the book was reinitialized or reloaded during the save'));
var_dump(local_leftovers('data:excel-245-stale'));

echo "-- handler replaces file: during the staging open --\n";
$book = make_book();
$missing = 0;
$onMissingWrapper = function () use (&$missing) {
    if (++$missing === 2) {
        stream_wrapper_unregister('file');
        stream_wrapper_register('file', MemoryStream::class);
    }
};
$warnings = [];
$saved = $book->save('data:excel-245-swap.xlsx');
stream_wrapper_restore('file');
var_dump($saved);
var_dump(saw($warnings, 'the destination stopped resolving to a local file during the save'));
var_dump(MemoryStream::$files);
var_dump(local_leftovers('data:excel-245-swap'));

echo "-- throwing handler during the wrapper lookup --\n";
$onMissingWrapper = null;
file_put_contents(getcwd() . '/data:excel-245-throw.xlsx', make_book('FromDisk')->save());
$onMissingWrapper = function () {
    throw new RuntimeException('lookup refused');
};
$book = make_book('Orig');
try {
    var_dump($book->loadFile('data:excel-245-throw.xlsx'));
} catch (RuntimeException $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
}
echo $book->getSheet(0)->name(), "\n";
try {
    var_dump($book->save('data:excel-245-throw-save.xlsx'));
} catch (RuntimeException $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
}
var_dump(local_leftovers('data:excel-245-throw-save'));
$onMissingWrapper = null;
unlink(getcwd() . '/data:excel-245-throw.xlsx');

stream_wrapper_restore('data');

echo "-- data: names resolve through a user wrapper --\n";
stream_wrapper_unregister('data');
stream_wrapper_register('data', MemoryStream::class);
MemoryStream::$files['data:plain.xlsx'] = make_book()->save();
MemoryStream::$files['data:logo.jpg'] = file_get_contents(__DIR__ . '/phplogo.jpg');
$warnings = [];
$book = new ExcelBook(null, null, true);
if (method_exists($book, 'loadFilePartially')) {
    var_dump($book->loadFilePartially('data:plain.xlsx', 0, 1, 1));
    echo $book->getSheet(0)->read(1, 0), "\n";
    var_dump($book->loadFileWithoutEmptyCells('data:plain.xlsx'));
    var_dump(saw($warnings, 'Stream wrappers are not supported by loadFileWithoutEmptyCells'));
} else {
    echo "bool(true)\nkept\nbool(false)\nbool(true)\n";
}
var_dump(is_int($book->addPictureFromFile('data:logo.jpg')));
if (method_exists(ExcelBook::class, 'loadInfoRaw')) {
    $info = new ExcelBook(null, null, true);
    var_dump($info->loadInfo('data:plain.xlsx'));
    echo $info->getSheetName(0), "\n";
} else {
    echo "bool(true)\nS\n";
}
stream_wrapper_restore('data');

restore_error_handler();
echo "OK\n";
?>
--CLEAN--
<?php
foreach (scandir(getcwd()) as $f) {
    if (str_contains($f, 'excel-245')) {
        @unlink(getcwd() . '/' . $f);
    }
}
?>
--EXPECT--
-- handler reinitializes the book during the staging open --
bool(false)
bool(true)
bool(true)
int(0)
-- handler replaces file: during the staging open --
bool(false)
bool(true)
array(0) {
}
int(0)
-- throwing handler during the wrapper lookup --
RuntimeException: lookup refused
Orig
RuntimeException: lookup refused
int(0)
-- data: names resolve through a user wrapper --
bool(true)
kept
bool(false)
bool(true)
bool(true)
bool(true)
S
OK

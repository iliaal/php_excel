--TEST--
save() through user wrappers over data: and file: survives the wrapper re-entering the book
--EXTENSIONS--
excel
--FILE--
<?php
/* Names without "://" can still resolve to a user wrapper: "data:" always
 * does, and every plain path does once file: is replaced. Such names must
 * take the stream route, where the workbook image is materialized before any
 * wrapper callback runs, so a callback that reinitializes or reloads the book
 * cannot free the handle the save is using. The saved file is the workbook as
 * it was when save() was called. */
class MemoryStream
{
    public static array $files = [];
    public static ?string $hookAt = null;
    public static ?Closure $hook = null;

    public $context;
    private string $path;
    private int $position = 0;

    private static function fire(string $point): void
    {
        if (self::$hookAt === $point && self::$hook) {
            $hook = self::$hook;
            self::$hook = null;
            $hook();
        }
    }

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
        self::fire('open');
        return true;
    }

    public function stream_write(string $data): int
    {
        self::fire('write');
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
    public function stream_close(): void { self::fire('close'); }

    public function rename(string $from, string $to): bool
    {
        if (!isset(self::$files[$from])) {
            return false;
        }
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

function make_book(): ExcelBook
{
    $book = new ExcelBook(null, null, true);
    $book->addSheet('S')->write(1, 0, 'kept');
    return $book;
}

function describe(string|false $bytes): string
{
    if ($bytes === false || $bytes === '') {
        return 'missing';
    }
    $check = new ExcelBook(null, null, true);
    if (!$check->load($bytes)) {
        return 'unloadable';
    }
    $sheet = $check->getSheet(0);
    return $sheet->name() . ':' . $sheet->read(1, 0);
}

$reloadSource = new ExcelBook(null, null, true);
$reloadSource->addSheet('Z');
$reloadBytes = $reloadSource->save();

$tmpDir = sys_get_temp_dir();
$filePath = $tmpDir . '/excel-242-file-wrapper.xlsx';
@unlink($filePath);

stream_wrapper_unregister('data');
stream_wrapper_register('data', MemoryStream::class);

echo "-- data: no re-entry --\n";
$book = make_book();
var_dump($book->save('data:plain.xlsx'));
echo describe(MemoryStream::$files['data:plain.xlsx'] ?? false), "\n";

foreach (['open' => 'construct', 'write' => 'load', 'close' => 'construct'] as $point => $action) {
    echo "-- data: $action from stream_$point --\n";
    $book = make_book();
    MemoryStream::$hookAt = $point;
    MemoryStream::$hook = function () use ($book, $action, $reloadBytes) {
        if ($action === 'construct') {
            $book->__construct(null, null, true);
        } else {
            $book->load($reloadBytes);
        }
    };
    $name = "data:$point.xlsx";
    var_dump($book->save($name));
    var_dump(MemoryStream::$hook === null);
    echo describe(MemoryStream::$files[$name] ?? false), "\n";
    echo 'receiver sheets: ', $book->sheetCount(), "\n";
}

echo "-- data: loadFile goes through the wrapper --\n";
$book = new ExcelBook(null, null, true);
var_dump($book->loadFile('data:plain.xlsx'));
echo $book->getSheet(0)->read(1, 0), "\n";

stream_wrapper_restore('data');

echo "-- file: construct from stream_open --\n";
$book = make_book();
MemoryStream::$hookAt = 'open';
MemoryStream::$hook = function () use ($book) {
    $book->__construct(null, null, true);
};
stream_wrapper_unregister('file');
stream_wrapper_register('file', MemoryStream::class);
$saved = $book->save($filePath);
$leftovers = count(array_filter(array_keys(MemoryStream::$files), fn($k) => str_ends_with($k, '.tmp')));
stream_wrapper_restore('file');
var_dump($saved);
var_dump(MemoryStream::$hook === null);
echo describe(MemoryStream::$files[$filePath] ?? false), "\n";
var_dump($leftovers);
var_dump(file_exists($filePath));

echo "-- plain local save --\n";
$local = tempnam($tmpDir, 'excel-242-');
$book = make_book();
var_dump($book->save($local));
echo describe(file_get_contents($local)), "\n";
var_dump(count(glob($local . '.*.tmp')));
@unlink($local);
echo "OK\n";
?>
--CLEAN--
<?php
@unlink(sys_get_temp_dir() . '/excel-242-file-wrapper.xlsx');
?>
--EXPECT--
-- data: no re-entry --
bool(true)
S:kept
-- data: construct from stream_open --
bool(true)
bool(true)
S:kept
receiver sheets: 0
-- data: load from stream_write --
bool(true)
bool(true)
S:kept
receiver sheets: 1
-- data: construct from stream_close --
bool(true)
bool(true)
S:kept
receiver sheets: 0
-- data: loadFile goes through the wrapper --
bool(true)
kept
-- file: construct from stream_open --
bool(true)
bool(true)
S:kept
int(0)
bool(false)
-- plain local save --
bool(true)
S:kept
int(0)
OK

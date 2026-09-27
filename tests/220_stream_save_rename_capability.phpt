--TEST--
ExcelBook::save() probes user-wrapper rename support before writing a payload
--EXTENSIONS--
excel
--FILE--
<?php
class RenameCapabilityStream
{
    public $context;
    public static array $writes = [];
    public static array $opens = [];
    public static array $unlinks = [];
    public static array $closes = [];
    public static array $files = [];
    public string $path = '';

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->path = $path;
        self::$opens[] = $path;
        return true;
    }

    public function stream_write(string $data): int
    {
        self::$writes[] = $this->path;
        self::$files[$this->path] = (self::$files[$this->path] ?? '') . $data;
        return strlen($data);
    }

    public function stream_flush(): bool { return true; }
    public function stream_close(): void { self::$closes[] = $this->path; }
    public function unlink(string $path): bool
    {
        self::$unlinks[] = $path;
        unset(self::$files[$path]);
        return true;
    }

    public static function reset(): void
    {
        self::$writes = [];
        self::$closes = [];
        self::$opens = [];
        self::$unlinks = [];
        self::$files = [];
    }
}

stream_wrapper_register('rename-capability', RenameCapabilityStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'payload');

$missingRenameWarnings = [];
set_error_handler(function (int $severity, string $message) use (&$missingRenameWarnings): bool {
    $missingRenameWarnings[] = $message;
    if (str_contains($message, 'rename is not implemented')) {
        throw new ErrorException($message);
    }
    return true;
});
try {
    $result = $book->save('rename-capability://destination.xlsx');
    $exception = null;
} catch (Throwable $e) {
    $result = null;
    $exception = $e::class;
}
restore_error_handler();

var_dump($result);
var_dump($exception);
var_dump(count(RenameCapabilityStream::$writes));
var_dump(RenameCapabilityStream::$writes);
var_dump(count(RenameCapabilityStream::$unlinks));
var_dump(count(RenameCapabilityStream::$closes));
var_dump(strlen(RenameCapabilityStream::$files['rename-capability://destination.xlsx']) > 1000);
var_dump(count(array_filter(
    $missingRenameWarnings,
    static fn(string $message): bool => str_contains($message, 'rename is not implemented')
)));

stream_wrapper_unregister('rename-capability');

/* A real but failing rename is still an atomic-path failure: one staged write,
 * no direct destination write, and no silent retry. */
class FailingRenameStream extends RenameCapabilityStream
{
    public static int $renameCalls = 0;
    public function rename(string $from, string $to): bool
    {
        self::$renameCalls++;
        return false;
    }
}
stream_wrapper_register('failing-rename', FailingRenameStream::class);
FailingRenameStream::reset();
set_error_handler(static fn(): bool => true);
$failed = $book->save('failing-rename://destination.xlsx');
restore_error_handler();
var_dump($failed);
var_dump(FailingRenameStream::$renameCalls);
var_dump(FailingRenameStream::$writes);
var_dump(count(FailingRenameStream::$closes));
var_dump(isset(FailingRenameStream::$files['failing-rename://destination.xlsx']));
stream_wrapper_unregister('failing-rename');

/* A reservation-close exception aborts before unlink/direct fallback and is
 * propagated unchanged to the public save() caller. */
class FailingCloseStream extends RenameCapabilityStream
{
    public function stream_close(): void
    {
        self::$closes[] = $this->path;
        if (str_ends_with($this->path, '.tmp')) {
            throw new RuntimeException('staging close failed');
        }
    }
}
stream_wrapper_register('failing-close', FailingCloseStream::class);
FailingCloseStream::reset();
set_error_handler(static fn(): bool => true);
try {
    $closeFailedResult = $book->save('failing-close://destination.xlsx');
    $closeFailedClass = null;
    $closeFailedMessage = null;
} catch (Throwable $e) {
    $closeFailedResult = null;
    $closeFailedClass = $e::class;
    $closeFailedMessage = $e->getMessage();
}
restore_error_handler();
var_dump($closeFailedResult);
var_dump($closeFailedClass);
var_dump($closeFailedMessage);
var_dump(count(FailingCloseStream::$writes));
var_dump(count(FailingCloseStream::$closes));
var_dump(count(FailingCloseStream::$unlinks));
var_dump(isset(FailingCloseStream::$files['failing-close://destination.xlsx']));
stream_wrapper_unregister('failing-close');

$local = tempnam(sys_get_temp_dir(), 'excel-local-') . '.xlsx';
var_dump($book->save($local));
$reload = new ExcelBook(null, null, true);
var_dump($reload->loadFile($local));
var_dump($reload->getSheet(0)->read(1, 0));
@unlink($local);
echo "OK\n";
?>
--EXPECTF--
bool(true)
NULL
int(2)
array(2) {
  [0]=>
  string(%d) "rename-capability://destination.xlsx.%s.tmp"
  [1]=>
  string(36) "rename-capability://destination.xlsx"
}
int(1)
int(2)
bool(true)
int(0)
bool(false)
int(1)
array(1) {
  [0]=>
  string(%d) "failing-rename://destination.xlsx.%s.tmp"
}
int(1)
bool(false)
NULL
string(16) "RuntimeException"
string(20) "staging close failed"
int(1)
int(1)
int(1)
bool(false)
bool(true)
bool(true)
string(7) "payload"
OK

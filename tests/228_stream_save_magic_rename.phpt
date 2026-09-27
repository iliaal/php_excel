--TEST--
ExcelBook::save() dispatches inherited magic rename and fails closed on errors
--EXTENSIONS--
excel
--FILE--
<?php
class MagicRenameBase
{
    public $context;
    public static string $result = 'success';
    public static int $renames = 0;
    public static array $modes = [];
    private $handle;
    private string $mode;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        self::$modes[] = $this->mode = $mode;
        $this->handle = fopen(substr($path, strlen('magic-rename://')), $mode);
        return $this->handle !== false;
    }

    public function stream_write(string $data): int
    {
        return $this->mode === 'wb' ? 0 : fwrite($this->handle, $data);
    }

    public function stream_flush(): bool { return fflush($this->handle); }
    public function stream_close(): void { fclose($this->handle); }
    public function unlink(string $path): bool
    {
        return unlink(substr($path, strlen('magic-rename://')));
    }

    public function __call(string $method, array $arguments): bool
    {
        if ($method !== 'rename') {
            throw new BadMethodCallException($method);
        }
        self::$renames++;
        if (self::$result === 'throw') {
            throw new RuntimeException('magic rename failed');
        }
        if (self::$result === 'false') {
            return false;
        }
        return rename(
            substr($arguments[0], strlen('magic-rename://')),
            substr($arguments[1], strlen('magic-rename://'))
        );
    }
}
class MagicRenameStream extends MagicRenameBase {}

stream_wrapper_register('magic-rename', MagicRenameStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'payload');
$destination = tempnam(sys_get_temp_dir(), 'magic-rename-');
foreach (['success', 'false', 'throw'] as $result) {
    echo $result, "\n";
    file_put_contents($destination, 'ORIGINAL');
    MagicRenameStream::$result = $result;
    MagicRenameStream::$renames = 0;
    MagicRenameStream::$modes = [];
    $warnings = [];
    set_error_handler(static function (int $level, string $message) use (&$warnings): bool {
        $warnings[] = $message;
        return true;
    });
    try {
        var_dump($book->save('magic-rename://' . $destination));
    } catch (RuntimeException $e) {
        echo $e->getMessage(), "\n";
    } finally {
        restore_error_handler();
    }
    var_dump(MagicRenameStream::$renames === 1);
    var_dump(MagicRenameStream::$modes === ['xb']);
    var_dump(count($warnings) === ($result === 'false' ? 1 : 0));
    if ($result === 'success') {
        $reload = new ExcelBook(null, null, true);
        var_dump($reload->loadFile($destination));
        var_dump($reload->getSheet(0)->read(1, 0));
    } else {
        var_dump(file_get_contents($destination) === 'ORIGINAL');
    }
    var_dump(glob($destination . '.*.tmp') === []);
}
unlink($destination);
stream_wrapper_unregister('magic-rename');
?>
--EXPECT--
success
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
string(7) "payload"
bool(true)
false
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
throw
magic rename failed
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

--TEST--
ExcelBook::save() stages a payload probe for a wrapper with unlink() but no rename()
--EXTENSIONS--
excel
--DESCRIPTION--
A wrapper that implements unlink() but omits rename() used to discard the empty
staging reservation and write the destination directly, so a short write
truncated the caller's file. The staged write is now the probe for every
rename-less wrapper: a short write fails closed, and only a complete staged
write falls back to the non-atomic direct write.
--FILE--
<?php
class UnlinkOnlyStream
{
    public $context;
    public static int $limit = 0;
    public static array $writes = [];
    public static int $written = 0;
    public string $path = '';
    private $fh;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->path = $path;
        $this->fh = fopen(substr($path, strlen('unlinkonly://')), 'wb');
        return $this->fh !== false;
    }

    public function stream_write(string $data): int
    {
        self::$writes[] = $this->path;
        if (self::$limit > 0 && self::$written >= self::$limit) {
            return 0;
        }
        $chunk = self::$limit > 0 ? substr($data, 0, self::$limit - self::$written) : $data;
        $written = fwrite($this->fh, $chunk);
        self::$written += $written;
        return $written;
    }

    public function stream_flush(): bool { return true; }
    public function stream_close(): void { fclose($this->fh); }
    public function unlink(string $path): bool
    {
        return @unlink(substr($path, strlen('unlinkonly://')));
    }

    public static function reset(int $limit): void
    {
        self::$limit = $limit;
        self::$writes = [];
        self::$written = 0;
    }
}

stream_wrapper_register('unlinkonly', UnlinkOnlyStream::class);
$book = new ExcelBook(null, null, true);
$book->addSheet('S')->write(1, 0, 'payload');

$shortDest = tempnam(sys_get_temp_dir(), 'unlinkonly-short-') . '.xlsx';
$original = 'ORIGINAL_PAYLOAD_MUST_SURVIVE';
file_put_contents($shortDest, $original);

UnlinkOnlyStream::reset(8);
set_error_handler(static fn(): bool => true);
$shortResult = $book->save('unlinkonly://' . $shortDest);
restore_error_handler();
var_dump($shortResult);
var_dump(file_get_contents($shortDest) === $original);
var_dump(UnlinkOnlyStream::$writes);
var_dump(!str_ends_with(UnlinkOnlyStream::$writes[0], '.xlsx'));
foreach (glob($shortDest . '.*.tmp') as $temporary) {
    @unlink($temporary);
}
@unlink($shortDest);

$completeDest = tempnam(sys_get_temp_dir(), 'unlinkonly-full-') . '.xlsx';
UnlinkOnlyStream::reset(0);
set_error_handler(static fn(): bool => true);
$completeResult = $book->save('unlinkonly://' . $completeDest);
restore_error_handler();
var_dump($completeResult);
var_dump(count(UnlinkOnlyStream::$writes));
$reload = new ExcelBook(null, null, true);
var_dump($reload->loadFile($completeDest));
var_dump($reload->getSheet(0)->read(1, 0));
@unlink($completeDest);
stream_wrapper_unregister('unlinkonly');
echo "OK\n";
?>
--EXPECTF--
bool(false)
bool(true)
array(2) {
  [0]=>
  string(%d) "unlinkonly://%s.%s.tmp"
  [1]=>
  string(%d) "unlinkonly://%s.%s.tmp"
}
bool(true)
bool(true)
int(2)
bool(true)
string(7) "payload"
OK

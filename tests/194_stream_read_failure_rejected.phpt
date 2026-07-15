--TEST--
Stream-backed readers reject read failure instead of accepting accumulated bytes
--EXTENSIONS--
excel
--FILE--
<?php
class FailingReadStream
{
    public $context;
    public static string $data = '';
    public static int $failAfter = 0;
    private int $position = 0;

    public function stream_open(string $path, string $mode, int $options, ?string &$openedPath): bool
    {
        $this->position = 0;
        return true;
    }
    public function stream_read(int $count)
    {
        if ($this->position >= self::$failAfter) {
            return false;
        }
        $length = min($count, self::$failAfter - $this->position, strlen(self::$data) - $this->position);
        $chunk = substr(self::$data, $this->position, $length);
        $this->position += strlen($chunk);
        return $chunk;
    }
    public function stream_eof(): bool { return false; }
    public function stream_close(): void {}
}

stream_wrapper_register('failing-read', FailingReadStream::class);
$source = new ExcelBook(null, null, true);
$source->addSheet('Loaded')->write(1, 0, 'payload');
$raw = $source->save();
FailingReadStream::$data = $raw;
FailingReadStream::$failAfter = strlen($raw);

$load = new ExcelBook(null, null, true);
$load->addSheet('Before');
var_dump(@$load->loadFile('failing-read://book'));
var_dump($load->getSheetName(0));

if (method_exists($load, 'loadFilePartially')) {
    $partial = new ExcelBook(null, null, true);
    $partial->addSheet('Before');
    var_dump(@$partial->loadFilePartially('failing-read://book', 0, 1, 1));
    var_dump($partial->getSheetName(0));
} else {
    echo "bool(false)\nstring(6) \"Before\"\n";
}

$info = new ExcelBook(null, null, true);
$info->addSheet('Before');
var_dump(@$info->loadInfo('failing-read://book'));
var_dump($info->getSheetName(0));

FailingReadStream::$data = file_get_contents(__DIR__ . '/phplogo.jpg');
FailingReadStream::$failAfter = 8192;
$pictures = new ExcelBook(null, null, true);
var_dump(@$pictures->addPictureFromFile('failing-read://picture'));
?>
--EXPECT--
bool(false)
string(6) "Before"
bool(false)
string(6) "Before"
bool(false)
string(6) "Before"
bool(false)

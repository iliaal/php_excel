# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- libxl 5.2.0 support. New methods, available when built against
  libxl 5.2.0 or newer:
  - `ExcelSheet::dataValidationSize()` — number of data validations on
    the sheet.
  - `ExcelSheet::dataValidation(int $index)` — read a data validation by
    zero-based index, returned as an associative array (`type`, `op`,
    `row_first`, `row_last`, `col_first`, `col_last`, `value1`,
    `value2`); `false` for an out-of-range index. Note: libxl only
    surfaces validations parsed from xlsx files written by Excel (or
    another standards-compliant writer), not those added in the same
    session via `addDataValidation()`.
  - `ExcelTable::isAutoFilter()` — whether the table has an autofilter.
  - `ExcelTable::removeFilter()` — remove the table's autofilter.

### Fixed
- Reject child-wrapper handles from a different `ExcelBook`. libxl
  Format/Font/RichString/AutoFilter/ConditionalFormat handles are scoped to
  the workbook that created them; applying one to another workbook silently
  produced wrong output and could dangle once the source book was freed.
  `ExcelSheet::write`/`writeRow`/`writeCol`/`setCellFormat`/`writeError`/
  `setColWidth`/`setRowHeight`/`setColPx`/`setRowPx`/`writeRichStr`/
  `applyFilter2`, `ExcelFormat::setFont`, and the `ExcelConditionalFormatting`
  rule methods now return `false` with a warning instead. Template-copy
  methods (`ExcelBook::addFormat`/`addFont`/`insertSheet`) still accept a
  handle from another book by design.
- Reject out-of-range values in libxl integer/enum/RGB setters instead of
  silently truncating the 64-bit argument to `int`. This is a complete sweep of
  every public `int`/enum setter, not only the `ExcelSheet` ones:
  `ExcelSheet` (`setZoom`, `setZoomPrint`, `setPaper`, `setPrintFit`,
  `setBorder`, `setColPx`/`setRowPx` pixel sizes, `setTabColor`,
  `setRightToLeft`, `writeComment` width/height), `ExcelBook` (`setCalcMode`,
  `setDefaultFont` size), `ExcelConditionalFormat` (all `setBorder*`,
  `setNumFormat`, `setFillPattern`, `setPattern*Color`), `ExcelFormControl`
  (`setChecked`, `setDropLines`, `setDx`, `setFirstButton`, `setHoriz`,
  `setInc`, `setMax`, `setMin`, `setSel`), `ExcelTable::setStyle`, and
  `ExcelFilterColumn::setCustomFilter` operators. `ExcelSheet::setTabRgbColor`
  requires each component in 0-255. (`ExcelFormat`/`ExcelFont` setters already
  range-checked.)

### Changed
- Tightened the published parameter types for several `ExcelSheet` setters that
  were declared `mixed` but parse a concrete scalar: `setZoom`/`setZoomPrint`
  (`int`), `setPrintGridlines`/`setLandscape`/`setHCenter`/`setVCenter`
  (`bool`), `setMarginLeft`/`setMarginRight`/`setMarginTop`/`setMarginBottom`
  (`float`), and `setHeader`/`setFooter` (`string`, `float`). Reflection and
  static analysis now report the real signatures.
- `ExcelAutoFilter::getSort()` accepts an optional zero-based sort
  `$level` argument (libxl 5.2.0+), reflecting libxl's added multi-level
  sort read support. Calls with no argument behave as before.

## [2.0.1] - 2026-05-03

### Changed (semantics)
- Optional getter/setter methods on `ExcelFormat` and `ExcelFont` now
  treat explicit `null` as "getter mode" instead of silently mutating
  state. Previously the `|l` ZPP weak-coerced `null` to `0`, fired the
  setter, and reset the underlying slot — `$F->numberFormat(null)`
  reset format `7` to `0` on a Format already pointed at format `7`,
  and `$f->name(null)` reset `"Arial"` to `""` because `|S` weak-
  coerced `null` to `""`. The new semantics: argument omitted or
  `null` → getter; any non-null value → setter (range-checked against
  the libxl `int` boundary). Affects `ExcelFormat::numberFormat`,
  `horizontalAlign`, `verticalAlign`, `wrap`, `rotate`, `indent`,
  `shrinkToFit`, `borderStyle`, `borderColor`, `borderLeftStyle`/
  `Color`, `borderRightStyle`/`Color`, `borderTopStyle`/`Color`,
  `borderBottomStyle`/`Color`, `borderDiagonalStyle`/`Color`,
  `fillPattern`, `patternForegroundColor`, `patternBackgroundColor`,
  `locked`, `hidden`, `ExcelFont::size`, `italics`, `strike`, `bold`,
  `color`, `mode`, `underline`, `name`. Stubs and IDE reference docs
  are updated accordingly (`?int $foo = null`, `?bool $foo = null`,
  `?string $name = null`).

### Added
- Comprehensive test coverage for FormControl (all 7 control types)
- Tests for previously untested methods: Book::addPictureAsLink,
  Book::conditionalFormat, Sheet::removeComment
- Tests for untested optional parameters across ExcelBook, ExcelSheet,
  ExcelFormat, ExcelFilterColumn, ExcelRichString, ExcelConditionalFormatting,
  and ExcelTable classes
- Licensed-only test for row 0 access and large cell counts

### Changed
- Tag convention: 2.0.1+ uses bare semver tags (e.g. `2.0.1`). 2.0.0 was tagged as `v2.0.0`.
- Migrated to stub-driven arginfo (`excel.stub.php` + generated
  `excel_arginfo.h`). Method parameter and return types are now declared
  and visible to Reflection / IDEs / static analyzers; previously they
  were untyped at the engine boundary on most methods. Behavior is
  unchanged. `excel.c` shrinks by 2010 lines (the inline arginfo blocks
  and per-class function-entry tables now come from the generated header).
  libxl-version-conditional methods (`addConditionalFormatting`,
  `loadInfoRaw`, `setPassword`, etc.) use `#if LIBXL_VERSION >= ...`
  blocks in the stub; gen_stub passes them through to the generated
  header.
- `php_excel.h` carries a polyfill for
  `zend_register_internal_class_with_flags` (added in PHP 8.4) so the
  generated arginfo header compiles cleanly against the project's
  PHP 8.3 minimum.

### Fixed (correctness)
- `Book::getSheet()`, `Book::deleteSheet()`, `Book::getSheetName()`,
  `Book::sheetType()`, `Book::copySheet()`, `Book::setActiveSheet()`,
  `Book::activeSheet()`, `Book::conditionalFormat()`, `Book::getPicture()`,
  `Book::moveSheet()`, `Sheet::delHyperlink()`, `Sheet::merge()`,
  `Sheet::delMergeByIndex()`, `Sheet::getNamedRange()`,
  `Sheet::getVerPageBreak()`, `Sheet::getHorPageBreak()`,
  `Sheet::getPictureInfo()`, `Sheet::hyperlink()`, `Sheet::table()`,
  `Sheet::removePictureByIndex()`, `Sheet::formControl()`,
  `Sheet::getTableByIndex()`, `Sheet::conditionalFormatting()`,
  `Sheet::removeConditionalFormatting()`, `Sheet::addPictureScaled()`,
  `Sheet::addPictureDim()`, `AutoFilter::column()`,
  `AutoFilter::columnByIndex()`, `AutoFilter::setSort()`,
  `AutoFilter::addSort()`, `FilterColumn::__construct()`,
  `FilterColumn::filter()`, `RichString::getText()`,
  `FormControl::__construct()`, `FormControl::item()`, and
  `Table::columnName()` now reject sheet/index values that exceed
  `INT_MAX` instead of silently wrapping when libxl narrows to `int`.
  Previously `getSheet(2**32)` aliased to index `0` and
  `deleteSheet(2**32)` deleted sheet `0`; the same narrowing
  applied to every other `zend_long`-to-libxl-`int` boundary,
  including `addPictureScaled` / `addPictureDim` where `pic_id`
  and the dimensional / offset / pos arguments wrapped — a
  `pic_id = 2**32` call used to alias to picture index 0 and
  silently embed the wrong picture into the sheet.
- Non-index integer setters on `ExcelFormat` and `ExcelFont`
  (`numberFormat`, all `border*Style` / `border*Color`,
  `fillPattern`, `pattern*Color`, `Font::size`, `Font::color`,
  `Font::mode`, `Font::underline`, `rotate`, `indent`) now reject
  values outside `[0, INT_MAX]` instead of silently truncating
  on the implicit `int` cast — `numberFormat(2**32 + 1)` and
  `Font::color(2**32 + 1)` previously both became `1`.
- Stub argument types and runtime ZPP signatures for `Sheet::groupRows()`,
  `Sheet::groupCols()`, `Sheet::setPrintHeaders()`, and
  `ExcelFormat::wrap()` / `shrinkToFit()` / `locked()` / `hidden()` are
  now consistent. The stubs declared `int`/`string`/`mixed` for
  parameters the C parsed as `bool`, fataling under debug PHP's arginfo
  / ZPP checker on calls like `setPrintHeaders(true)`. The IDE
  reference files in `docs/` (`ExcelSheet.php`, `ExcelFormat.php`,
  `ExcelAutoFilter.php`) were also updated so their published
  signatures match runtime reflection — `groupRows`/`groupCols` now
  publish `bool $collapse = false`, `setPrintHeaders` publishes
  `bool $value`, `wrap`/`shrinkToFit`/`locked`/`hidden` publish
  `bool ... = false`, and `ExcelAutoFilter::setRef` publishes the
  required four `int` arguments instead of optional defaults.
- `ExcelAutoFilter::__construct()` stub now requires the `ExcelSheet`
  argument (was advertised as optional nullable). The C had always
  required it; reflection-driven callers replaying the documented
  default `null` got `TypeError`.
- `Sheet::addDataValidationDouble()` left the optional `$val_2` parameter
  uninitialized when the caller used a non-(NOT)BETWEEN operator. The C
  declared `double val_1, val_2;` without a default, so the unused slot
  forwarded a stack-garbage value to `xlSheetAddDataValidationDoubleEx`.
  Initialized to `0.0` so the unused slot is deterministic.
- Stub defaults for `Sheet::addDataValidation()` and
  `Sheet::addDataValidationDouble()` now match the C implementation:
  `allow_blank=true`, `show_inputmessage=true`, `show_errormessage=true`,
  `error_style=1`. The previous stub said `false`/`0`, so reflection-
  driven callers using `getDefaultValue()` got opposite behavior from
  the documented defaults.
- `Sheet::addDataValidation()` and `Sheet::addDataValidationDouble()`
  now treat `null` (and, for the string variant, an empty string) as
  "second value not supplied". The check that BETWEEN/NOT-BETWEEN
  operators require a second endpoint previously only looked at
  `ZEND_NUM_ARGS()`, so a reflection-driven caller replaying
  `getDefaultValue()` (`""` for string, `0.0` for double) sneaked past
  the guard and produced a one-sided rule. Stub `$val_2` is now
  `?string $val_2 = null` / `?float $val_2 = null`. The double variant
  uses FAST_ZPP `Z_PARAM_DOUBLE_OR_NULL` so PHP's standard `d`
  coercion (numeric strings, bool, int) still applies to `$val_2`,
  matching `$val_1`. Only explicit `null` (or omission) trips the
  BETWEEN guard.
- `Book::activeSheet()`'s stub default is now `-1` (the C "getter mode"
  sentinel). The previous default of `0` caused reflection-driven calls
  to silently switch the workbook back to sheet index 0 instead of just
  returning the current active sheet.
- Stale ZPP comments in `excel.stub.php` for `insertSheet`, `setCellFormat`,
  and `writeError` now reflect the typed-object signatures actually
  parsed by the C (`O`/`O!`).

### Security
- `ExcelSheet::setColWidth()` and `setRowHeight()` now reject non-`ExcelFormat`
  objects with `TypeError` instead of crashing. ZPP previously parsed the
  optional format as a generic zval, so passing a scalar would feed garbage
  to `FORMAT_FROM_OBJECT()` and segfault.
- `Sheet::applyFilter2()`, `ConditionalFormatting::addRule()` /
  `addTopRule()` / `addOpNumRule()` / `addOpStrRule()` /
  `addAboveAverageRule()` / `addTimePeriodRule()`, and `Sheet::writeRichStr()`
  now route their object arguments through the standard `*_FROM_OBJECT`
  macros so a stale `ExcelAutoFilter`/`ExcelConditionalFormat`/
  `ExcelRichString` (after `Book::load()` etc.) raises a warning and
  returns `false` instead of dereferencing a freed libxl handle.
- ZPP signatures for `Sheet::setCellFormat()`, `Sheet::writeError()`, and
  `Book::insertSheet()` were tightened from generic `o` to `O`/`O!` with
  the expected class entry. Previously the methods accepted any object,
  reaching the `*_FROM_OBJECT` macros with a `stdClass` and producing an
  arginfo/ZPP fatal under debug PHP and undefined behaviour otherwise.
  Calls now raise `TypeError` at the boundary as the stub advertises.
- `Book::loadInfo()` and `Book::addPictureAsLink()` now run
  `php_check_open_basedir()` before handing the path to libxl. The two
  methods talked to libxl directly without a stream wrapper, so paths
  outside `open_basedir` were readable / linkable into otherwise
  permitted output workbooks.
- `ExcelFormat::__construct()`, `ExcelFont::__construct()`,
  `ExcelSheet::__construct()`, `ExcelRichString::__construct()`,
  `ExcelConditionalFormat::__construct()`, and
  `ExcelCoreProperties::__construct()` now throw when handed an
  uninitialized `ExcelBook` (e.g. one obtained via
  `ReflectionClass::newInstanceWithoutConstructor()`). Previously
  `BOOK_FROM_OBJECT` warned and returned `false`, but PHP ignores
  constructor return values, so the caller received an unusable child
  wrapper that crashed only on first use.
- `Sheet::writeRow()` and `Sheet::writeCol()` pre-validate the full target
  range before mutating any cell. An overflowing run (e.g. XLS
  `writeRow(1, [a, b], 255)` where col `255+1=256` is past the limit)
  used to write the first cells and only fail on the overflowing one,
  leaving partial data behind. Now it fails before any write. The
  start coordinate is validated before any signed arithmetic on it,
  so extreme inputs (`PHP_INT_MIN` / `PHP_INT_MAX`) don't trip UBSan.
- `Book::moveSheet()` bumps the book generation on success. Previously
  existing `ExcelSheet` wrappers silently retargeted to the wrong sheet
  when indices shifted under them.

- Stale child wrapper crashes: an `ExcelSheet`/`ExcelFormat`/`ExcelFont`/etc.
  retained from before `Book::load()`, `Book::loadFile()`, `Book::loadInfo()`,
  `Book::loadInfoRaw()`, `Book::clear()`, `Book::deleteSheet()`, or a manual
  `Book::__construct()` reuse now refuses to call libxl with a stale handle.
  Previously, calling a method on such a wrapper could segfault
  (`pure virtual method called`, `SEGV in xlFormatNumFormatA`, etc.). A
  book-level generation counter is bumped on each invalidating operation;
  child wrappers stamp the value at creation and check it before every libxl
  call, returning `false` with a warning when stale. Sibling sheets must be
  re-fetched after `deleteSheet()`.
- Stale-wrapper checks extended to `ExcelFont` and `ExcelFormat` clone
  handlers, which previously bypassed the FROM_OBJECT macros and called
  `xlBookAddFont`/`xlBookAddFormat` directly with freed handles. `clone $f`
  on a stale wrapper now throws an exception instead of producing an ASAN
  SEGV.
- NUL-byte protection extended to
  `ExcelConditionalFormatting::add2ColorScaleFormulaRule()` and
  `add3ColorScaleFormulaRule()` (formula strings) and to
  `ExcelSheet::addDataValidationDouble()` (prompt/error strings) — all
  previously passed `ZSTR_VAL()` directly to libxl, where embedded NULs
  would silently truncate the value while peer methods rejected it.
- `ExcelBook::__construct()` with a NUL-bearing license name or key now
  throws an exception. Previously it emitted a warning and returned an
  initialized workbook, since PHP ignores constructor return values —
  callers received a usable object built from rejected input.
- NUL-byte gaps closed on the libxl boundary. Embedded NUL bytes are now
  rejected (rather than silently truncating the value) in `Sheet::write()`,
  `Sheet::writeRow()`, `Sheet::writeCol()` cell strings; `Book::getSheetByName()`;
  `Book::loadFile()`, `Book::save()`, `Book::addPictureFromFile()` paths; and
  the `ExcelBook::__construct()` license name/key arguments.
- `Book::getSheetByName()` now compares names with binary-safe length-aware
  comparison rather than `strcmp`/`strcasecmp`, eliminating prefix-match
  false positives on names that differ only past an embedded NUL.

### Fixed
- `Sheet::write()` / `writeRow()` / `writeCol()` now return `false` for
  unsupported zval types (array, object, resource). Previously they emitted a
  warning but returned `true`, leading callers to believe data was persisted
  when the cell was actually left empty.
- Coordinate validation: `Sheet::read()`, `Sheet::write()`, `Sheet::cellType()`,
  `Sheet::cellFormat()`, `Sheet::setCellFormat()`, `Sheet::isDate()`,
  `Sheet::isFormula()`, `Sheet::writeRow()`, and `Sheet::writeCol()` now
  reject cell coordinates outside the workbook's format-specific limits —
  XLSX (1048576 rows x 16384 cols) for books created with
  `new ExcelBook(null, null, true)`, XLS (65536 rows x 256 cols) otherwise —
  instead of silently truncating to libxl's `int` and returning empty cells.
  Read paths previously accepted any in-range integer because libxl doesn't
  range-check reads. `Sheet::insertRow()`, `Sheet::removeRow()`,
  `Sheet::insertCol()`, and `Sheet::removeCol()` validate against the
  appropriate axis limit for both arguments (the second argument is a row
  or column endpoint, not the perpendicular axis). Coordinate validation
  also covers `Sheet::rowColToAddr()`, `setActiveCell()`, `setPrintArea()`,
  `setMerge()`, `getMerge()`, `deleteMerge()`, `clear()` (range), `copy()`,
  `splitSheet()`, `setTopLeftView()`, `readComment()`, `writeComment()`,
  `removeComment()`, `removePicture()`, `readRichStr()`, `writeRichStr()`,
  `hyperlinkIndex()`, `writeError()`, `addPictureScaled()`, `addPictureDim()`,
  `setColWidth()`, `setColPx()`, `setRowPx()`, `setBorder()`, `addTable()`,
  `setPrintRepeatRows()`, `setPrintRepeatCols()`,
  `addDataValidation()`/`addDataValidationDouble()`,
  `AutoFilter::setRef()`, `Sheet::addConditionalFormatting()`,
  `ConditionalFormatting::addRange()`,
  `ConditionalFormatting::__construct()` (libxl 5.1.0+ 5-arg form),
  `Sheet::colWidth()`, `rowHeight()`, `colWidthPx()`, `rowHeightPx()`,
  `colFormat()`, `rowFormat()`, `rowHidden()`, `setRowHidden()`,
  `colHidden()`, `setColHidden()`, `groupRows()`, `groupCols()`,
  `setAutoFitArea()`, and `addIgnoredError()`.
- `ExcelConditionalFormatting::__construct()` stub now exposes the
  libxl-5.1.0+ 5-argument form via gen_stub `#if/#else` so reflection
  reports the same shape that ZPP actually parses. Previously the stub
  advertised a 1-argument constructor and instantiating it on libxl 5.1.0
  failed with `ArgumentCountError`. Bad-coordinate inputs now throw an
  exception (PHP ignores constructor return values, so `RETURN_FALSE`
  would have left the caller with an uninitialized wrapper).
- `ExcelAutoFilter::__construct()`, `ExcelFilterColumn::__construct()`,
  `ExcelConditionalFormatting::__construct()`, `ExcelTable::__construct()`,
  and `ExcelFormControl::__construct()` now throw when handed a stale
  parent (sheet or autofilter from a book that has been reloaded,
  cleared, or reinitialized). Previously the underlying `*_FROM_OBJECT`
  macros emitted a warning and `RETURN_FALSE`, but PHP ignores
  constructor return values, so callers received an uninitialized
  wrapper that failed only on first use.
- `ExcelBook::addFont(null)`, `ExcelBook::addFormat(null)`, and
  `ExcelSheet::writeRow(..., null)` now match the nullable arginfo
  declared in the stub. ZPP was `|O` / `la|lO` (non-nullable), so
  passing the explicit `null` advertised by reflection raised
  `TypeError`. Switched to `|O!` / `la|lO!`.
- `ExcelSheet::writeCol()`'s first argument is now named `$column` in
  the stub (was `$row`). The C implementation has always parsed it as
  the column index; reflection and named arguments were misleading
  callers.
- `Sheet::read()`, `Sheet::readRow()`, and `Sheet::readCol()` stub
  defaults aligned with the C implementation. The stubs declared
  `read_formula = false` and `end_column/end_row = 0`; the C parses
  defaults are `1` (true) and `-1` ("read to last column/row"). Calling
  these methods with no optional arguments behaves as the C has always
  behaved, but reflection and static analysis now match.

### Performance
- `Book::loadFile()`, `Book::save($path)`, and `Book::addPictureFromFile()`
  use libxl's direct path APIs (`xlBookLoad`/`xlBookSave`/`xlBookAddPicture`)
  for plain filesystem paths that pass the `open_basedir` check. Previously
  every call read or wrote the entire workbook through a PHP stream into a
  full in-memory buffer before handing the bytes to libxl, doubling peak
  memory for large files. Stream-wrapper paths (`phar://`, `php://`, etc.)
  still go through the original wrapper machinery.
- `Sheet::write()` / `writeRow()` / `writeCol()` with `ExcelFormat::AS_DATE`
  and no explicit format now share a single book-cached date format
  instead of allocating a new format per cell. Five sequential AS_DATE
  writes used to grow `Book::getAllFormats()` by five entries; they now
  grow by one. Bulk date exports stay well below libxl's per-book
  format-table cap. The cache is cleared on book-state resets that
  free libxl's format table (`load()`, `loadFile()`, `loadInfo()`,
  `loadInfoRaw()`, `clear()`, `__construct()` reuse), so post-reset
  AS_DATE writes don't reuse the freed handle and still record as
  dates. Sheet-index shifts (`deleteSheet()`, `moveSheet()`) preserve
  the cache because libxl preserves the format table across them.
- `Sheet::horPageBreak()` and `Sheet::verPageBreak()` validate the page-
  break coordinate against the row/column axis of the workbook
  respectively. They previously accepted up to `INT_MAX`, so XLSX
  `horPageBreak(1048576)` and XLS `verPageBreak(300)` would succeed and
  silently produce a no-op break.
- `Sheet::addHyperlink()` and `Sheet::setNamedRange()` now use the
  workbook-aware row- and column-range validators instead of the generic
  `INT_MAX` check, matching the rest of the range-taking sheet methods.
- `Sheet::autoFilter()`, `Sheet::applyFilter()`, `Sheet::removeFilter()`, and
  `Sheet::splitInfo()` now call `ZEND_PARSE_PARAMETERS_NONE()` so calling
  them with extra arguments raises `ArgumentCountError` rather than a debug-
  PHP arginfo/ZPP mismatch fatal.
- Stub signatures corrected to match the C implementation:
  `Book::addPictureFromFile(string)`, `Book::addPictureFromString(string)`
  (were `mixed`/`bool`); `Sheet::insertRow/insertCol/removeRow/removeCol(int, int)`
  (the spurious third `update_named_ranges` parameter is removed —
  the underlying C ZPP only ever parsed `"ll"`); `Sheet::horPageBreak(int, bool)`,
  `Sheet::verPageBreak(int, bool)` (were `int, int`); `Sheet::setPaper(int)`
  (was `string`); `Sheet::setPrintRepeatRows/Cols(int, int)` (had drifted
  to `mixed`/`bool`); `ConditionalFormatting::addRule/addOpNumRule/
  addOpStrRule/addAboveAverageRule/addTimePeriodRule` `stopIfTrue` is
  `bool` (was `string`/`float`/`int`); `ConditionalFormatting::addTopRule`
  `bottom` is `bool` (was `int`); `ConditionalFormatting::addAboveAverageRule`
  `stdDev` is `int` (was `bool`); `ExcelTable::__construct` `rowFirst:int`
  / `hasHeaders:bool` / `style:int` (were `string`/`int`/`bool`).
  Reflection, IDEs, and static analyzers now see the true signatures.
- LibXL 4.6.0 test compatibility: tests calling LibXL 5.x-only APIs
  (`addConditionalFormatting` 4-arg, `dpiAwareness`, `conditionalFormatSize`,
  etc.) now skip via `method_exists()` instead of failing.
- Test 002 (date pack/unpack) timezone detection to work cross-platform

## [2.0.0] - 2026-04-05
	* Added PHP 8.3, 8.4, 8.5 and master support
	* Minimum PHP version is now 8.3
	* Minimum LibXL version is now 4.6.0
	* Added PIE (PHP Installer for Extensions) support via composer.json
	* Fixed crash when trying to read an empty file (from unreleased 1.0.3)
	* Fixed bug with writing references (see issue #234, from unreleased 1.0.3)
	* Fixed memory leaks in setProtect(), addDataValidation(), addDataValidationDouble()
	* Fixed use-after-free when parent object is GC'd before child
	* Fixed memory corruption in getIndexRange() on 64-bit (int/zend_long aliasing)
	* Fixed NULL pointer dereferences in FilterColumn::filter(), getCustomFilter(),
	  addPicture(), Sheet::writeError()
	* Fixed Sheet::autoFilter() returning true instead of ExcelAutoFilter object
	* Fixed AutoFilter::column()/columnByIndex() returning true instead of FilterColumn
	* Fixed AutoFilter::getRef()/getSortRange() writing wrong value to col_last key
	* Fixed FilterColumn::__construct() arginfo/parameter mismatch
	* Fixed Book::getSheetName() rejecting 0-based index
	* Fixed Sheet::cellFormat() not setting book handle on returned format
	* Fixed constructors returning false on error (now throw exceptions)
	* Replaced all zend_bool with bool, long with zend_long
	* Replaced getThis() with ZEND_THIS, ZEND_NUM_ARGS() checks with
	  ZEND_PARSE_PARAMETERS_NONE()
	* All classes marked ZEND_ACC_NOT_SERIALIZABLE
	* Added typed parameter arginfo (399 parameters) and return type arginfo
	  (277 methods)
	* Added new classes:
		- ExcelRichString (rich text with mixed fonts in a single cell)
		- ExcelFormControl (checkboxes, dropdowns, buttons, spinners, etc.)
		- ExcelConditionalFormat (conditional formatting style rules)
		- ExcelConditionalFormatting (conditional formatting ranges and rules)
		- ExcelCoreProperties (workbook metadata: title, author, dates, etc.)
		- ExcelTable (structured table support for xlsx)
	* Added new ExcelBook methods:
		- addRichString(), calcMode(), setCalcMode()
		- addConditionalFormat(), addFormatFromStyle()
		- removeVBA(), removePrinterSettings()
		- dpiAwareness(), setDpiAwareness()
		- coreProperties(), removeAllPhonetics()
		- getLibXlVersion(), getPhpExcelVersion()
		- addPictureAsLink(), moveSheet()
	* Added new ExcelBook methods (requires LibXL 5.0.0+):
		- setPassword(), dpiAwareness(), setDpiAwareness()
	* Added new ExcelBook methods (requires LibXL 5.0.1+):
		- loadInfoRaw()
	* Added new ExcelBook methods (requires LibXL 5.1.0+):
		- errorCode(), conditionalFormat(), conditionalFormatSize(), clear()
	* Added new ExcelSheet methods:
		- firstFilledRow(), lastFilledRow(), firstFilledCol(), lastFilledCol()
		- removePicture(), removePictureByIndex()
		- isRichStr(), readRichStr(), writeRichStr()
		- formControlSize(), formControl()
		- getActiveCell(), setActiveCell()
		- selectionRange(), addSelectionRange(), removeSelection()
		- tabColor(), getTabRgbColor(), setTabRgbColor()
		- hyperlinkIndex()
		- colWidthPx(), rowHeightPx(), colFormat(), rowFormat()
		- setColPx(), setRowPx(), setBorder()
		- addTable(), getTableByName(), getTableByIndex()
		- applyFilter2()
		- addConditionalFormatting()
		- addDataValidation(), addDataValidationDouble(), removeDataValidations()
	* Added new ExcelSheet methods (requires LibXL 5.1.0+):
		- conditionalFormatting(), removeConditionalFormatting()
		- conditionalFormattingSize()
	* Added ExcelAutoFilter::addSort()
	* Added ExcelSheet::AS_STRING const (from unreleased 1.0.3, PR 183 by ederuiter)
	* Added constants for CalcMode, CellStyle, TableStyle, FormControl types,
	  ConditionalFormat types/operators/time periods, and CFVOType

## [1.0.2] - 2016-06-23
	* Fixed bug in ExcelSheet::addPictureDim() (see issue #120)
	* Fixed bug in ExcelSheet::isLicensed() (see issue #122)
	* Added new methods (requires LibXL 3.6.2)
		- ExcelBook::setAutoFitArea()
		- ExcelBook::printArea()
		- ExcelBook::printRepeatCols()
		- ExcelBook::printRepeatRows()
	* Added support for LibXL 3.6.3
	* ExcelSheet::addPictureDim() & ExcelSheet::addPictureScaled() now support position parameters
	* Includes official PHP7 release
	* Added support for libxl compiled from source

## [1.0.1] - 2015-02-26
	* Added methods
		- ExcelBook::sheetType()
		- ExcelSheet::colHidden()
		- ExcelSheet::rowHidden()
		- ExcelSheet::setColHidden()
		- ExcelSheet::setRowHidden()
		- ExcelSheet::isLicensed()
	* Added new methods (requires LibXL 3.6.0)
		- ExcelSheet::hyperlinkSize()
		- ExcelSheet::hyperlink()
		- ExcelSheet::delHyperlink()
		- ExcelSheet::addHyperlink()
		- ExcelSheet::mergeSize()
		- ExcelSheet::merge()
		- ExcelSheet::delMergeByIndex()
		- ExcelSheet::splitInfo()
	* Added data type argument for ExcelSheet::writeCol() (see issue #29)
	* Fixed bug in ExcelSheet::read() (see issue #86)
	* Fixed bug in ExcelBook::setDefaultFont() (see issue #66)
	* Added autofit support for ExcelBook::setColWidth()
	* Added implicit formula recognition on ExcelSheet::write() operations (prefix formula with '=')
	* Added multibyte support for license name and license key (see issue #60 and README)
	* Added default date format when writing dates
	* Added ExcelSheet::__construct(ExcelBook $book, $name) for customized ExcelSheets
	* Added warnings on read/write errors
	* Removed final keyword for ExcelFormat::__construct(ExcelBook $book)
	* Removed invalid PAPER_* constants in ExcelSheet
	* Removed invalid COLOR_* constants in ExcelFormat
	* Updated documentation and README
	* Changed php.ini variable name: excel.ini_skip_empty --> excel.skip_emtpy

## [1.0] - 2014-06-03
	* Fixed issue #63 writing for NULL values using writeRow()
	* Added excel.ini_skip_empty INI setting that allows null values to be skipped, if set to 2 the empty strings will be skipped as well
	* Added Sheet::setRightToLeft() and Sheet::setRightToLeft() methods that specifies whether the sheet in is "right to left" display mode
	* Added Book::isTemplate() and Book::setTemplate() methods for detecting xltx files and converting from xlsx to xltx and vice versa
	* Fixed compilation with LibXL 3.5.4
	* Added optional scope parameter to setNamedRange()/delNamedRange()/getNamedRange() methods
	* Updated getNamedRange()/getIndexRange() methods to retrieve hidden state
	* Added parameter to readRow/readCol/read methods that specifiy whether to parse formula or its value.
	* Added ExcelBook::packDateValues() method for generating Excel date value
	* Added ExcelBook::setPrintArea() method for setting print area
	* Fixed parsing of non-numeric formulas inside read() method
	* Fixed parameter processing inside ExcelSheet::getVerPageBreak()/getHorPageBreak() methods
	* Fixed parameter order inside ExcelSheet::getNamedRange()/getIndexRange() methods
	* Fixed return value processing inside ExcelBook::setActiveSheet() method

## [0.9.9] - 2013-11-18
	* Added ExcelBook::isDate1904()/ExcelBook::setDate1904() methods to set/retrieve base date format
	* Allow compilation against LibXL 3.5.3
	* Fixed bug with parameter order inside setNamedRange method (jacksonja)

## [0.9.8] - 2012-12-31
	* Allow compilation against LibXL 3.4
	* Updated ExcelSheet::addPictureScaled() and ExcelSheet::addPictureDim() to support optional x/y offset parameters

## [0.9.7] - 2012-10-16
	* Updated INFO section of the extension to show libXL version
	* Added new methods (requires LibXL 3.2.4+):
		- added ExcelSheet::getTopLeftView() and ExcelSheet::setTopLeftView() methods for getting/setting a view position in the sheet
		- added ExcelSheet::addrToRowCol() and ExcelSheet::rowColToAddr() methods for translating Excel cell position to row/column definitions

## [0.9.6] - 2011-10-12
	* Added new methods (requires LibXL 3.2.3+):
		- ExcelSheet::isHidden() that whether sheet is hidden.
		- ExcelSheet::setHidden(bool flag) that hides/unhides the sheet.
	* Allow compilation against LibXL 3.2.3/3.2.2
	* Added a work-around for image index bug inside LibXL 3.2.2

## [0.9.1] - 2011-08-03
	* Added new methods (requires LibXL 3.2.0+):
		- ExcelSheet::setPrintFit(int wPages, int hPages) that fits sheet width and sheet height to wPages and hPages respectively
		- ExcelSheet::getPrintFit() that returns whether fit to page option is enabled, and if so to what width & height
		- ExcelSheet::getNamedRange(string name) that gets the named range coordianates by name, returns false if range is not found
		- ExcelSheet::getIndexRange(int index) that gets the named range coordianates by index, returns false if range is not found
		- ExcelSheet::namedRangeSize() that returns the number of named ranges in the sheet
		- ExcelSheet::getVerPageBreak(int index) that returns column with vertical page break at position index
		- ExcelSheet::getVerPageBreakSize() that returns a number of vertical page breaks in the sheet
		- ExcelSheet::getHorPageBreak(int index) that eturns column with horizontal page break at position index
		- ExcelSheet::getHorPageBreakSize() that returns a number of horizontal page breaks in the sheet
		- ExcelSheet::getPictureInfo(int index) that returns a information about a workbook picture at position index in worksheet
		- ExcelSheet::getNumPictures() that returns a number of pictures in this worksheet
		- ExcelBook::biffVersion() that returns BIFF version of binary file. (Used for xls format only)
		- ExcelBook::getRefR1C1() that returns whether the R1C1 reference mode is active
		- ExcelBook::setRefR1C1(bool active) that sets the R1C1 reference mode
		- ExcelBook::getPicture(int picture_index) that returns a picture at position index
		- ExcelBook::getNumPictures() that returns a number of pictures in this workbook
		- ExcelSheet ExcelBook::insertSheet(int index, string name [, ExcelSheet sh]) that inserts a new sheet to this book at position index,
		  returns the sheet handle. If ExcelSheet parameter is missing a new sheet will be created.

## [0.9.1] - 2011-01-14
	* Added support for LibXL 3.1.0+
	* Added work-around for a bug inside xlSheetIsDate() that detects treats custom number formats as dates
	* Added ExcelBook::getSheetByName() method to retrieve sheet by name (original patch by Rob Gagnon)
	* Added basic documentation
	* Fixed a bug with ExcelSheet::readRow()/ExcelSheet::readCol() when end parameter is specified

## [0.9.0] - 2010-08-27
	* Added support for LibXL 3.1.0
	* Added new methods
		- ExcelSheet::setNamedRange(string name, int row, int col, int to_row, int to_col)
		- ExcelSheet::delNamedRange(string name)
		- ExcelSheet::setPrintRepeatRows(int rowFirst, int rowLast)
		- ExcelSheet::setPrintRepeatCols(int colFirst, int colLast)
		- ExcelSheet::getGroupSummaryBelow()
		- ExcelSheet::setGroupSummaryBelow(bool direction)
		- ExcelSheet::getGroupSummaryRight()
		- ExcelSheet::setGroupSummaryRight(bool direction)
		- ExcelSheet::clearPrintRepeats()
		- ExcelSheet::clearPrintArea()

## [0.8.6] - 2010-08-27
	* Added support for LibXL 3.0.0
	* Added support for generating Excel 2007/2010 (xlsx files)
		- the ExcelBook() method now has a constructor parameter that can be used to toggle xlsx mode
	* Added ExcelBook methods for xlsx color
		- colorUnpack, colorPack, setRGBMode, rgbMode
	* Added missing validation checks and fixed on-error memory leak

## [0.8.5] - 2010-08-11
	* Fixed build on 5.4 (patch from Felipe Pena)

## [0.8.2] - 2010-08-01
	* Fixed shared build

## [0.8.1] - 2010-08-01
	* Fixed build on 5.3 (patch from Felipe Pena)
	* Allow compilation against LibXL 2.4.3k
	* Added ExcelSheet::setProtect(), ExcelSheet::protect() to set/get sheet protection
	* Added ExcelSheet::setPrintHeaders() to set printability of column/row headers
	* Added ExcelSheet::setCellFormat() method (LibXL 2.4.3k+) to set a cell format
	* Added ExcelBook::getAllFormats() method (LibXL 2.4.3k+) to get a list of all document formats

## [0.8] - 2010-08-01
	* Initial Release

[Unreleased]: https://github.com/iliaal/php_excel/compare/2.0.1...HEAD
[2.0.1]: https://github.com/iliaal/php_excel/releases/tag/2.0.1
[2.0.0]: https://github.com/iliaal/php_excel/releases/tag/v2.0.0

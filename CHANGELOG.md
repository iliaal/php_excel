# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2.8.0] - 2026-09-03

### Docs
- Documented that `ExcelSheet::writeRow()`/`writeCol()` are non-atomic on libxl-side failure: PHP-side validation rejects the whole row or column before any cell is written, but a failure inside libxl can leave earlier cells committed.

### Fixed
- Stream-wrapper saves whose rename() is present but fails now also fail closed (destination left unchanged) instead of falling back to a truncating direct write. Only wrappers that omit rename() keep the warned fallback.
- ExcelBook::save() to a local path no longer falls back to a truncating direct write when replacing the destination with the staged file fails. It now removes the staged file, warns that the destination was left unchanged, and returns false. Stream wrappers that do not implement rename() keep the previous direct-write fallback with its warning.
- ExcelBook::load(), loadFile(), loadPartially(), loadFilePartially(), loadFileWithoutEmptyCells(), loadInfo(), and loadInfoRaw() no longer destroy the previously loaded workbook when the new load fails. The existing book and its live child wrappers survive a failed reload.
- ExcelBook::save() to a local path now checks the staging-stream close result before handing the path to LibXL instead of saving over a failed reservation.
- Failed handle creation (getSheet, sheet copies, fonts, formats, custom number formats, default font) and rejected header, footer, font-size, and staging inputs now emit warnings naming the cause instead of failing silently.
### Changed
- Stream-backed loads set a read timeout where the wrapper supports it, and bulk read and write loops now honor executor interruption. Wrappers without stream_set_option support are unaffected.

## [2.7.0] - 2026-09-01

### Breaking
- ExcelRichString::addText() now rejects a font that belongs to a different ExcelBook, matching every other cross-book handle check. Code that passed a foreign font to addText() in 2.5.0 or 2.6.0 has to add the font to the rich string's own book first. ExcelRichString::addFont() keeps accepting a foreign font as a copy template.

### Fixed
- ExcelBook::unpackDate() no longer rejects the valid timestamp -1 (the instant 1969-12-31T23:59:59 in the configured timezone), which mktime() returns as a legitimate value rather than only as an error code. Date cells holding that timestamp read back instead of failing.
- ExcelBook::loadInfoRaw() now rejects an empty data string up front, like ExcelBook::load(), instead of forwarding it to LibXL.

## [2.6.0] - 2026-07-27

### Breaking
- ExcelFormat::borderDiagonalStyle() now addresses the diagonal line style (BORDERSTYLE_*) instead of the diagonal direction it had been wired to. Code passing a BORDERDIAGONAL_* constant keeps running without error but stops drawing its diagonal, and has to move to ExcelFormat::borderDiagonal().

### Added
- ExcelFormat::borderDiagonal() gets and sets which diagonals of a cell are drawn, one of the BORDERDIAGONAL_* constants. Styles above BORDERDIAGONAL_BOTH reach borderDiagonalStyle() intact rather than being clamped away.

## [2.5.0] - 2026-07-27

### Security
- Stream-backed load, save, read, write, and picture operations now remain safe when user callbacks reconstruct the workbook during the operation.
- Stream callbacks can now unregister their wrapper or throw during read, write, flush, or close without leaving native code with a freed wrapper pointer or committing incomplete input.
- Releasing a child wrapper no longer exposes a live WeakReference to an object whose native storage is already being destroyed.
- Path operations now use PHP streams while open_basedir is active, closing the gap between a policy check and LibXL reopening a changed path. The LibXL APIs that require a pathname (ExcelBook::loadFileWithoutEmptyCells(), ExcelBook::addPictureAsLink(), and ExcelBook::loadInfo() with LibXL older than 5.0.1) reject calls while open_basedir is active.
- Stream saves now create unpredictable temporary files exclusively, so an existing file or symlink cannot redirect the staged write.
- ExcelBook::save() to a local path now carries the destination's existing permission bits onto the staged file, so replacing a restricted workbook no longer widens it to the process umask.

### Changed
- ExcelBook::save() now stages every write and renames it into place, including plain local paths. A save that fails while writing the staged file leaves the destination untouched. Consequences on local paths: a symlink destination is replaced by a regular file instead of being written through, an existing hard link is broken, and saving into a writable file inside a non-writable directory now fails instead of succeeding.
- ExcelSheet::write(), writeRow() and writeCol() now reject a value whose type cannot satisfy an explicitly requested data type, instead of silently storing a different kind (previously `AS_DATE` with a string stored text, and `AS_NUMERIC_STRING` with an int stored a plain number). `null` is still accepted for every data type and writes a blank cell, so bulk writes of a typed column with gaps keep working.
- Write rejections now name the reason, and writeRow()/writeCol() name the cell they stopped at.
- ExcelRichString::addText() once again accepts a font from another workbook. LibXL copies the font attributes into the rich string, so the source workbook can be released immediately after the call.
- Re-invoking a constructor on a child wrapper now throws instead of rebinding or replacing its native handle.
- ExcelSheet::addrToRowCol() now accepts only complete A1 references within the active XLS or XLSX limits; trailing text, row zero, and out-of-range addresses return false.
- ExcelBook::save() through a stream wrapper stages the workbook to a sibling temporary URL and renames it into place. A wrapper that does not implement rename() still saves: the staged replacement falls back to a direct write and warns that the save was not atomic. The same fallback now applies when a wrapper's rename() fails, so a failed rename no longer leaves the destination unchanged.

### Fixed
- ExcelBook::save() to a local path streams through LibXL again rather than materializing the whole workbook in memory first, so its peak memory no longer scales with the workbook and the archive no longer counts against `memory_limit`. Measured on a 3.3 MB workbook: 67.7 MB of peak RSS back down to 0.7 MB, with no change in save time.
- ExcelBook::save() now rejects flush and close failures, removes the staged file after write exceptions, and re-resolves wrapper metadata after every user callback.
- Stream-backed load, partial load, metadata load, and picture import now reject read and close failures instead of passing accumulated bytes to LibXL.
- ExcelBook::insertSheet() no longer invalidates wrappers for existing sheets; LibXL keeps those sheet handles stable across insertion.
- ExcelSheet::setAutoFitArea() and direct ExcelConditionalFormatting construction now reject inverted finite ranges.
- Direct ExcelTable construction now rejects inverted rows and columns before LibXL can corrupt the workbook.
- Public double parameters now reject NAN and infinity before calling LibXL.
- ExcelSheet::setHeader() and setFooter() now apply the 255-character limit by character when the book uses a UTF-8 locale.
- ExcelBook::colorUnpack() now accepts packed black (`0`) and returns its RGB components.
- ExcelBook::unpackDate() now accepts time-only Excel serials in `[0,1)` instead of returning false.
- ExcelSheet::readRow(), readCol(), readSparseRow() and readSparseCol() return an empty array on a sheet with no used range, instead of false with a misleading "Invalid ending column number '-1'" warning.
- ExcelBook::loadPartially() and loadFilePartially() now report the LibXL error message when a load fails.
- ExcelSheet::cellFormat() rejects a null format handle instead of wrapping it.
- Strict-date cells (`CELLTYPE_STRICTDATE`) are now recognized by ExcelSheet::read() and isDate().
- ExcelSheet::read() clears its by-reference `$format` argument again when the cell read fails, rather than leaving the previous cell's format in place.
- The API reference now carries an `@since` marker on every version-gated method.
- Sparse row and column reads with an omitted end now start at LibXL's first filled row or column, avoiding full-sheet scans for data near the XLSX limits.
- ExcelAutoFilter::getSort() now builds with both the transient LibXL 5.2.0 sort-level API and the restored 5.2.0.1 signature.
- The API reference now matches every public runtime signature, including parameter names, types, defaults, and return types.
- The API reference now matches all runtime constant values and documents the LibXL 4.6.0 through 5.0.x conditional-formatting signatures.
- Added the PHP License 3.01 file referenced by the source headers and package documentation.

### For contributors
- CI now rejects all-skipped PHPT runs and verifies that the extension loaded before running the suite.
- GitHub Actions workflows are pinned and permission-scoped, with a zizmor security lane.
- The PHPTs that exercise LibXL 5.x-only APIs now skip cleanly against LibXL 4.6.0, so the minimum-version suite runs green.
- The main ASan suite now runs without LibXL leak suppressions. Only the two tests that exercise a confirmed LibXL leak use the vendor suppression.
- Configure now keeps automatic header and library discovery under one prefix and rejects a library that lacks symbols required by the selected LibXL headers.
- CONTRIBUTING.md now states the supported PHP 8.1 minimum.

## [2.4.0] - 2026-07-09

### Security
- excel.license_key and excel.license_name are now PHP_INI_SYSTEM, so a per-request ini_set() or .user.ini can no longer overwrite the commercial license in a shared pool (a runtime key can still be passed to the ExcelBook constructor).

### Added
- ExcelSheet::getIndexRange() now includes the range's "name" in the returned array (libxl exposes it; it was previously discarded).

### Changed
- ExcelBook::load(), loadFile(), and save() now emit an E_WARNING carrying the underlying libxl error message on failure, matching ExcelSheet::write(); they previously returned a bare false with no diagnostic.
- SECURITY.md and the README now document writing untrusted values with ExcelFormat::AS_TEXT to prevent spreadsheet formula injection (the implicit leading-'=' formula promotion is unchanged).
- Documented the previously-undocumented ExcelSheet::removeComment() method and the load()/loadFile() contract that a load invalidates any previously-fetched child wrappers (they must be re-fetched), success or failure.

### Fixed
- ExcelFont::size() now rejects a non-positive size (E_WARNING + false) instead of silently ignoring it and returning the current size as if it were a getter. Getter mode (no argument or null) and valid sizes are unchanged.
- ExcelBook::save() to a stream path (e.g. file://) no longer truncates and destroys the caller's existing file on a short or interrupted write. Native wrappers without rename support use a non-atomic direct write; PHP user-defined wrappers must implement rename() and unlink() because PHP's adapter advertises both operations.
- load(), loadFile(), loadInfoRaw(), and addPictureFromString()/addPictureFromFile() now reject buffers larger than UINT_MAX instead of silently truncating a >4 GiB payload at libxl's unsigned size parameter.
- ExcelSheet::writeRow()/writeCol() now reject an unwritable value (bad type, embedded NUL, unpackable AS_DATE) before writing any cell, instead of committing earlier cells and then failing mid-array.
- Inverted ranges (first > last) are now rejected across the range methods (hyperlinks, autofilter setRef, conditional formatting, data validation, clear, insert/remove row/col) instead of being stored as garbage coordinates; equal endpoints stay valid.
- ExcelSheet::write() with ExcelFormat::AS_DATE now fails (returns false) when no date format can be allocated (the xlsx style table is exhausted), instead of writing a bare number and reporting success.
- ExcelSheet::autoFilter() and ExcelAutoFilter::column()/columnByIndex() now return false when libxl yields no handle, matching their documented `|false` return, instead of a usable-looking object that only failed on the next method call.
- ExcelRichString::addText() now rejects a font that belongs to a different ExcelBook, closing the last cross-book handle gap (the other use-sites were already guarded).
- ExcelSheet::write() with ExcelFormat::AS_DATE now packs float timestamps as dates; previously only integer timestamps were treated as dates.
- ExcelBook::addPictureFromFile() now fails closed with a single warning when a plain path is denied by open_basedir, instead of emitting a duplicate warning and falling through to the stream wrapper.
- Re-invoking a child object's __construct() (e.g. ExcelFont, ExcelFilterColumn) no longer leaks a reference to the previously bound parent.
- docs/ API reference signatures now match the implementation: ExcelAutoFilter::setRef() argument order, the removed nonexistent third argument on ExcelSheet::insertRow()/insertCol()/removeRow()/removeCol(), and the corrected bool/int/required parameter types on the ExcelConditionalFormatting rule methods.
- ExcelSheet::write()/writeRow()/writeCol() now reject a non-finite (NAN/INF) number, and an AS_DATE double beyond zend_long range, instead of storing a corrupt cell or casting out of range.
- A reference cycle through a child wrapper's hidden parent (e.g. $book->prop = $sheet) is now reclaimable by the cycle collector; get_gc handlers expose the parent that previously leaked such cycles until request shutdown.
- Zero-arg methods across all classes now throw ArgumentCountError on surplus arguments instead of triggering a debug-build arginfo/zpp mismatch fatal (the earlier pass missed the methods whose arginfo is a shared alias).
- ExcelFormat::fillPattern()'s named argument is now $pattern (was misspelled $patern in the generated arginfo), so pattern: named calls resolve.
- ExcelBook::loadInfo() and loadInfoRaw() now emit the libxl error message on failure, matching load()/loadFile().
- ExcelBook::save()'s stub return type is now string|bool (was the invalid string|true|false union that broke stub linting and regeneration); the runtime signature is unchanged.

### Hardening
- ExcelBook::loadFile(), loadFilePartially(), and addPictureFromFile() reject a seekable stream whose stat size is >= UINT_MAX up front, and otherwise cap the read at UINT_MAX, so an oversized source cannot allocate ~4 GiB before rejection.
- Module globals now install the ZTS TLS cache (ZEND_TSRMLS_CACHE_*), so EXCEL_G access on a threaded build no longer does a per-access thread-local lookup.
- CI build and test steps now fail on a non-zero make/run-tests exit (previously a compile failure with no 'warning:' output, or a runner crash before the summary line, could pass green).

## [2.3.0] - 2026-07-03

### Added
- ExcelSheet::readRange() reads a rectangular block of cells into a 2D array in one call, avoiding per-cell PHP dispatch on read-heavy import paths.
- ExcelSheet::readSparseRow()/readSparseCol() return only the occupied cells of a row or column, keyed by their original column or row index.
- ExcelBook::loadPartially()/loadFilePartially() load a single sheet's row slice from an Excel string or file, with an optional flag to keep the remaining sheets (requires libxl 5.0.0+).
- ExcelBook::loadFileWithoutEmptyCells() loads an xls file while dropping empty cells (xls format only; requires libxl 5.0.0+).
- ExcelSheet::writeRow() now accepts an optional $data_type argument, matching writeCol(), to apply an explicit cell data type across the row.

### Fixed
- ExcelSheet::read() now sets the format out-param for error-type cells; it previously returned an uninitialized ExcelFormat that warned on use.
- ExcelBook::packDate() now accepts the Unix epoch; timestamp 0 was wrongly rejected as invalid.
- ExcelBook::loadFile()/save() no longer emit a duplicate open_basedir warning when a plain path is denied.
- Child wrappers (ExcelAutoFilter, ExcelFilterColumn, ExcelConditionalFormatting) fetched before their autofilter, table filter, or conditional formatting is removed are now invalidated and reject reuse, while the owning sheet, table, or book stays valid.

## [2.2.0] - 2026-06-11

### Added
- PHP 8.1 support (lowered the minimum from 8.3).
- ExcelFormat::AS_TEXT data type for ExcelSheet::write()/writeCol(): writes
  the string verbatim with no leading-quote stripping, no implicit formula
  promotion, and no numeric coercion. Use it when writing untrusted input to
  prevent formula injection.

### Changed
- The implicit "leading '=' becomes a formula" promotion now applies only
  when no data type argument is passed; an explicitly passed type (e.g.
  AS_NUMERIC_STRING) is honored instead of being overridden.

### Fixed
- excel.skip_empty was backed by an int global while OnUpdateLong stores a
  zend_long; the setting read as 0 on big-endian 64-bit. The field is now
  zend_long.
- ExcelAutoFilter::getRef() returned rowLast as col_first and colFirst as
  row_last; the libxl call passed the output pointers in the wrong order.
- ExcelAutoFilter::setRef() declared its parameters as (row_first, col_first,
  row_last, col_last) while the implementation reads (row_first, row_last,
  col_first, col_last); named-argument callers got rows and columns crossed.
  The signature now matches the implementation.
- Methods on wrappers derived from a subclassed Excel class (e.g. class
  MyBook extends ExcelBook) failed with a spurious "handle is stale" warning;
  the book resolver now walks the parent chain with instanceof checks.
- ExcelSheet::printArea() returned rowLast as col_start and colFirst as
  row_end; the libxl call passed the output pointers in the wrong order.
- ExcelSheet::setNamedRange() declared its parameters as (row, col, to_row,
  to_col) while the implementation reads (row_first, row_last, col_first,
  col_last); the signature now matches, fixing named-argument callers.
- ExcelFilterColumn::setTop10() required all three arguments despite the
  declared defaults; $top and $percent are now optional ($top defaults true).
- ExcelFilterColumn::setCustomFilter() now accepts null for $value_2 and
  declares the -1/null no-second-criterion defaults; passing the previously
  documented defaults (0, "") warned and failed.
- ExcelFont::name() crashed on a null name from libxl; it now returns false.
- Declared defaults corrected to match behavior: addTable() $hasHeaders
  (true), rowColToAddr() relatives (true), named-range $scope_id (-1,
  workbook scope), setProtect() $enhancedProtection (-1, libxl default),
  setAutoFitArea() ends (-1, no limit).
- Return types corrected for the stale-book error path:
  dataValidationSize() is int|false, the nine ExcelCoreProperties string
  getters are string|null|false.
- The module now unregisters its INI entries at shutdown; previously they
  stayed registered after a dl-unload and dangled.

## [2.1.0] - 2026-06-01

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
  methods (`ExcelBook::addFormat`/`addFont`) still accept a
  handle from another book by design (`ExcelBook::insertSheet` requires the same book).
- Reject out-of-range values at every public libxl integer/enum/color boundary
  instead of silently truncating the 64-bit argument to `int`. This covers the
  full surface, not only `set*` methods:
  - Setters: `ExcelSheet` (`setZoom`, `setZoomPrint`, `setPaper`, `setPrintFit`,
    `setBorder`, `setColPx`/`setRowPx` pixel sizes, `setTabColor`,
    `setRightToLeft`, `writeComment` width/height, `setProtect`
    enhanced-protection bitmask), `ExcelBook` (`setCalcMode`, `setDefaultFont`
    size), `ExcelConditionalFormat` (all `setBorder*`, `setNumFormat`,
    `setFillPattern`, `setPattern*Color`), `ExcelFormControl` (`setChecked`,
    `setDropLines`, `setDx`, `setFirstButton`, `setHoriz`, `setInc`, `setMax`,
    `setMin`, `setSel`), `ExcelTable::setStyle`, and
    `ExcelFilterColumn::setCustomFilter` operators.
  - Other `ExcelBook` int boundaries: `colorUnpack` (colour index — now also
    upper-bounded), `addFormatFromStyle` (builtin-style id), `packDateValues`
    (year upper bound; month/day/time were already checked), and
    `getCustomFormat` (id — had a `< 1` check but no upper bound).
  - Named-range scope: `ExcelSheet::setNamedRange`, `delNamedRange`, and
    `getNamedRange` passed `scope_id` to libxl unchecked, so a 64-bit value
    could alias the `SCOPE_WORKBOOK (-1)` sentinel or a different sheet after
    narrowing. Now bounded to `[SCOPE_UNDEFINED (-2), INT_MAX]`, preserving both
    documented scope sentinels.
  - Add/rule/constructor APIs: `ExcelSheet::addDataValidation`/
    `addDataValidationDouble` (type/op/error-style), `addIgnoredError`,
    `addTable` (style), the `ExcelConditionalFormatting` rule methods
    (`addRule`, `addTopRule`, `addOpNumRule`, `addOpStrRule`,
    `addAboveAverageRule`, `addTimePeriodRule`, and the 2-/3-colour-scale
    rules' colour and type enums), and `ExcelTable::__construct` (style).
  - `ExcelSheet::setTabRgbColor` requires each component in 0-255.
  Documented `-1` sentinels (`PROT_DEFAULT`, `setCustomFilter`'s absent second
  operator, named-range scope) are preserved. `ExcelFormat`/`ExcelFont` setters
  were already range-checked.

### Changed
- Tightened the published parameter types for several `ExcelSheet` setters that
  were declared `mixed` but parse a concrete scalar: `setZoom`/`setZoomPrint`
  (`int`), `setPrintGridlines`/`setLandscape`/`setHCenter`/`setVCenter`
  (`bool`), `setMarginLeft`/`setMarginRight`/`setMarginTop`/`setMarginBottom`
  (`float`), and `setHeader`/`setFooter` (`string`, `float`). Reflection and
  static analysis now report the real signatures.

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

[Unreleased]: https://github.com/iliaal/php_excel/compare/2.8.0...HEAD
[2.7.0]: https://github.com/iliaal/php_excel/releases/tag/2.7.0
[2.8.0]: https://github.com/iliaal/php_excel/releases/tag/2.8.0
[2.6.0]: https://github.com/iliaal/php_excel/releases/tag/2.6.0
[2.5.0]: https://github.com/iliaal/php_excel/releases/tag/2.5.0
[2.4.0]: https://github.com/iliaal/php_excel/releases/tag/2.4.0
[2.3.0]: https://github.com/iliaal/php_excel/releases/tag/2.3.0
[2.2.0]: https://github.com/iliaal/php_excel/releases/tag/2.2.0
[2.1.0]: https://github.com/iliaal/php_excel/releases/tag/2.1.0
[2.0.1]: https://github.com/iliaal/php_excel/releases/tag/2.0.1
[2.0.0]: https://github.com/iliaal/php_excel/releases/tag/v2.0.0

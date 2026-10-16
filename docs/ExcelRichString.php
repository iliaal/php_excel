<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelRichString                                                           |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelRichString
{
	/**
	* Create a rich string within an Excel workbook
	*
	* @see ExcelBook::addRichString()
	* @param ExcelBook $book
	*/
	public function __construct(ExcelBook $book)
	{
	}

	/**
	* Adds a font to the rich string. Returns a new font for use with addText().
	*
	* The optional font is a copy template and may belong to another workbook.
	* The returned font belongs to this rich string's workbook; changing the
	* copy does not change the template. Pass the returned font to addText(),
	* rather than passing a foreign-workbook template directly.
	*
	* @see ExcelRichString::addText()
	* @param ExcelFont|null $font (optional) Font to copy from, including another workbook
	* @return ExcelFont|false
	*/
	public function addFont(?ExcelFont $font = null): ExcelFont|false
	{
	}

	/**
	* Adds text to the rich string with an optional font
	*
	* A supplied font must belong to the same workbook as this rich string.
	* A font from another workbook emits a warning and returns false without
	* adding a text segment. Use addFont($font) to copy a foreign font first.
	* Omit the font or pass null for a segment without an explicit font.
	*
	* Text must be valid UTF-8 unless ExcelBook::setLocale() selected a
	* non-UTF-8 locale, in which case it must be valid in that locale's
	* encoding. Text that cannot be decoded emits a warning and returns false
	* without adding a text segment.
	*
	* @see ExcelRichString::addFont()
	* @param string $text
	* @param ExcelFont|null $font (optional) Same-workbook font, or null
	* @return bool
	*/
	public function addText(string $text, ?ExcelFont $font = null): bool
	{
	}

	/**
	* Returns the text and font at the specified index
	*
	* If LibXL returns no font handle, the font entry is null. Check for null
	* before calling ExcelFont methods on the returned value.
	*
	* @param int $index Zero-based text segment index
	* @return array|false Array with keys "text" (string) and "font" (ExcelFont|null), or false on failure
	*/
	public function getText(int $index): array|false
	{
	}

	/**
	* Returns the number of text segments in the rich string
	*
	* @return int|false
	*/
	public function textSize(): int|false
	{
	}

} // end ExcelRichString

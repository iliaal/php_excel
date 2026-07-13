<?php
/*
  +---------------------------------------------------------------------------+
  | ExcelCoreProperties                                                       |
  |                                                                           |
  | Reference file for NuSphere PHPEd (and possibly other IDE's) for use with |
  | php_excel interface to libxl by Ilia Alshanetsky <ilia@ilia.ws>           |
  |                                                                           |
  | php_excel "PECL" style module (http://github.com/iliaal/php_excel)        |
  | libxl library (http://www.libxl.com)                                      |
  |                                                                           |
  +---------------------------------------------------------------------------+
*/
class ExcelCoreProperties
{
	/**
	* Create a core properties object for a workbook
	*
	* @see ExcelBook::coreProperties()
	* @param ExcelBook $book
	*/
	public function __construct(ExcelBook $book)
	{
	}

	/**
	* Returns the title
	*
	* @return string|null|false
	*/
	public function title(): string|false|null
	{
	}

	/**
	* Sets the title
	*
	* @param string $value
	* @return bool
	*/
	public function setTitle(string $value): bool
	{
	}

	/**
	* Returns the subject
	*
	* @return string|null|false
	*/
	public function subject(): string|false|null
	{
	}

	/**
	* Sets the subject
	*
	* @param string $value
	* @return bool
	*/
	public function setSubject(string $value): bool
	{
	}

	/**
	* Returns the creator
	*
	* @return string|null|false
	*/
	public function creator(): string|false|null
	{
	}

	/**
	* Sets the creator
	*
	* @param string $value
	* @return bool
	*/
	public function setCreator(string $value): bool
	{
	}

	/**
	* Returns the last modified by value
	*
	* @return string|null|false
	*/
	public function lastModifiedBy(): string|false|null
	{
	}

	/**
	* Sets the last modified by value
	*
	* @param string $value
	* @return bool
	*/
	public function setLastModifiedBy(string $value): bool
	{
	}

	/**
	* Returns the created date as a string
	*
	* @return string|null|false
	*/
	public function created(): string|false|null
	{
	}

	/**
	* Sets the created date as a string
	*
	* @param string $value
	* @return bool
	*/
	public function setCreated(string $value): bool
	{
	}

	/**
	* Returns the modified date as a string
	*
	* @return string|null|false
	*/
	public function modified(): string|false|null
	{
	}

	/**
	* Sets the modified date as a string
	*
	* @param string $value
	* @return bool
	*/
	public function setModified(string $value): bool
	{
	}

	/**
	* Returns the tags
	*
	* @return string|null|false
	*/
	public function tags(): string|false|null
	{
	}

	/**
	* Sets the tags
	*
	* @param string $value
	* @return bool
	*/
	public function setTags(string $value): bool
	{
	}

	/**
	* Returns the categories
	*
	* @return string|null|false
	*/
	public function categories(): string|false|null
	{
	}

	/**
	* Sets the categories
	*
	* @param string $value
	* @return bool
	*/
	public function setCategories(string $value): bool
	{
	}

	/**
	* Returns the comments
	*
	* @return string|null|false
	*/
	public function comments(): string|false|null
	{
	}

	/**
	* Sets the comments
	*
	* @param string $value
	* @return bool
	*/
	public function setComments(string $value): bool
	{
	}

	/**
	* Returns the created date as a double (Excel timestamp)
	*
	* @return float
	*/
	public function createdAsDouble(): mixed
	{
	}

	/**
	* Sets the created date as a double (Excel timestamp)
	*
	* @param float $value
	* @return bool
	*/
	public function setCreatedAsDouble(float $value): bool
	{
	}

	/**
	* Returns the modified date as a double (Excel timestamp)
	*
	* @return float
	*/
	public function modifiedAsDouble(): mixed
	{
	}

	/**
	* Sets the modified date as a double (Excel timestamp)
	*
	* @param float $value
	* @return bool
	*/
	public function setModifiedAsDouble(float $value): bool
	{
	}

	/**
	* Removes all core properties
	*
	* @return void
	*/
	public function removeAll(): mixed
	{
	}

} // end ExcelCoreProperties

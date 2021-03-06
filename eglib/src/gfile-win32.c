/*
 * File utility functions.
 *
 * Author:
 *   Gonzalo Paniagua Javier (gonzalo@novell.com)
 *
 * (C) 2006 Novell, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <config.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef G_OS_WIN32
#include <io.h>
#define open _open
#define S_ISREG(x) ((x &  _S_IFMT) == _S_IFREG)
#define S_ISDIR(x) ((x &  _S_IFMT) == _S_IFDIR)
#endif

int mkstemp (char *tmp_template)
{
	int fd;
	gunichar2* utf16_template;

	utf16_template  = u8to16 (tmp_template);

	fd = -1;
	utf16_template = _wmktemp( utf16_template);
	if (utf16_template && *utf16_template) {
		/* FIXME: _O_TEMPORARY causes file to disappear on close causing a test to fail */
		//lulu add _O_RDWR for windows. otherwise will call err ERROR_ACCESS_DENIED when write to file.
		//fd = _wopen( utf16_template, _O_BINARY | _O_CREAT /*| _O_TEMPORARY*/ | _O_EXCL, _S_IREAD | _S_IWRITE);
		fd = _wopen( utf16_template, _O_BINARY | _O_CREAT /*| _O_TEMPORARY*/ | _O_EXCL | _O_RDWR, _S_IREAD | _S_IWRITE);
	}

	sprintf (tmp_template + strlen (tmp_template) - 6, "%S", utf16_template + wcslen (utf16_template) - 6);

	g_free (utf16_template);
	return fd;
}

#ifdef _MSC_VER
#pragma warning(disable:4701)
#endif

gboolean
g_file_test (const gchar *filename, GFileTest test)
{
	struct __stat64 stat;
	int ret = 0;
	gunichar2* utf16_filename = NULL;

	if (filename == NULL || test == 0)
		return FALSE;

	utf16_filename = u8to16 (filename);
	ret = _wstati64 (utf16_filename, &stat);
	g_free (utf16_filename);

	if ((test & G_FILE_TEST_EXISTS) != 0) {
		if (ret == 0)
			return TRUE;
	}

	if (ret != 0)
		return FALSE;

	if ((test & G_FILE_TEST_IS_EXECUTABLE) != 0) {
		if (stat.st_mode & _S_IEXEC)
			return TRUE;
	}

	if ((test & G_FILE_TEST_IS_REGULAR) != 0) {
		if (stat.st_mode & _S_IFREG)
			return TRUE;
	}

	if ((test & G_FILE_TEST_IS_DIR) != 0) {
		if (stat.st_mode & _S_IFDIR)
			return TRUE;
	}

	/* make this last in case it is OR'd with something else */
	if ((test & G_FILE_TEST_IS_SYMLINK) != 0) {
		return FALSE;
	}

	return FALSE;
}

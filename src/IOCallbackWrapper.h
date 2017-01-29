/*
 *  Copyright (C) Jory Stone - June 2003
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 */

#ifndef IO_CALLBACK_WRAPPER_H
#define IO_CALLBACK_WRAPPER_H

#include "ebml/IOCallback.h"
#include "ebml/StdIOCallback.h"
#include "ebml/EbmlTypes.h"

// WinIOCallback (support for 4GB+ files)
#ifdef WIN32
#include "WinIOCallback.h"
#endif // WIN32


namespace mkvwriter {


class FileIOCallback : public libebml::IOCallback
{
	public:
	FileIOCallback(const char *Path, const open_mode Mode) {		
#ifdef WIN32
		m_fileHandle = new WinIOCallback(Path, Mode);
#else // Under Linux?
		m_fileHandle = new StdIOCallback(Path, Mode);
#endif
	};

#ifdef WIN32
	FileIOCallback(const wchar_t *Path, const open_mode Mode) {		
#ifdef _UNICODE		
		m_fileHandle = new WinIOCallback(Path, Mode);
#else
		//I have to convert the file name to ANSI
		m_fileHandle = new WinIOCallback(Path, Mode);
#endif // _UNICODE
	};
#endif		
	~FileIOCallback() {
		delete m_fileHandle; 
	};

	uint32 read(void *Buffer, size_t Size) {
		return m_fileHandle->read(Buffer, Size);
	};
	size_t write(const void *Buffer, size_t Size) {
		return m_fileHandle->write(Buffer, Size);
	};
	void setFilePointer(int64 Offset, seek_mode Mode=seek_beginning) {
		m_fileHandle->setFilePointer(Offset, Mode);
	};
	uint64 getFilePointer() {
		return m_fileHandle->getFilePointer();
	};
	void close() {
		m_fileHandle->close();
	};

	protected:
	IOCallback *m_fileHandle;
};


} // namespace mkvwriter


#endif // IO_CALLBACK_WRAPPER_H

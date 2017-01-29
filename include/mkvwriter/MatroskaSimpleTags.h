/*
 *  General Matroska Muxer Interface
 *
 *  Copyright (C) 2003-2004, Jory Stone <jcsston @ toughguy.net>
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

/*!
	\version $Id: MatroskaSimpleTags.h,v 1.1 2004/03/24 05:46:51 jcsston Exp $
	\author Jory Stone           <jcsston @ toughguy.net>
 	\brief

 	\par CVS Log
  	$Log: MatroskaSimpleTags.h,v $
  	Revision 1.1  2004/03/24 05:46:51  jcsston
  	Initial Commit

  	Revision 1.1  2004/01/04 01:07:31  jcsston
  	Finally some matroska muxing code
  	

*/


#ifndef _MATROSKA_SIMPLE_TAGS_H_
#define _MATROSKA_SIMPLE_TAGS_H_


#include <vector>

#include "ebml/EbmlTypes.h"
#include "ebml/EbmlString.h"
#include "ebml/EbmlUnicodeString.h"


namespace mkvwriter {


class MatroskaSimpleTag {
public:
	MatroskaSimpleTag();

	libebml::UTFstring name;
	libebml::UTFstring value;
};

class MatroskaTagInfo {
public:
	MatroskaTagInfo();
	void SetTagValue(const char *name, const char *value);

	uint64 targetTrackUID;
	uint64 targetChapterUID;
	uint64 targetAttachmentUID;

	std::vector<MatroskaSimpleTag> tags;
};

class MatroskaTagVector : public std::vector<MatroskaTagInfo>
{
public:
	MatroskaTagInfo *FindTagWithTrackUID(uint64 trackUID);
	MatroskaTagInfo *FindTagWithChapterUID(uint64 chapterUID, uint64 trackUID = 0);
};


} // namespace mkvwriter


#endif // _MATROSKA_SIMPLE_TAGS_H_


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
	\version $Id: MatroskaSimpleTags.cpp,v 1.1 2004/03/24 05:46:51 jcsston Exp $
	\author Jory Stone           <jcsston @ toughguy.net>
 	\brief

 	\par CVS Log
  	$Log: MatroskaSimpleTags.cpp,v $
  	Revision 1.1  2004/03/24 05:46:51  jcsston
  	Initial Commit

  	Revision 1.1  2004/01/04 01:07:31  jcsston
  	Finally some matroska muxing code
  	

*/

#include "mkvwriter/MatroskaSimpleTags.h"


using namespace LIBEBML_NAMESPACE;


namespace mkvwriter {


MatroskaSimpleTag::MatroskaSimpleTag()
{
	name = L"";
	value = L"";
};

MatroskaTagInfo::MatroskaTagInfo()
{
	targetTrackUID = 0;
	targetChapterUID = 0;
	targetAttachmentUID = 0;
};

void MatroskaTagInfo::SetTagValue(const char *name, const char *value)
{
	for (size_t s = 0; s < tags.size(); s++) {
		MatroskaSimpleTag &currentSimpleTag = tags.at(s);
		if (!strcasecmp(currentSimpleTag.name.GetUTF8().c_str(), name)) {
			currentSimpleTag.value.SetUTF8(value);
			return;
		}
	}
	// If we are here then we didn't find this tag in the vector already
	MatroskaSimpleTag newSimpleTag;
	newSimpleTag.name.SetUTF8(name);
	newSimpleTag.value.SetUTF8(value);
	tags.push_back(newSimpleTag);
};

MatroskaTagInfo *MatroskaTagVector::FindTagWithTrackUID(uint64 trackUID) 
{
	MatroskaTagInfo *foundTag = NULL;

	for (size_t t = 0; t < size(); t++) {
		MatroskaTagInfo &currentTag = at(t);	
		if (currentTag.targetTrackUID == trackUID) {
			foundTag = &currentTag;
			break;
		}
	}

	return foundTag;
}

MatroskaTagInfo *MatroskaTagVector::FindTagWithChapterUID(uint64 chapterUID, uint64 trackUID)
{
	MatroskaTagInfo *foundTag = NULL;

	for (size_t t = 0; t < size(); t++) {
		MatroskaTagInfo &currentTag = at(t);	
		if (currentTag.targetChapterUID == chapterUID) {
			if (trackUID != 0) {
				if (currentTag.targetTrackUID != trackUID)
					continue;
			}
			foundTag = &currentTag;
			break;
		}
	}

	return foundTag;
};


} // namespace mkvwriter

/*
 *  General Matroska Muxer Interface
 *
 *  Copyright (C) 2003-2004, Jory Stone <jcsston @ toughguy.net>
 *  Copyright (C) 2003, John Cannon <spyder482 @ yahoo.com>
 *  Copyright (C) 2002, Steve Lhomme <steve.lhomme @ free.fr>
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
	\version $Id: MatroskaMuxer.cpp,v 1.1 2004/03/24 05:46:51 jcsston Exp $
	\author Jory Stone           <jcsston @ toughguy.net>
 	\brief

 	\par CVS Log
  	$Log: MatroskaMuxer.cpp,v $
  	Revision 1.1  2004/03/24 05:46:51  jcsston
  	Initial Commit

  	Revision 1.2  2004/01/04 04:16:01  jcsston
  	It actual works!
  	
  	Revision 1.1  2004/01/04 01:07:31  jcsston
  	Finally some matroska muxing code
  	

*/

#include "mkvwriter/MatroskaMuxer.h"
#include "IOCallbackWrapper.h"

using namespace LIBEBML_NAMESPACE;
using namespace LIBMATROSKA_NAMESPACE;


namespace mkvwriter {


MatroskaMuxer::MatroskaMuxer()
{
	m_outputFile = NULL;
	framesMuxed = 0;
	clustersMuxed = 0;
	m_LaceCount = 0;

	currentClusterBaseTimecode = 0;
	currentClusterSize = 0;

	MySeekHead = &GetChild<KaxSeekHead>(FileSegment);
	MyDummy = &GetChild<EbmlVoid>(FileSegment);
	// Setup the track element
	m_Tracks = &GetChild<KaxTracks>(FileSegment);
	MySeekHeadOverFill = &GetChild<KaxSeekHead>(FileSegment);

	m_UseLacing = true;
	m_WriteDuration = false;
	clust = NULL;	
	m_MyKaxBlockGroup = NULL;
	m_AllCues = NULL;
	m_prevTimecode = 0xFFFFFFFFFFFFFFFF;
	m_prevTrack = 0xFFFF;
	m_lastTimecode = 0;
	m_timecodeScale = DefaultTimecodeScale;
	
	Set_ClusterLimits();

	Set_WritingApp(L"C++ Interface");
};

MatroskaMuxer::~MatroskaMuxer() {
    for (MatroskaAttachmentVector::iterator a = m_Attachments.begin(); a != m_Attachments.end(); ++a)
    {
        delete *a;
    }
    m_Attachments.clear();

    if (m_outputFile != NULL)
        delete m_outputFile;
};

int MatroskaMuxer::Set_OutputFilename(const std::string &ouputFilename)
{
	if (m_outputFile != NULL)
		delete m_outputFile;

	m_outputFile = new FileIOCallback(ouputFilename.c_str(), MODE_WRITE);
	
	if (m_outputFile == NULL)
		return -1;

	return 0;
}

int MatroskaMuxer::Set_WritingApp(const UTFstring &writingApp)
{
	m_WritingApp.SetUTF8(writingApp.GetUTF8() + " - MatroskaMuxer 1.0");
	return 0;
}

int MatroskaMuxer::Set_Lacing(bool bUseLacing) 
{
	m_UseLacing = bUseLacing;
	return 0;
};

int MatroskaMuxer::Set_WriteDuration(bool bWriteDuration)
{
	m_WriteDuration = bWriteDuration;
	return 0;
};

int MatroskaMuxer::Set_ClusterLimits(uint32 maxKBSize, uint32 maxMSLength)
{
	m_maxBytesPerCluster = maxKBSize * 1024;
	m_maxTimePerCluster = maxMSLength;
	return 0;
};

int MatroskaMuxer::Set_TimecodeScale(uint64 timecodeScale)
{
	m_timecodeScale = timecodeScale;
	
	unsigned int Index1;
	for (Index1 = 0; Index1 < m_Tracks->ListSize(); Index1++)
		((KaxTrackEntry *)(*m_Tracks)[Index1])->SetGlobalTimecodeScale(m_timecodeScale);

	Set_ClusterLimits();
	return 0;
};

int MatroskaMuxer::Set_Track_Type(uint8 trackNo, int trackType)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;
	
	*(static_cast<EbmlUInteger *>(&GetChild<KaxTrackType>(*track))) = trackType;

	if (m_UseLacing && (trackType == track_audio))
		Set_Track_Lacing(trackNo, true);
	else
		Set_Track_Lacing(trackNo, false);

	return 0;
};

int MatroskaMuxer::Set_Track_CodecID(uint8 trackNo, const std::string &codecID)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;
	
	*(static_cast<EbmlString *>(&GetChild<KaxCodecID>(*track))) = codecID;

	return 0;
};

int MatroskaMuxer::Set_Track_CodecPrivate(uint8 trackNo, const binary *codecPrivate, uint32 codecPrivateSize)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	KaxCodecPrivate &codecP = GetChild<KaxCodecPrivate>(*track);
	codecP.CopyBuffer(codecPrivate, codecPrivateSize);

	return 0;
};

int MatroskaMuxer::Set_Track_DefaultDuration(uint8 trackNo, uint64 defaultDuration)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	*(static_cast<EbmlUInteger *>(&GetChild<KaxTrackDefaultDuration>(*track))) = defaultDuration;

	return 0;
};

int MatroskaMuxer::Set_Track_Language(uint8 trackNo, const std::string &lang)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	*(static_cast<EbmlString *>(&GetChild<KaxTrackLanguage>(*track))) = lang;

	return 0;
};

int MatroskaMuxer::Set_Track_Video(uint8 trackNo, uint32 width, uint32 height, uint32 displayWidth, uint32 displayHeight)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	Set_Track_Type(trackNo, track_video);

	KaxTrackVideo & MyTrackVideo = GetChild<KaxTrackVideo>(*track);

	if (width != 0) {
		KaxVideoPixelWidth & MyTrackVideoPixelWidth = GetChild<KaxVideoPixelWidth>(MyTrackVideo);
		*static_cast<EbmlUInteger *>(&MyTrackVideoPixelWidth) = width;
	}
	if (height != 0) {
		KaxVideoPixelHeight & MyTrackVideoPixelHeight = GetChild<KaxVideoPixelHeight>(MyTrackVideo);
		*static_cast<EbmlUInteger *>(&MyTrackVideoPixelHeight) = height;
	}

	// Get ready to write the display sizes
	KaxVideoDisplayWidth & MyTrackVideoDisplayWidth = GetChild<KaxVideoDisplayWidth>(MyTrackVideo);
	KaxVideoDisplayHeight & MyTrackVideoDisplayHeight = GetChild<KaxVideoDisplayHeight>(MyTrackVideo);

	// Use the PixelWidth if the DisplayWidth is not set
	if (displayWidth != 0) {
		*static_cast<EbmlUInteger *>(&MyTrackVideoDisplayWidth) = displayWidth;
	} else {
		*static_cast<EbmlUInteger *>(&MyTrackVideoDisplayWidth) = width;
	}
	// Do the same here, except for the Height
	if (displayHeight != 0) {
		*static_cast<EbmlUInteger *>(&MyTrackVideoDisplayHeight) = displayHeight;
	} else {
		*static_cast<EbmlUInteger *>(&MyTrackVideoDisplayHeight) = height;
	}
	
	return 0;
};

int MatroskaMuxer::Set_Track_Audio(uint8 trackNo, uint8 channels, double sampleRate, uint16 bitdepth, double outputSampleRate)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	Set_Track_Type(trackNo, track_audio);

	KaxTrackAudio &MyTrackAudio = GetChild<KaxTrackAudio>(*track);

	if (channels != 0) {
		KaxAudioChannels & MyTrackAudioChannels = GetChild<KaxAudioChannels>(MyTrackAudio);
		*(static_cast<EbmlUInteger *>(&MyTrackAudioChannels)) = channels;
	}

	if (sampleRate != 0) {
		KaxAudioSamplingFreq & MyTrackFreq = GetChild<KaxAudioSamplingFreq>(MyTrackAudio);
		*(static_cast<EbmlFloat *>(&MyTrackFreq)) = sampleRate;

		MyTrackFreq.ValidateSize();
	}

	
	if (outputSampleRate != 0) {
		KaxAudioOutputSamplingFreq & MyTrackAudioOutputSamplingFreq = GetChild<KaxAudioOutputSamplingFreq>(MyTrackAudio);
		*(static_cast<EbmlFloat *>(&MyTrackAudioOutputSamplingFreq)) = outputSampleRate;
		MyTrackAudioOutputSamplingFreq.ValidateSize();
	}

	if (bitdepth != 0) {
		KaxAudioBitDepth & MyTrackAudioBitDepth = GetChild<KaxAudioBitDepth>(MyTrackAudio);
		*(static_cast<EbmlUInteger *>(&MyTrackAudioBitDepth)) = bitdepth;
	}

	return 0;
};

int MatroskaMuxer::Set_Track_Lacing(uint8 trackNo, bool bUseLacing)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;

	*(static_cast<EbmlUInteger *>(&GetChild<KaxTrackFlagLacing>(*track))) = bUseLacing;

	return 0;
};

int MatroskaMuxer::Set_Tag(uint8 trackNo, const char *name, const char *value)
{
	MatroskaTagInfo *currentTags = NULL;

	if (trackNo > 0) {
		KaxTrackEntry *entry = _LookupTrackNo(trackNo);
		if (entry == NULL)
			return -1;

		KaxTrackUID &trackUID = GetChild<KaxTrackUID>(*entry);		
		currentTags = m_Tags.FindTagWithTrackUID((uint64)trackUID);
		if (currentTags == NULL) {
			MatroskaTagInfo tempTag;
			tempTag.targetTrackUID = (uint64)trackUID;
			m_Tags.push_back(tempTag);
			currentTags = &m_Tags.at(m_Tags.size()-1);
		}		
	} else {
		currentTags = m_Tags.FindTagWithTrackUID(0);
		if (currentTags == NULL) {
			MatroskaTagInfo tempTag;
			tempTag.targetTrackUID = 0;
			m_Tags.push_back(tempTag);
			currentTags = &m_Tags.at(m_Tags.size()-1);
		}		
	}
	currentTags->SetTagValue(name, value);
	return 0;
};

void MatroskaMuxer::Attach_File(MatroskaAttachment *attachment)
{
	m_Attachments.push_back(attachment);
}

int MatroskaMuxer::Get_Track_CodecID(uint8 trackNo, std::string &codecID)
{
	KaxTrackEntry *track = _LookupTrackNo(trackNo);
	if (track == NULL)
		return -1;
	
	codecID = *(static_cast<EbmlString *>(&GetChild<KaxCodecID>(*track)));

	return 0;
}

KaxTrackEntry *MatroskaMuxer::_LookupTrackNo(uint8 trackNo)
{
	KaxTrackEntry *entry = NULL;

	if (trackNo == 0)
		return NULL;

	trackNo--;

	if (m_Tracks->ListSize() > trackNo) {
		entry = static_cast<KaxTrackEntry *>((*m_Tracks)[trackNo]);
		assert(entry != NULL);
	} else {
		if (m_Tracks->ListSize() > 0) {
			entry = &GetNextChild<KaxTrackEntry>(*m_Tracks, static_cast<KaxTrackEntry &>(*(*m_Tracks)[m_Tracks->ListSize()-1]));
		} else {
			entry = &GetChild<KaxTrackEntry>(*m_Tracks);
		}
		assert(entry != NULL);

		KaxTrackNumber & MyTrack1Number = GetChild<KaxTrackNumber>(*entry);
		*(static_cast<EbmlUInteger *>(&MyTrack1Number)) = trackNo + 1;			

		KaxTrackUID & MyKaxTrack1UID = GetChild<KaxTrackUID>(*entry);			
		*(static_cast<EbmlUInteger *>(&MyKaxTrack1UID)) = rand();		

		*(static_cast<EbmlString *>(&GetChild<KaxTrackLanguage>(*entry))) = "eng";

		entry->SetGlobalTimecodeScale(m_timecodeScale);

		m_Queue.resize(trackNo+1);
		for (size_t it = 0;	it < m_Queue.size(); it++)
			m_Queue.at(it).trackNo = it+1;
		
	}
		
	return entry;
};

bool MatroskaMuxer::WriteHeaders() {
	try {
		//wxLogStatus(_T("MatroskaMuxer::WriteHeaders()"));

		// write output file
		EbmlHead FileHead;
	  
		EDocType & MyDocType = GetChild<EDocType>(FileHead);
		*static_cast<EbmlString *>(&MyDocType) = "matroska";
	  
		EDocTypeVersion & MyDocTypeVer = GetChild<EDocTypeVersion>(FileHead);
		*(static_cast<EbmlUInteger *>(&MyDocTypeVer)) = 1;
	  
		EDocTypeReadVersion & MyDocTypeReadVer = GetChild<EDocTypeReadVersion>(FileHead);
		*(static_cast<EbmlUInteger *>(&MyDocTypeReadVer)) = 1;
	  
		FileHead.Render(*m_outputFile);

		// Total segment size is unknown and will always be
    // So we alloc a fixed amount of space (5 bytes) that we can render right away
		uint64 SegmentSize = FileSegment.WriteHead(*m_outputFile, 5);
		
		// Write a dummy element to reserve space for the seekhead
		MyDummy->SetSize(2000);
		
		MyDummy->Render(*m_outputFile);


		//Add the Info segment
		KaxInfo & MyInfos = GetChild<KaxInfo>(FileSegment);

		//Add info tag for writing app
		KaxWritingApp & MyKaxWritingApp = GetChild<KaxWritingApp>(MyInfos);		
		*static_cast<EbmlUnicodeString *>(&MyKaxWritingApp) = m_WritingApp;

		std::string muxingapp;
		muxingapp.append("libebml v");
		muxingapp.append(LIBEBML_NAMESPACE::EbmlCodeVersion);
		muxingapp.append(" + libmatroska v");
		muxingapp.append(LIBMATROSKA_NAMESPACE::KaxCodeVersion);
		UTFstring muxingappW;
		muxingappW.SetUTF8(muxingapp);

		KaxMuxingApp & MyKaxMuxingApp = GetChild<KaxMuxingApp>(MyInfos);
		*static_cast<EbmlUnicodeString *>(&MyKaxMuxingApp) = muxingappW;
		
		KaxDateUTC &MyKaxDateUTC = GetChild<KaxDateUTC>(MyInfos);
		MyKaxDateUTC.SetEpochDate(time(NULL));

		//Add timecode scale
		KaxTimecodeScale & TimeScale = GetChild<KaxTimecodeScale>(MyInfos);
		*(static_cast<EbmlUInteger *>(&TimeScale)) = m_timecodeScale;

		KaxDuration &Duration = GetChild<KaxDuration>(MyInfos);
		*(static_cast<EbmlFloat *>(&Duration)) = 10000;

		KaxSegmentUID & MyKaxSegmentUID = GetChild<KaxSegmentUID>(MyInfos);		
		// Now we need to make a 128-bits UID
		uint32 *segmentUID = reinterpret_cast<uint32_t*>(std::malloc(4 * sizeof(uint32_t)));
		srand((unsigned int)time(NULL));
		segmentUID[0] = rand();
		srand((unsigned int)time(NULL));
		segmentUID[1] = rand();
		srand((unsigned int)time(NULL));
		segmentUID[2] = rand();
		srand((unsigned int)time(NULL));
		segmentUID[3] = rand();
		MyKaxSegmentUID.SetBuffer((const binary *)segmentUID, 4 * sizeof(uint32_t));

		uint32 InfoSize = MyInfos.Render(*m_outputFile);
		MySeekHead->IndexThis(MyInfos, FileSegment);

		// Setup the Cues but don't render them yet
		m_AllCues = &GetChild<KaxCues>(FileSegment);
		m_AllCues->SetGlobalTimecodeScale(m_timecodeScale);		

		uint64 TrackSize = m_Tracks->Render(*m_outputFile);
		MySeekHead->IndexThis(*m_Tracks, FileSegment);
		
		// Lastly, write any attachments that have been submitted by this point.
		return _WriteAttachments();
	}catch (...) {
		//wxLogMessage(_T("MatroskaMuxer: Failed to write headers"));
		return false;
	}
	return false;	
};

bool MatroskaMuxer::_WriteAttachments() {
	try {
		//wxLogStatus(_T("Matroska Writer: Writing Attachments..."));

		if (!m_Attachments.empty()) {
			KaxAttachments & MyAttachments = GetChild<KaxAttachments>(FileSegment);

			KaxAttached *lastAttachment = NULL;		
			for (uint16 a = 0; a < m_Attachments.size(); a++) {
				MatroskaAttachment *attachment = m_Attachments.at(a);		
				if (!attachment) continue;
				m_Attachments.at(a) = NULL;

				KaxAttached *MyAttachedFile = NULL;	
				if (lastAttachment != NULL) {
					MyAttachedFile = &GetNextChild<KaxAttached>(MyAttachments, *lastAttachment);
					lastAttachment = MyAttachedFile;
				} else {
					MyAttachedFile = &GetChild<KaxAttached>(MyAttachments);
					lastAttachment = MyAttachedFile;
				}
				
				KaxFileUID & MyKaxFileUID = GetChild<KaxFileUID>(*MyAttachedFile);
				static_cast<EbmlUInteger *>(&MyKaxFileUID)->SetValue(attachment->UID);

				KaxFileName & MyAttachedFile_Filename  = GetChild<KaxFileName>(*MyAttachedFile);
				static_cast<EbmlUnicodeString *>(&MyAttachedFile_Filename)->SetValueUTF8(attachment->FileName);
				
				KaxMimeType & MyAttachedFile_MimieType  = GetChild<KaxMimeType>(*MyAttachedFile);
				static_cast<EbmlString *>(&MyAttachedFile_MimieType)->SetValue(attachment->MimeType);

				KaxFileDescription & MyKaxFileDescription  = GetChild<KaxFileDescription>(*MyAttachedFile);
				static_cast<EbmlUnicodeString *>(&MyKaxFileDescription)->SetValueUTF8(attachment->Description);

				KaxFileData & MyAttachedFile_FileData  = GetChild<KaxFileData>(*MyAttachedFile);
				MyAttachedFile_FileData.CopyBuffer(attachment->GetAttachmentData(), attachment->GetAttachmentDataLength());

				delete attachment;
			}
			MyAttachments.Render(*m_outputFile);
			MySeekHead->IndexThis(MyAttachments, FileSegment);

			m_Attachments.clear();
		}
		return true;
	} catch (std::exception &ex) {
		//wxString errMsg;
		//errMsg = _T("Matroska Writer: std::exception in MatroskaWriter::WriteAttachments() - ");
		//errMsg += wxString(ex.what(), wxConvUTF8);
		//wxLogError(errMsg);
	}catch (...) {
		//wxLogError(_T("Matroska Writer: Failed to write attachments."));
	}
	return false;
};

bool MatroskaMuxer::_WriteTags() {
	try {
		//wxLogStatus(_T("MatroskaMuxer::WriteTags()"));

		KaxTags & MyKaxTags = GetChild<KaxTags>(FileSegment);	

		//Start going through the list and adding tags
		KaxTag *MyKaxTag_last = NULL;
		for(size_t current_tag_track = 0; current_tag_track < m_Tags.size(); current_tag_track++) {
			KaxTag *MyKaxTag = NULL;
			if (MyKaxTag_last != NULL)
			{
				MyKaxTag = &GetNextChild<KaxTag>(MyKaxTags, *MyKaxTag_last);
				MyKaxTag_last = MyKaxTag;
			}else {
				MyKaxTag = &GetChild<KaxTag>(MyKaxTags);
				MyKaxTag_last = MyKaxTag;
			}
			MatroskaTagInfo &currentTag = m_Tags.at(current_tag_track);
			//The Targets group
			KaxTagTargets & MyKaxTagTargets = GetChild<KaxTagTargets>(*MyKaxTag);
			if (currentTag.targetTrackUID != 0) {
				KaxTagTrackUID & MyKaxTagTrackUID = GetChild<KaxTagTrackUID>(MyKaxTagTargets);
				*static_cast<EbmlUInteger *>(&MyKaxTagTrackUID) = currentTag.targetTrackUID;
			}
			if (currentTag.targetChapterUID != 0) {
				KaxTagChapterUID & MyKaxTagChapterUID = GetChild<KaxTagChapterUID>(MyKaxTagTargets);
				*static_cast<EbmlUInteger *>(&MyKaxTagChapterUID) = currentTag.targetChapterUID;
			}
			if (currentTag.targetAttachmentUID != 0) {
				KaxTagAttachmentUID & MyKaxTagAttachmentUID = GetChild<KaxTagAttachmentUID>(MyKaxTagTargets);
				*static_cast<EbmlUInteger *>(&MyKaxTagAttachmentUID) = currentTag.targetAttachmentUID;
			}

			// Add the millions of simple tags we can have ;)
			KaxTagSimple *MySimpleTag_last = NULL;
			for(size_t st = 0; st < currentTag.tags.size(); st++) {
				KaxTagSimple *MySimpleTag = NULL;
				if (MySimpleTag_last != NULL)
				{
					MySimpleTag = &GetNextChild<KaxTagSimple>(*MyKaxTag, *MySimpleTag_last);
					MySimpleTag_last = MySimpleTag;
				}else {
					MySimpleTag = &GetChild<KaxTagSimple>(*MyKaxTag);
					MySimpleTag_last = MySimpleTag;
				}
				MatroskaSimpleTag &currentSimpleTag = currentTag.tags.at(st);

				KaxTagName & MyKaxTagName = GetChild<KaxTagName>(*MySimpleTag);
				*static_cast<EbmlUnicodeString *>(&MyKaxTagName) = currentSimpleTag.name;

				KaxTagString & MyKaxTagString = GetChild<KaxTagString>(*MySimpleTag);
				*static_cast<EbmlUnicodeString *>(&MyKaxTagString) = currentSimpleTag.value;
			}
		}

		MyKaxTags.Render(*m_outputFile);
		MySeekHead->IndexThis(MyKaxTags, FileSegment);

		return true;
	} catch (std::exception &ex) {
		//wxString errMsg;
		//errMsg = _T("Matroska Writer: std::exception in MatroskaWriter::WriteTags() - ");
		//errMsg += wxString(ex.what(), wxConvUTF8);
		//wxLogError(errMsg);
	}catch (...) {
		//wxLogError(_T("Matroska Writer: Failed to write tags."));
	}
	return false;
};

int MatroskaMuxer::_UpdateDuration(double newDuration)
{
  KaxInfo & MyInfos = GetChild<KaxInfo>(FileSegment);
  
  KaxDuration &Duration = GetChild<KaxDuration>(MyInfos);
  *(static_cast<EbmlFloat *>(&Duration)) = newDuration;

  MyInfos.UpdateSize();
  
  uint64 origPos = m_outputFile->getFilePointer();
  m_outputFile->setFilePointer(MyInfos.GetElementPosition());
  MyInfos.Render(*m_outputFile);
  
  m_outputFile->setFilePointer(origPos);
  
  return 0;
};

int MatroskaMuxer::_AddFrame(uint16 trackNo, uint64 timecode, uint32 duration, SimpleDataBuffer *data, std::vector<int32> *references) {
	KaxTrackEntry *track = _LookupTrackNo(trackNo);	
	if (track == NULL)
		return -1;

	_ChangeClusters(timecode);
		
	if ((m_MyKaxBlockGroup == NULL) || (m_prevTrack != trackNo) || (m_LaceCount)) {
		m_MyKaxBlockGroup = &clust->GetNewBlock();		
		m_LaceCount = 0;
		m_MyKaxBlockGroup->SetParent(*clust);
	}	 	


	KaxBlock &MyKaxBlock = GetChild<KaxBlock>(*m_MyKaxBlockGroup);
	MyKaxBlock.SetParent(*clust);	
	uint8 UseLacing = (uint8)GetChild<KaxTrackFlagLacing>(*track);
	if (UseLacing)
		m_LaceCount = !MyKaxBlock.AddFrame(*track, timecode * m_timecodeScale, *data, LACING_AUTO);
	else
		MyKaxBlock.AddFrame(*track, timecode * m_timecodeScale, *data, LACING_NONE);

	currentClusterSize += data->Size();
	
	if (UseLacing) {
		m_prevTrack = trackNo;
	}
	if (timecode+duration > m_lastTimecode)
		m_lastTimecode = timecode+duration;
	
	// We only need to add the BlockDuration element if the Block Duration is different from the default
	if (m_WriteDuration && (duration != 0) && (duration != (uint32)GetChild<KaxTrackDefaultDuration>(*track))) {
		KaxBlockDuration &MyKaxBlockDuration = GetChild<KaxBlockDuration>(*m_MyKaxBlockGroup);
		*(static_cast<EbmlUInteger *>(&MyKaxBlockDuration)) = duration;
	}

	if ((references != NULL) && (references->size() > 0)) {
		KaxTimecodeReferenceBlock &MyKaxReferenceBlock = GetChild<KaxTimecodeReferenceBlock>(*m_MyKaxBlockGroup);
		MyKaxReferenceBlock.SetTimecode(references->at(0));
		// More than one reference?
		if (references->size() > 1) {
			KaxTimecodeReferenceBlock *lastReferenceBlock = &MyKaxReferenceBlock;
			for (size_t ref = 1; ref < references->size(); ref++) {				
				KaxTimecodeReferenceBlock *newReferenceBlock = &GetNextChild<KaxTimecodeReferenceBlock>(*m_MyKaxBlockGroup, *lastReferenceBlock);
				newReferenceBlock->SetTimecode(references->at(ref));
				lastReferenceBlock = newReferenceBlock;
			}
		}
	} else {
#if 0 // TO_DO
		if ((uint8)GetChild<KaxTrackType>(*track) == track_video)
			m_AllCues->AddBlockGroup(*m_MyKaxBlockGroup);
#endif
	}
	return 0;
};

int MatroskaMuxer::AddFrame(uint16 trackNo, uint64 timecode, uint32 duration, const void *data, uint32 dataSize, int32 reference1, int32 reference2)
{
	if ((trackNo > 0) && (trackNo <= m_Queue.size()))
	{
		MatroskaFrameQueue &trackQueue = m_Queue.at(trackNo-1);
		MatroskaFrame *frame = new MatroskaFrame();
		frame->timecode = timecode;
		frame->duration = duration;
		if (reference1 != 0) {
			if (reference1 == REFERENCE_PREV_FRAME) {
				if (!trackQueue.empty()) {
					MatroskaFrame *prevFrame = trackQueue.front();
					reference1 = (int64)prevFrame->timecode - (int64)timecode;

					frame->references.push_back(reference1);
				}
			} else {
				frame->references.push_back(reference1);
			}
		}
		if (reference2 != 0)
			frame->references.push_back(reference2);
		
		binary *buffer = reinterpret_cast<binary*>(std::malloc(dataSize));
		memcpy(buffer, data, dataSize);
		frame->buffer = new SimpleDataBuffer(buffer, dataSize, 0);

		trackQueue.push(frame);
		
		while (_ProcessQueue() != -1);

		return 0;
	}

	return -1;
};

int MatroskaMuxer::_ProcessQueue()
{
	MatroskaFrame *nextFrame = NULL;
	MatroskaFrameQueue *sourceQueue = NULL;
	
	// Go through the track queues
	for (size_t it = 0;	it < m_Queue.size(); it++) {
		MatroskaFrameQueue *inQueue = &m_Queue.at(it);


		// If queue has at least 1+ frames in the queue
		if (inQueue->size() > 1) {
			// Check if we NEED a frame or the frame in the queue has a earlier timecode than the frame we have
			if ((nextFrame == NULL) || 
				(((MatroskaFrame *)inQueue->front())->timecode < nextFrame->timecode))
			{
				// It does, so we store this frame
				nextFrame = ((MatroskaFrame *)inQueue->front());
				// Store the source track queue (so we can take this frame out of the tracks queue)
				sourceQueue = inQueue;
			}			
		}
	}
	if (nextFrame != NULL) {
		// Ok we have the frame with the smallest timecode
		// Take it out of it's source track queue
		sourceQueue->pop();
		int ret = _AddFrame(sourceQueue->trackNo, nextFrame->timecode, nextFrame->duration, nextFrame->buffer, &nextFrame->references);
		delete nextFrame;
		return ret;
	} else {
		// No more frames
		return -1;
	}


	return 0;
}

int MatroskaMuxer::_FlushQueue()
{
	MatroskaFrame *nextFrame = NULL;
	MatroskaFrameQueue *sourceQueue = NULL;
	
	bool bFlushed = false;
	while (!bFlushed) {
		// Go through the track queues
		for (size_t it = 0;	it < m_Queue.size(); it++) {
			MatroskaFrameQueue *inQueue = &m_Queue.at(it);


			// If queue has at least 1+ frames in the queue
			if (inQueue->size() > 0) {
				// Check if we NEED a frame or the frame in the queue has a earlier timecode than the frame we have
				if ((nextFrame == NULL) || 
					(((MatroskaFrame *)inQueue->front())->timecode < nextFrame->timecode))
				{
					// It does, so we store this frame
					nextFrame = ((MatroskaFrame *)inQueue->front());
					// Store the source track queue (so we can take this frame out of the tracks queue)
					sourceQueue = inQueue;
				}			
			}
		}
		if (nextFrame != NULL) {
			// Ok we have the frame with the smallest timecode
			// Take it out of it's source track queue
			sourceQueue->pop();
			int ret = _AddFrame(sourceQueue->trackNo, nextFrame->timecode, nextFrame->duration, nextFrame->buffer, &nextFrame->references);
			delete nextFrame;			
			nextFrame = NULL;
		} else {
			// No more frames
			bFlushed = true;
		}
	}
	return 0;
}

bool MatroskaMuxer::_ChangeClusters(uint64 timecode, bool bForce) {
	if (!clust || (bForce || ((timecode - currentClusterBaseTimecode) >= m_maxTimePerCluster) || (currentClusterSize >= m_maxBytesPerCluster))) {
#ifdef _DEBUG_CLUSTER_LOG
		if ((timecode - currentClusterBaseTimecode) >= m_maxTimePerCluster) {
			printf("Limited by time");
		}
		if (currentClusterSize >= m_maxBytesPerCluster) {
			printf("Limited by size");
		}
#endif
		//Preform the cluster cleanup
		if (clust) {

			clust->Render(*m_outputFile, *m_AllCues);
			clust->ReleaseFrames();				

			// We leave 75 bytes of space reserved for the first seek head
			// Each entry is about 15 bytes, so we leave space for 4 entries to fit easily
			if (MySeekHead->GetSize()+75 < MyDummy->GetSize()) {
				MySeekHead->IndexThis(*clust, FileSegment);
				MySeekHead->UpdateSize();
			} else {
				// The first one is full, we add the clusters to the linked seekhead, at the end of the file
				MySeekHeadOverFill->IndexThis(*clust, FileSegment);
			}

			delete clust;
			clust = NULL;
			currentClusterSize = 0;
			// We have to start a new blockgroup
			m_MyKaxBlockGroup = NULL;
		}
		++clustersMuxed;
		clust = new KaxCluster();
		clust->SetParent(FileSegment);
		clust->SetGlobalTimecodeScale(m_timecodeScale);
		clust->SetPreviousTimecode(currentClusterBaseTimecode, m_timecodeScale);
		clust->InitTimecode(timecode, m_timecodeScale);

		//Set the timecode for this cluster
		KaxClusterTimecode & MyClusterTimeCode = GetChild<KaxClusterTimecode>(*clust);
		*(static_cast<EbmlUInteger *>(&MyClusterTimeCode)) = timecode;// * m_timecodeScale;
		
		//Store the starting timecode of this cluster
		currentClusterBaseTimecode = timecode;
	}
	return true;
};

int MatroskaMuxer::CloseFile() {
	//wxLogStatus(_T("MatroskaMuxer::CloseFile()"));
	_FlushQueue();

	if (clust) {		
    clust->Render(*m_outputFile, *m_AllCues);
    clust->ReleaseFrames();
    delete clust;	
		clust = NULL;
  }	

	if ((m_AllCues != NULL) && (m_AllCues->ListSize() > 0)) {
		m_AllCues->Render(*m_outputFile);
		MySeekHead->IndexThis(*m_AllCues, FileSegment);
	}

	_UpdateDuration((double)(int64)m_lastTimecode);

	// If we never used the Overfill seekhead there isn't much use of rendering it :P
	if (MySeekHeadOverFill->ListSize() != 0) {
		MySeekHeadOverFill->Render(*m_outputFile);
		// Index the seekhead that is at the end in the one at the beginning
		// creating a 'linked-list' type of seekheads
		MySeekHead->IndexThis(*MySeekHeadOverFill, FileSegment);
	}

	_WriteTags();
	_WriteAttachments();

	MyDummy->ReplaceWith(*MySeekHead, *m_outputFile);

	m_outputFile->setFilePointer(0, seek_end);
	if (FileSegment.ForceSize(m_outputFile->getFilePointer() -
                             FileSegment.GetElementPosition() -
                             FileSegment.HeadSize()))
    FileSegment.OverwriteHead(*m_outputFile);

	return 0;
};


} // namespace mkvwriter




#ifndef _MATROSKA_ATTACHMENT_H_
#define _MATROSKA_ATTACHMENT_H_


#include <string>
#include <vector>

#include "ebml/EbmlTypes.h"


namespace mkvwriter {


class MatroskaAttachment {
public:
    MatroskaAttachment();

    const uint8 *GetAttachmentData() const
    {
        return &Data.front();
    }

    size_t GetAttachmentDataLength() const
    {
        return Data.size();
    }

    std::string         Description;
    std::string         FileName;
    std::string         MimeType;
    std::vector<uint8>  Data;
    uint64              UID;
};

typedef std::vector<MatroskaAttachment *> MatroskaAttachmentVector;


} // namespace mkvwriter


#endif // _MATROSKA_ATTACHMENT_H_


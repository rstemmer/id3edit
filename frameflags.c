#include <id3v2.h>

int ID3V2_DecodeFrameFlags(int version, const ID3V2_FRAME *frame, ID3V2_FRAMEFLAGS *frameflags)
{
    if(frame == NULL || frameflags == NULL)
        return ID3V2ERROR_NOERROR;

    unsigned short flags;
    flags = frame->flags;

#define DECODEFRAMEFLAG(C,v) if(flags & C) v = true; else v = false;
    if(version == 4)
    {
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_TagAlterPreservation , frameflags->TagAlterPreservation );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_FileAlterPreservation, frameflags->FileAlterPreservation);
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_ReadOnly             , frameflags->ReadOnly             );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_Compression          , frameflags->Compression          );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_Encryption           , frameflags->Encryption           );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_GroupingIdentity     , frameflags->GroupingIdentity     );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_Unsynchronisation    , frameflags->Unsynchronisation    );
        DECODEFRAMEFLAG(ID3V240FRAMEFLAG_DataLengthIndicator  , frameflags->DataLengthIndicator  );
    }
    else
    {
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_TagAlterPreservation , frameflags->TagAlterPreservation );
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_FileAlterPreservation, frameflags->FileAlterPreservation);
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_ReadOnly             , frameflags->ReadOnly             );
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_Compression          , frameflags->Compression          );
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_Encryption           , frameflags->Encryption           );
        DECODEFRAMEFLAG(ID3V230FRAMEFLAG_GroupingIdentity     , frameflags->GroupingIdentity     );
        frameflags->Unsynchronisation   = false;    // \_ Does not exist for ID3v2.3.0
        frameflags->DataLengthIndicator = false;    // /
    }

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_EncodeFrameFlags(int version, ID3V2_FRAME *frame, const ID3V2_FRAMEFLAGS *frameflags)
{
    if(frame == NULL || frameflags == NULL)
        return ID3V2ERROR_NOERROR;

    unsigned short flags = 0;

#define ENCODEFRAMEFLAG(C,v) if(v) flags |= C;
    if(version == 4)
    {
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_TagAlterPreservation , frameflags->TagAlterPreservation );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_FileAlterPreservation, frameflags->FileAlterPreservation);
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_ReadOnly             , frameflags->ReadOnly             );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_Compression          , frameflags->Compression          );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_Encryption           , frameflags->Encryption           );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_GroupingIdentity     , frameflags->GroupingIdentity     );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_Unsynchronisation    , frameflags->Unsynchronisation    );
        ENCODEFRAMEFLAG(ID3V240FRAMEFLAG_DataLengthIndicator  , frameflags->DataLengthIndicator  );
    }
    else
    {
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_TagAlterPreservation , frameflags->TagAlterPreservation );
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_FileAlterPreservation, frameflags->FileAlterPreservation);
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_ReadOnly             , frameflags->ReadOnly             );
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_Compression          , frameflags->Compression          );
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_Encryption           , frameflags->Encryption           );
        ENCODEFRAMEFLAG(ID3V230FRAMEFLAG_GroupingIdentity     , frameflags->GroupingIdentity     );
    }

    frame->flags = flags;
    return ID3V2ERROR_NOERROR;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


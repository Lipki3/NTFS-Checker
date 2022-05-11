#pragma once
#include "Header.h"

#pragma pack(push, 1)



struct BootSector
{
    BYTE        jump[3];
    BYTE        oemID[8];
    WORD        bytePerSector;
    BYTE        sectorPerCluster;
    BYTE        reserved[2];
    BYTE        zero1[3];
    BYTE        unused1[2];
    BYTE        mediaDescriptor;
    BYTE        zeros2[2];
    WORD        sectorPerTrack;
    WORD        headNumber;
    DWORD       hiddenSector;
    BYTE        unused2[8];
    LONGLONG    totalSector;
    LONGLONG    MFTCluster;
    LONGLONG    MFTMirrCluster;
    signed char clusterPerRecord;
    BYTE        unused3[3];
    signed char clusterPerBlock;
    BYTE        unused4[3];
    LONGLONG    serialNumber;
    DWORD       checkSum;
    BYTE        bootCode[0x1aa];
    BYTE        endMarker[2];
};

struct RecordHeader
{
    BYTE        signature[4];
    WORD        updateOffset;
    WORD        updateNumber;
    LONGLONG    logFile;
    WORD        sequenceNumber;
    WORD        hardLinkCount;
    WORD        attributeOffset;
    WORD        flag;
    DWORD       usedSize;
    DWORD       allocatedSize;
    LONGLONG    baseRecord;
    WORD        nextAttributeID;
    BYTE        unsed[2];
    DWORD       MFTRecord;
};

struct AttributeHeaderR
{
    DWORD       typeID;
    DWORD       length;
    BYTE        formCode;
    BYTE        nameLength;
    WORD        nameOffset;
    WORD        flag;
    WORD        attributeID;
    DWORD       contentLength;
    WORD        contentOffset;
    WORD        unused;
};

struct AttributeHeaderNR
{
    DWORD       typeID;
    DWORD       length;
    BYTE        formCode;
    BYTE        nameLength;
    WORD        nameOffset;
    WORD        flag;
    WORD        attributeID;
    LONGLONG    startVCN;
    LONGLONG    endVCN;
    WORD        runListOffset;
    WORD        compressSize;
    DWORD       zero;
    LONGLONG    contentDiskSize;
    LONGLONG    contentSize;
    LONGLONG    initialContentSize;
};

struct FileName
{
    LONGLONG    parentDirectory;
    LONGLONG    dateCreated;
    LONGLONG    dateModified;
    LONGLONG    dateMFTModified;
    LONGLONG    dateAccessed;
    LONGLONG    logicalSize;
    LONGLONG    diskSize;
    DWORD       flag;
    DWORD       reparseValue;
    BYTE        nameLength;
    BYTE        nameType;
    BYTE        name[1];
};

struct AttributeRecord
{
    DWORD       typeID;
    WORD        recordLength;
    BYTE        nameLength;
    BYTE        nameOffset;
    LONGLONG    lowestVCN;
    LONGLONG    recordNumber;
    WORD        sequenceNumber;
    WORD        reserved;
};

#pragma pack(pop)


typedef struct {
    ULONG Type;
    USHORT UsaOffset;
    USHORT UsaCount;
    USN Usn;
} NTFS_RECORD_HEADER, * PNTFS_RECORD_HEADER;



// Type needed for interpreting the MFT-records

typedef struct {
    NTFS_RECORD_HEADER RecHdr;    // An NTFS_RECORD_HEADER structure with a Type of 'FILE'.
    USHORT SequenceNumber;        // Sequence number - The number of times
                                                  // that the MFT entry has been reused.
    USHORT LinkCount;             // Hard link count - The number of directory links to the MFT entry
    USHORT AttributeOffset;       // Offset to the first Attribute - The offset, in bytes,
                                                  // from the start of the structure to the first attribute of the MFT
    USHORT Flags;                 // Flags - A bit array of flags specifying properties of the MFT entry
                                                  // InUse 0x0001 - The MFT entry is in use
                                                  // Directory 0x0002 - The MFT entry represents a directory
    ULONG BytesInUse;             // Real size of the FILE record - The number of bytes used by the MFT entry.
    ULONG BytesAllocated;         // Allocated size of the FILE record - The number of bytes
                                                  // allocated for the MFT entry
    ULONGLONG BaseFileRecord;     // reference to the base FILE record - If the MFT entry contains
                                                  // attributes that overflowed a base MFT entry, this member
                                                  // contains the file reference number of the base entry;
                                                  // otherwise, it contains zero
    USHORT NextAttributeNumber;   // Next Attribute Id - The number that will be assigned to
                                                  // the next attribute added to the MFT entry.
    USHORT Pading;                // Align to 4 byte boundary (XP)
    ULONG MFTRecordNumber;        // Number of this MFT Record (XP)
    USHORT UpdateSeqNum;          //
} FILE_RECORD_HEADER, * PFILE_RECORD_HEADER;


struct Run
{
    LONGLONG    offset;
    LONGLONG    length;
    Run() : offset(0LL), length(0LL) {}
    Run(LONGLONG offset, LONGLONG length) : offset(offset), length(length) {}
};
#include "Header.h"
#include "Structs.h"
using namespace std;

void fixRecord(BYTE* buffer, DWORD recordSize, DWORD sectorSize)
{
    RecordHeader* header = (RecordHeader*)buffer;
    LPWORD update = LPWORD(buffer + header->updateOffset);

    if (LPBYTE(update + header->updateNumber) > buffer + recordSize)
        throw _T("Update sequence number is invalid");

    for (int i = 1; i < header->updateNumber; i++)
        *LPWORD(buffer + i * sectorSize - 2) = update[i];
}

void readRecord(HANDLE h, LONGLONG recordIndex, const vector<Run>& MFTRunList,
    DWORD recordSize, DWORD clusterSize, DWORD sectorSize, BYTE* buffer)
{
    LONGLONG sectorOffset = recordIndex * recordSize / sectorSize;
    DWORD sectorNumber = recordSize / sectorSize;

    for (DWORD sector = 0; sector < sectorNumber; sector++)
    {
        LONGLONG cluster = (sectorOffset + sector) / (clusterSize / sectorSize);
        LONGLONG vcn = 0LL;
        LONGLONG offset = -1LL;

        for (const Run& run : MFTRunList)
        {
            if (cluster < vcn + run.length)
            {
                offset = (run.offset + cluster - vcn) * clusterSize
                    + (sectorOffset + sector) * sectorSize % clusterSize;
                break;
            }
            vcn += run.length;
        }
        if (offset == -1LL)
            throw _T("Failed to read file record");

        seek(h, offset);
        DWORD read;
        if (!ReadFile(h, buffer + sector * sectorSize, sectorSize, &read, NULL) ||
            read != sectorSize)
            throw _T("Failed to read file record");
    }

    fixRecord(buffer, recordSize, sectorSize);
}
#include "Header.h"
#include "Structs.h"
using namespace std;

vector<Run> parseRunList(BYTE* runList, DWORD runListSize, LONGLONG totalCluster)
{
    vector<Run> result;

    LONGLONG offset = 0LL;

    LPBYTE p = runList;
    while (*p != 0x00)
    {
        if (p + 1 > runList + runListSize)
            throw _T("Invalid data run");

        int lenLength = *p & 0xf;
        int lenOffset = *p >> 4;
        p++;

        if (p + lenLength + lenOffset > runList + runListSize ||
            lenLength >= 8 ||
            lenOffset >= 8)
            throw _T("Invalid data run");

        if (lenOffset == 0)
            throw _T("Sparse file is not supported");

        ULONGLONG length = 0;
        for (int i = 0; i < lenLength; i++)
            length |= *p++ << (i * 8);

        LONGLONG offsetDiff = 0;
        for (int i = 0; i < lenOffset; i++)
            offsetDiff |= *p++ << (i * 8);
        if (offsetDiff >= (1LL << ((lenOffset * 8) - 1)))
            offsetDiff -= 1LL << (lenOffset * 8);

        offset += offsetDiff;

        if (offset < 0 || totalCluster <= offset)
            throw _T("Invalid data run");

        result.push_back(Run(offset, length));
    }

    return result;
}
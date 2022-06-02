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
        int lenLength = *p & 0xf;
        int lenOffset = *p >> 4;
        p++;

        ULONGLONG length = 0;
        for (int i = 0; i < lenLength; i++)
            length |= *p++ << (i * 8);

        LONGLONG offsetDiff = 0;
        for (int i = 0; i < lenOffset; i++)
            offsetDiff |= *p++ << (i * 8);
        if (offsetDiff >= (1LL << ((lenOffset * 8) - 1)))
            offsetDiff -= 1LL << (lenOffset * 8);

        offset += offsetDiff;

        result.push_back(Run(offset, length));
    }

    return result;
}
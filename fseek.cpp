#include "Header.h"

void seek(HANDLE h, ULONGLONG position)
{
    LARGE_INTEGER pos;
    pos.QuadPart = (LONGLONG)position;

    LARGE_INTEGER result;
    if (!SetFilePointerEx(h, pos, &result, SEEK_SET) ||
        pos.QuadPart != result.QuadPart)
        throw "Failed to seek";
}
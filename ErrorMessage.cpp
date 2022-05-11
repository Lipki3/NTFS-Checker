#include "Header.h"


void ErrorMessage(DWORD dwCode) {
    DWORD dwErrCode = dwCode;
    DWORD dwNumChar;
    LPWSTR szErrString = NULL;  // will be allocated and filled by FormatMessage

    dwNumChar = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM, // use windows internal message table
        0,       // 0 since source is internal message table
        dwErrCode, // this is the error code number
        0,       // auto-determine language to use
        (LPWSTR)&szErrString, // the messsage
        0,                 // min size for buffer
        0);               // since getting message from system tables

    if (dwNumChar == 0)
        printf("LFormatMessage() failed, error % u\n", GetLastError());
    printf("LError code % u:\n % s\n", dwErrCode, szErrString);

    if (LocalFree(szErrString) != NULL)
        printf("LFailed to free up the buffer, error % u\n", GetLastError());
}

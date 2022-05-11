#include "Header.h"

using namespace std;

BOOL GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY* pdg)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;  // handle to the drive to be examined 
    BOOL bResult = FALSE;                 // results flag
    DWORD junk = 0;                     // discard results

    hDevice = CreateFileW(wszPath,          // drive to open
        0,                // no access to the drive
        FILE_SHARE_READ | // share mode
        FILE_SHARE_WRITE,
        NULL,             // default security attributes
        OPEN_EXISTING,    // disposition
        0,                // file attributes
        NULL);            // do not copy file attributes

    if (hDevice == INVALID_HANDLE_VALUE)    // cannot open the drive
    {
        return (FALSE);
    }

    bResult = DeviceIoControl(hDevice,                       // device to be queried
        IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
        NULL, 0,                       // no input buffer
        pdg, sizeof(*pdg),            // output buffer
        &junk,                         // # bytes returned
        (LPOVERLAPPED)NULL);          // synchronous I/O

    CloseHandle(hDevice);

    return (bResult);
}

void printgeometry() {
    TCHAR wszDrive[] = _T("\\\\.\\PhysicalDrive0");
    DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure
    BOOL bResult = FALSE;      // generic results flag
    ULONGLONG DiskSize = 0;    // size of the drive, in bytes
    bResult = GetDriveGeometry((LPWSTR)wszDrive, &pdg);
    if (bResult) {
        printf("\n*** DISK GEOMETRY ***\n");
        wprintf(L"Drive path      = %ws\n", wszDrive);
        wprintf(L"Cylinders       = %I64d\n", pdg.Cylinders);
        wprintf(L"Tracks/cylinder = %ld\n", (ULONG)pdg.TracksPerCylinder);
        wprintf(L"Sectors/track   = %ld\n", (ULONG)pdg.SectorsPerTrack);
        wprintf(L"Bytes/sector    = %ld\n", (ULONG)pdg.BytesPerSector);
        DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
            (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
        wprintf(L"Disk size       = %I64d (Bytes)\n"
            L"                = %.2f (Gb)\n",
            DiskSize, (double)DiskSize / (1024 * 1024 * 1024));

    }
    else {
        wprintf(L"GetDriveGeometry failed. Error %ld.\n", GetLastError());
    }
    printf("\n");
}
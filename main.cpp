
#include "Header.h"
#include "Structs.h"

using namespace std;


LPBYTE findAttribute(RecordHeader* record, DWORD recordSize, DWORD typeID)
{
    LPBYTE p = LPBYTE(record) + record->attributeOffset;
    while (true)
    {
        if (p + sizeof(AttributeHeaderR) > LPBYTE(record) + recordSize)
            break;

        AttributeHeaderR* header = (AttributeHeaderR*)p;
        if/+ (header->typeID == 0xffffffff)
            break;

        if (header->typeID == typeID &&
            p + header->length <= LPBYTE(record) + recordSize)
            return p;

        p += header->length;
    }
    return NULL;
}


//  read a run list of typeID of recordIndex
//  return stage of the attribute
int readList(HANDLE h, LONGLONG recordIndex, DWORD typeID,
    const vector<Run>& MFTRunList, DWORD recordSize, DWORD clusterSize,
    DWORD sectorSize, LONGLONG totalCluster, vector<Run>* runList,
    LONGLONG* contentSize = NULL)
{
    vector<BYTE> record(recordSize);
    readRecord(h, recordIndex, MFTRunList, recordSize, clusterSize, sectorSize,
        &record[0]);
    RecordHeader* recordHeader = (RecordHeader*)&record[0];
    int stage = 0;
        AttributeHeaderNR* headerNR = (AttributeHeaderNR*)findAttribute(
            recordHeader, recordSize, typeID);
        if (headerNR == NULL)
            return 0;
        if (headerNR->formCode == 0)
        {
             // attr resident
            stage = 1;
        }
        else
        {
            //  attr non-resident
            stage = 2;

            vector<Run> runListTmp = parseRunList(
                LPBYTE(headerNR) + headerNR->runListOffset,
                headerNR->length - headerNR->runListOffset, totalCluster);

            runList->resize(runListTmp.size());
            for (size_t i = 0; i < runListTmp.size(); i++)
                (*runList)[i] = runListTmp[i];

            if (contentSize != NULL)
                *contentSize = headerNR->contentSize;
        }
    return stage;
}


int main()
{

    LPWSTR* argv = NULL;
    HANDLE h = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    HANDLE hVolume;
  
    NTFS_VOLUME_DATA_BUFFER ntfsVolData = { 0 };
    BOOL bDioControl = FALSE;
    DWORD dwWritten = 0;
    int result = -1;

    try {
        int argc;
        argv = CommandLineToArgvW(GetCommandLine(), &argc);
        
        TCHAR drive[] = _T("\\\\?\\_:");
        LONGLONG targetRecord = -1;
        LPWSTR outputFile = NULL;
        
        switch (argc)   {
        case 2:
            drive[4] = argv[1][0];
            break;
        case 4:
            drive[4] = argv[1][0];
            targetRecord = _tstoi64(argv[2]);
            outputFile = argv[3];
            break;
        default:
            printf("Invalid parameters\n");
            throw 0;
        }

        //  Open
        h = CreateFile(
            drive,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
        if (h == INVALID_HANDLE_VALUE)
            throw _T("Failed to open drive");

       
            // DriveGeometry.cpp
            printgeometry(drive);

            //  Boot Sector
            BootSector bootSector;
            DWORD read;
            if (!ReadFile(h, &bootSector, sizeof bootSector, &read, NULL) ||
                read != sizeof bootSector)
                throw _T("Failed to read boot sector");

            if (memcmp(bootSector.oemID, "NTFS    ", 8) != 0) {
                printf("\nVolume is not NTFS. OEM ID: %s\n\n", bootSector.oemID);
                throw _T("Volume is not NTFS");
                return 0;
            }
            printf("Volume is NTFS. OEM ID: \"%s\"\n", bootSector.oemID);

            printf("*** MFT STRUCTURE\n");

            bDioControl = DeviceIoControl(h, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &ntfsVolData, sizeof(ntfsVolData), &dwWritten, NULL);
            _tprintf(_T("TotalClusters: %lu"), ntfsVolData.TotalClusters);
            _tprintf(_T("\nFreeClusters: %lu\n"), ntfsVolData.FreeClusters);
            // get ntfsVolData by calling DeviceIoControl()
            // with CtlCode FSCTL_GET_NTFS_VOLUME_DATA
            // setup output buffer - FSCTL_GET_NTFS_FILE_RECORD depends on this
            // a call to FSCTL_GET_NTFS_VOLUME_DATA returns the structure NTFS_VOLUME_DATA_BUFFER

            DWORD lpBytesReturned = 0;
          
            // Variables for MFT-reading
            NTFS_FILE_RECORD_INPUT_BUFFER   ntfsFileRecordInput;
            PNTFS_FILE_RECORD_OUTPUT_BUFFER ntfsFileRecordOutput;

            DWORD sectorSize = bootSector.bytePerSector;
            DWORD clusterSize = bootSector.bytePerSector * bootSector.sectorPerCluster;
            DWORD recordSize = bootSector.clusterPerRecord >= 0 ?
                bootSector.clusterPerRecord * clusterSize :
                1 << -bootSector.clusterPerRecord;
            LONGLONG totalCluster = bootSector.totalSector / bootSector.sectorPerCluster;


            //  _tprintf(_T("Byte/Sector: %u\n"), sectorSize);
            _tprintf(_T("Cluster Size: %u (Bytes)\n"), clusterSize);
            _tprintf(_T("Sector/Cluster: %u\n"), bootSector.sectorPerCluster);
            _tprintf(_T("Total Sector: %llu\n"), bootSector.totalSector);
            _tprintf(_T("Cluster of MFT Start: %llu\n"), bootSector.MFTCluster);
            _tprintf(_T("Record Size: %u (Bytes)\n"), recordSize);

            //  Read MFT size and run list
            vector<Run> MFTRunList(1, Run(
                bootSector.MFTCluster,
                24 * recordSize / clusterSize));
            LONGLONG MFTSize = 0LL;
            int MFTStage = readList(
                h,
                0,      //  $MFT
                0x80,   //  $Data
                MFTRunList,
                recordSize,
                clusterSize,
                sectorSize,
                totalCluster,
                &MFTRunList,
                &MFTSize);

            _tprintf(_T("MFT stage: %d\n"), MFTStage);
            if (MFTStage == 0 || MFTStage == 1)
                throw _T("MFT stage is 1");
            _tprintf(_T("MFT size: %llu\n"), MFTSize);
            ULONGLONG recordNumber = MFTSize / recordSize;
            _tprintf(_T("Record number: %llu\n\n"), recordNumber);

        if (argc == 2) {
            int choice = 0;
           printf("1 - Print n files\n2 - Print all files\n");
            while (!scanf("%d", &choice) || choice > 2 || choice < 1) {
                rewind(stdin);
                printf("\nError!\n1 - Print n files\n2 - Print all files\n");
            }
            if (choice == 1) {
                long int n, k;
                printf("\nEnter n (1 - %d): ", recordNumber);
                do {
                    while (!scanf("%d", &n) || n > recordNumber || n < 1) {
                        rewind(stdin);
                        printf("\nError!\nEnter n (1 - %d): ", recordNumber);
                    }
                    printf("\n");
                    if (n > recordNumber) printf("Error!\n");
                } while (n > recordNumber);
                printf("Enter start point: (0 - %d): ", recordNumber -n);
                do {
                    while (!scanf("%d", &k) || k > recordNumber - n || k < 0) {
                        rewind(stdin);
                        printf("\nError!\nEnter k (0 - %d): ", recordNumber -n);
                    }
                    printf("\n");
                    if (n > recordNumber)  printf("Error!\n");
                } while (n+k > recordNumber);

                printf("\n*** MFT FILES ***\n");
                //  Read file list
                _tprintf(_T("File List:\n"));
                vector<BYTE> record(recordSize);
 
                RecordHeader* recordHeader = (RecordHeader*)&record[0];
                for (ULONGLONG recordIndex = k; recordIndex < n+k; recordIndex++) {
                    _tprintf(_T("%12lld"), recordIndex);
                    try {
                        readRecord(h, recordIndex, MFTRunList, recordSize,
                            clusterSize, sectorSize, &record[0]);

                        if (memcmp(recordHeader->signature, "FILE", 4) != 0) {
                            _tprintf(_T(" -\n"));
                            continue;
                        }

                        if (recordHeader->baseRecord != 0LL) {
                            _tprintf(_T(" extension for %llu\n"), recordHeader->baseRecord);
                            continue;
                        }

                        switch (recordHeader->flag) {
                        case 0x0001: _tprintf(_T("      "));  break;
                        case 0x0003: _tprintf(_T("   dir"));  break;
                        default:     _tprintf(_T(" ?????"));
                        }

                        AttributeHeaderR* name = (AttributeHeaderR*)findAttribute(
                            recordHeader, recordSize, 0x30);

                        //  $File_Name outside the record is not supported
                        if (name == NULL)
                            throw _T("Failed to find $File_Name attribute");

                        FileName* content = (FileName*)(LPBYTE(name)
                            + name->contentOffset);

                        _tprintf(_T(" %.*s\n"), content->nameLength,
                            (LPTSTR)content->name);
                    }
                    catch (LPCTSTR error) {
                        _tprintf(_T(" %s\n"), error);
                    }
                }
            }
            if (choice == 2) {

                printf("\n*** MFT FILES ***\n");
                //  Read file list
                _tprintf(_T("File List:\n"));
                vector<BYTE> record(recordSize);
                RecordHeader* recordHeader = (RecordHeader*)&record[0];
                for (ULONGLONG recordIndex = 0; recordIndex < recordNumber; recordIndex++) {
                    _tprintf(_T("%12lld"), recordIndex);
                    try {
                        readRecord(h, recordIndex, MFTRunList, recordSize,
                            clusterSize, sectorSize, &record[0]);

                        if (memcmp(recordHeader->signature, "FILE", 4) != 0) {
                            _tprintf(_T(" -\n"));
                            continue;
                        }

                        if (recordHeader->baseRecord != 0LL) {
                            _tprintf(_T(" extension for %llu\n"), recordHeader->baseRecord);
                            continue;
                        }

                        switch (recordHeader->flag) {
                        case 0x0001: _tprintf(_T("      "));  break;
                        case 0x0003: _tprintf(_T("   dir"));  break;
                        default:     _tprintf(_T(" ?????"));
                        }

                        AttributeHeaderR* name = (AttributeHeaderR*)findAttribute(
                            recordHeader, recordSize, 0x30);
                          
                        //  $File_Name outside the record is not supported
                        if (name == NULL)
                            throw _T("Failed to find $File_Name attribute");

                        FileName* content = (FileName*)(LPBYTE(name)
                            + name->contentOffset);

                        _tprintf(_T(" %.*s\n"), content->nameLength,
                            (LPTSTR)content->name);
                    }
                    catch (LPCTSTR error) {
                        _tprintf(_T(" %s\n"), error);
                    }
                }

            }
        }
        
        if (argc == 4)
        {
            //  Read file
            _tprintf(_T("Record index: %llu\n"), targetRecord);
            _tprintf(_T("Output file name: %s\n"), outputFile);

            vector<Run> runList;
            LONGLONG contentSize;
            int stage = readList(
                h,
                targetRecord,
                0x80,   //  $Data
                MFTRunList,
                recordSize,
                clusterSize,
                sectorSize,
                totalCluster,
                &runList,
                &contentSize);
            if (stage == 0)
                throw _T("Not found $Data");

            switch (stage)
            {
            case 1: _tprintf(_T("Stage: 1 ($Data is resident)\n")); break;
            case 2: _tprintf(_T("Stage: 2 ($Data is non-resident)\n")); break;
            }

            output = CreateFile(outputFile, GENERIC_WRITE, FILE_SHARE_READ,
                NULL, CREATE_ALWAYS, 0, NULL);
            if (output == INVALID_HANDLE_VALUE)
                throw _T("Failed to open output file");

            if (stage == 1)
            {
                vector<BYTE> record(recordSize);
                RecordHeader* recordHeader = (RecordHeader*)&record[0];

                readRecord(h, targetRecord, MFTRunList, recordSize, clusterSize,
                    sectorSize, &record[0]);

                AttributeHeaderR* data = (AttributeHeaderR*)findAttribute(
                    recordHeader, recordSize, 0x80);

                _tprintf(_T("File size: %u\n"), data->contentLength);

                DWORD written;
                WriteFile(output, LPBYTE(data) + data->contentOffset,
                    data->contentLength, &written, NULL);
            }
            else
            {
                vector<BYTE> cluster(clusterSize);
                LONGLONG writeSize = 0;

                _tprintf(_T("Processing...\n"), writeSize);
                for (Run& run : runList)
                {
                    seek(h, run.offset * clusterSize);
                    for (LONGLONG i = 0; i < run.length; i++)
                    {
                        if (writeSize + run.length > contentSize)
                            throw _T("File size error");

                        if (!ReadFile(h, &cluster[0], clusterSize, &read, NULL) ||
                            read != clusterSize)
                            throw _T("Failed to read cluster");

                        DWORD s = DWORD(min(contentSize - writeSize, clusterSize));
                        DWORD written;
                        if (!WriteFile(output, &cluster[0], s, &written, NULL) ||
                            written != s)
                            throw _T("Failed to write output file");
                        writeSize += s;
                    }
                }
                if (writeSize != contentSize)
                {
                    _tprintf(_T("Expected size: %llu\n"), contentSize);
                    _tprintf(_T("Actual size: %llu\n"), writeSize);
                    throw _T("File size error");
                }
            }
            _tprintf(_T("Success\n"));
        }

        result = 0;
    }
    catch (LPWSTR error)
    {
        _tprintf(_T("Error: %s\n"), error);
    }
    catch (...)
    {}

    if (argv != NULL)
        GlobalFree(argv);
    if (h != NULL)
        CloseHandle(h);
    if (output != NULL)
        CloseHandle(output);

    return result;
}
#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define FSCTL_GET_NTFS_VOLUME_DATA \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)
#include <Windows.h>
#include <stdio.h>
#include <winioctl.h>        // Windows NT IOCTL коды
#include <tchar.h>
#include <shellapi.h>
#include <vector>
#include <functional>



// Structs.h
struct Run;
struct RecordHeader;
struct AttributeHeaderR;
struct AttributeHeaderNR;
struct AttributeRecord;
struct BootSector;
struct FileName;

// ErrorMessage.cpp
void ErrorMessage(DWORD dwCode);

// DriveGeometry.cpp
BOOL GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY* pdg);
void printgeometry(TCHAR*);

// fseek.cpp
void seek(HANDLE h, ULONGLONG position);

// run.cpp
std::vector<Run> parseRunList(BYTE* runList, DWORD runListSize, LONGLONG totalCluster);

// Records.cpp
void readRecord(HANDLE h, LONGLONG recordIndex, const std::vector<Run>& MFTRunList, 
    DWORD recordSize, DWORD clusterSize, DWORD sectorSize, BYTE* buffer);



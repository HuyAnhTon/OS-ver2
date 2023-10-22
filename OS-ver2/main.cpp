#include <windows.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFileW(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
        printf("Success!\n");
    }
}



// ------------------------------------------------------

int Hex2Dec(string hex) {

    int res = 0;
    int base = 1;

    for (int i = hex.size() - 1; i >= 0; i--) {

        if (hex[i] >= '0' && hex[i] <= '9') {

            res += (int(hex[i]) - 48) * base;
            base *= 16;
        }
        else {

            res += (int(hex[i]) - 87) * base;
            base *= 16;
        }
    }
    return res;
}



string read_offset(string offset, int n, const BYTE sector[512]) {

    stringstream builder;

    int start_index = stoi(offset, 0, 16);
    int end_index = start_index + (n - 1);

    builder.clear();

    for (int i = 0; i < n; i++) {

        builder << hex << setw(2) << setfill('0') << int(sector[end_index - i]);
    }

    return builder.str();
}


void printBootSectorInformation(const BYTE sector[512]) {

    for (int i = 0; i < 512; i++) {

        cout << hex << setw(2) << setfill('0') << int(sector[i]) << " ";

        if ((i + 1) % 16 == 0)
            cout << endl;
    }
}

string FAT_type(const BYTE sector[512]) {
    // Offset 52, doc 8 byte

    string offset = "52";
    int n = 8;


    stringstream builder;
    int start_index = stoi(offset, 0, 16);

    for (int i = 0; i < n; i++) {
        builder << sector[start_index + i];
    }

    return builder.str();

}

int bytes_per_sector(const BYTE sector[512]) {

    // Offset B, doc 2 bytes

    string offset = "B";
    int n = 2;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}

int sectors_per_cluster(const BYTE sector[512]) {

    // Offset D, doc 1 byte

    string offset = "D";
    int n = 1;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}

int sectors_BootSector(const BYTE sector[512]) {

    // offset E, doc 2 byte
    string offset = "E";
    int n = 2;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}

int quantity_FAT(const BYTE sector[512]) {

    // offset E, doc 2 byte
    string offset = "10";
    int n = 1;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}

int total_sectors_on_disk(const BYTE sector[512]) {

    // offset E, doc 2 byte
    string offset = "20";
    int n = 4;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}

int sectors_per_fat(const BYTE sector[512]) {

    // offset E, doc 2 byte
    string offset = "24";
    int n = 4;

    string data = read_offset(offset, n, sector);
    return Hex2Dec(data);
}




int main(int argc, char** argv)
{

    BYTE sector[512];
    ReadSector(L"\\\\.\\D:", 0, sector);

    printBootSectorInformation(sector);


    cout << "1. FAT type: " << FAT_type(sector) << endl;
    cout << "2. Bytes / 1 sector: " << dec <<  bytes_per_sector(sector) << endl;
    cout << "3. Sectors / 1 cluster (sC): " << dec << sectors_per_cluster(sector) << endl;
    cout << "4. Sectors in BootSector (sB): " << dec << sectors_BootSector(sector) << endl;
    cout << "5. Quantity of FAT (nF): " << dec <<  quantity_FAT(sector) << endl;
    cout << "6. Sectors of RDET: " << endl;
    cout << "7. Total sectors on disk: " << dec << total_sectors_on_disk(sector) << endl;
    cout << "8. Sectors / 1 FAT (sF): " << dec << sectors_per_fat(sector) << endl;
    cout << "9. First sector of FAT 1: " << dec << sectors_BootSector(sector) << endl;
    cout << "10. First sector of RDET: " << dec << sectors_BootSector(sector) + quantity_FAT(sector) * sectors_per_fat(sector) << endl;
    cout << "11. First sector of Data: " << dec << sectors_BootSector(sector) + quantity_FAT(sector) * sectors_per_fat(sector) << endl;


    BYTE fat[512];
    int fat_read_point = (102 - 1) * 512;
    ReadSector(L"\\\\.\\D:", fat_read_point, fat);
    printBootSectorInformation(fat);

    string s = "1a";
    int a = Hex2Dec(s);

    return 0;
}
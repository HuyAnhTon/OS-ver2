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


void printInformation(const BYTE sector[512]) {

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

int main(int argc, char** argv)
{

    BYTE sector[512];
    ReadSector(L"\\\\.\\D:", 0, sector);

    printInformation(sector);

    int bytes_per_sector = Hex2Dec(read_offset("B", 2, sector));
    int sectors_per_cluster = Hex2Dec(read_offset("D", 1, sector));
    int sectors_BootSector = Hex2Dec(read_offset("E", 2, sector));
    int quantity_FAT = Hex2Dec(read_offset("10", 1, sector));
    int first_cluster_RDET = Hex2Dec(read_offset("2C", 4, sector));
    int sectors_per_fat = Hex2Dec(read_offset("24", 4, sector));
    int total_sectors_on_disk = Hex2Dec(read_offset("20", 4, sector));


    cout << "1. FAT type: " << FAT_type(sector) << endl;
    cout << "2. Bytes / 1 sector: " << dec << bytes_per_sector << endl;
    cout << "3. Sectors / 1 cluster (sC): " << dec << sectors_per_cluster << endl;
    cout << "4. Sectors in BootSector (sB): " << dec << sectors_BootSector << endl;
    cout << "5. Quantity of FAT (nF): " << dec <<  quantity_FAT << endl;
    cout << "6. First cluster of RDET: " << dec << first_cluster_RDET << endl;
    cout << "7. Total sectors on disk: " << dec << total_sectors_on_disk << endl;
    cout << "8. Sectors / 1 FAT (sF): " << dec << sectors_per_fat << endl;
    cout << "9. First sector of FAT 1: " << dec << sectors_BootSector << endl;
    cout << "10. First sector of RDET: " << dec << sectors_BootSector + quantity_FAT * sectors_per_fat << endl;
    cout << "11. First sector of Data: " << dec << sectors_BootSector + quantity_FAT * sectors_per_fat << endl;



    return 0;
}
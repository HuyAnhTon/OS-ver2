#include <windows.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include "File.h"



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


    for (int i = 0; i < n; i++) {

        builder << hex << setw(2) << setfill('0') << int(sector[end_index - i]);
    }

    return builder.str();
}

string read_offset_raw(string offset, int n, const BYTE sector[512]) {

    stringstream builder;

    for (int i = 0; i < n; i++) {

        builder << sector[i];
    }

    return builder.str();
}



void printInformation(const BYTE sector[512]) {

    for (int i = 0; i < 512; i++) {

        cout << hex << setw(2) << setfill('0') << int(sector[i]) << " ";

        if ((i + 1) % 4 == 0)
            cout << " ";
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

int first_sector_of_cluster(int cluster_number) {

    BYTE sector[512];
    ReadSector(L"\\\\.\\D:", 0, sector);
    int sectors_per_cluster = Hex2Dec(read_offset("D", 1, sector));
    int sectors_BootSector = Hex2Dec(read_offset("E", 2, sector));
    int quantity_FAT = Hex2Dec(read_offset("10", 1, sector));
    int sectors_per_fat = Hex2Dec(read_offset("24", 4, sector));

    int first_data_sector = sectors_BootSector + quantity_FAT * sectors_per_fat;

    int result = first_data_sector + (cluster_number - 2) * sectors_per_cluster;

    return result;
}

string Int2String(int n) {
    stringstream builder;
    builder << hex << n;
    return builder.str();
}

//int clusters_root_directory_holding(int first_RDET_cluster, int bytes_per_sector, int sectors_BootSector) {
//
//    BYTE fat[512];
//    ReadSector(L"\\\\.\\D:", sectors_BootSector * bytes_per_sector, fat);
//    int count = 0;
//
//    
//    for (int i = 4 * first_RDET_cluster; i < 512; i = i + 4) {
//
//        string offset = Int2String(i);
//        cout << offset << endl;
//        string data = read_offset(offset, 4, fat);
//        cout << data << endl;
//
//        count++;
//
//        if (data == "0fffffff") {
//            break;
//        }
//    }
//    return count;
//
//}

vector<int> clusters_holding(int first_cluster, int sectors_BootSector, int sectors_per_fat) {

    vector<int> clusters;
    int read_point = 4 * first_cluster;
    int fat_sector = 0;
    string data = "";


    while (data != "0fffffff") {

        while ((fat_sector + 1) * 512 < read_point)
            fat_sector++;

        BYTE fat[512];
        ReadSector(L"\\\\.\\D:", (sectors_BootSector + fat_sector) * 512, fat);
        //printInformation(fat);

        clusters.push_back(read_point / 4);
        read_point = read_point - fat_sector * 512;

        string offset = Int2String(read_point);
        data = read_offset(offset, 4, fat);

        if (data != "0fffffff") {
            read_point = stoi(data, 0, 16);
        }
    }
    return clusters;
}


vector<BYTE*> get_entries(int first_cluster) {

    // initial
    BYTE sector[512];
    ReadSector(L"\\\\.\\D:", 0, sector);
    int sectors_per_cluster = Hex2Dec(read_offset("D", 1, sector));
    int sectors_BootSector = Hex2Dec(read_offset("E", 2, sector));
    int sectors_per_fat = Hex2Dec(read_offset("24", 4, sector));
    int quantity_FAT = Hex2Dec(read_offset("10", 1, sector));
    int first_data_sector = sectors_BootSector + quantity_FAT * sectors_per_fat;



    vector<int> clusters = clusters_holding(first_cluster, sectors_BootSector, sectors_per_fat);
    vector<BYTE*> entries;


    for (auto cluster : clusters) {

        int first_sector_of_cluster = first_data_sector + (cluster - 2) * sectors_per_cluster;

        for (int j = 0; j < sectors_per_cluster; j++) {

            BYTE sector[512];

            ReadSector(L"\\\\.\\D:", (first_sector_of_cluster + j) * 512, sector);


            for (int k = 0; k < 512; k = k + 32) {

                BYTE* new_entry = new BYTE[32];

                for (int h = 0; h < 32; h++) {
                    new_entry[h] = sector[k + h];
                }

                entries.push_back(new_entry);
            }
        }
    }

    return entries;
}

File* read_entries(vector<BYTE*> entries) {

    File* root = new File(true, "Root directory", 0, 0);

    bool isFolder = false;
    string name = "";
    string ext = "";
    int first_cluster;
    int size = 0;

    for (int i = 2; i < entries.size(); i++) {

        if (entries[i][0] == stoi("E5", 0, 16) || entries[i][0] == 0) {
            continue;
        }
        // sub_entry
        else if (entries[i][11] == 15) {

            name += read_offset_raw("1", 10, entries[i]);
            name += read_offset_raw("E", 12, entries[i]);
            name += read_offset_raw("1C", 4, entries[i]);
        }
        else {

            if (entries[i][11] == 16) {
                isFolder = true;
                ext = "";
                name = name + read_offset_raw("0", 8, entries[i]);

                string cluster_bytes = read_offset("14", 2, entries[i]) + read_offset("1A", 2, entries[i]);
                first_cluster = stoi(cluster_bytes, 0, 16);

                //string size_bytes = read_offset("1C", 4, entries[i]);
                //size = stoi(size_bytes, 0, 16);


                File* f = new File(isFolder, name, size, first_cluster);
                vector<BYTE*> folder_entries = get_entries(first_cluster);
                File* folder = read_entries(folder_entries);
                folder->setName(name);
                folder->setFirstCluster(first_cluster);
                root->add(folder);

            }
            else {
                name = name + read_offset_raw("0", 8, entries[i]);
                ext = read_offset_raw("8", 3, entries[i]);
                name += "." + ext;
                string cluster_bytes = read_offset("14", 2, entries[i]) + read_offset("1A", 2, entries[i]);
                first_cluster = stoi(cluster_bytes, 0, 16);

                string size_bytes = read_offset("1C", 4, entries[i]);
                size = stoi(size_bytes, 0, 16);

                File* f = new File(isFolder, name, size, first_cluster);
                root->add(f);
            }


            name = "";
            ext = "";
            isFolder = false;
        }
    }
    return root;
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
    int first_RDET_cluster = Hex2Dec(read_offset("2C", 4, sector));
    int sectors_per_fat = Hex2Dec(read_offset("24", 4, sector));
    int total_sectors_on_disk = Hex2Dec(read_offset("20", 4, sector));
    int first_data_sector = sectors_BootSector + quantity_FAT * sectors_per_fat;
    int first_RDET_sector = first_sector_of_cluster(first_RDET_cluster);


    cout << "1. FAT type: " << FAT_type(sector) << endl;
    cout << "2. Bytes / 1 sector: " << dec << bytes_per_sector << endl;
    cout << "3. Sectors / 1 cluster (sC): " << dec << sectors_per_cluster << endl;
    cout << "4. Sectors in BootSector (sB): " << dec << sectors_BootSector << endl;
    cout << "5. Quantity of FAT (nF): " << dec << quantity_FAT << endl;
    cout << "6. First cluster of RDET: " << dec << first_RDET_cluster << endl;
    cout << "7. Total sectors on disk: " << dec << total_sectors_on_disk << endl;
    cout << "8. Sectors / 1 FAT (sF): " << dec << sectors_per_fat << endl;
    cout << "9. First sector of FAT 1: " << dec << sectors_BootSector << endl;
    cout << "10. First sector of RDET: " << dec << first_RDET_sector << endl;
    cout << "11. First sector of Data: " << dec << first_data_sector << endl;





    cout << "------------Read entries-------------" << endl;
    vector<BYTE*> entries = get_entries(first_RDET_cluster);
    File* root = read_entries(entries);

    root->printTree();

    return 0;
}
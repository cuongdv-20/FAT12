#ifndef __FAT_FS_H_
#define __FAT_FS_H_
#include "HAL.h"

typedef struct {
    uint16_t rootLocation;
    uint32_t numberOfRootEntries;
    uint16_t sectorPerCluster;
    uint16_t numberOfFat;
    uint32_t sizeOfFat;
    uint32_t typeFat;
    uint32_t endOfFile;
    uint16_t numOfSecRoot;
    uint16_t sectorBeforeData;
    uint32_t sectorSize;
}fatBootGetInfo;
fatBootGetInfo fatBoot;

typedef struct fatEntryGetInfo{
    uint32_t entryIndex;
    uint32_t size;
    uint32_t date;
    uint32_t month;
    uint32_t year;
    uint32_t hrs;
    uint32_t min;
    uint8_t  name[9];
    uint8_t  extension[4];
    uint32_t address;
    uint32_t attr;
    uint16_t longNameBuff[255];
    bool longName;
    struct fatEntryGetInfo *pNext;
}fatEntry;

bool fat_read_boot_info(uint8_t *filePath);

void fat_read_entry(uint8_t *buff, uint32_t countEntry);

void* fat_read_root_dir(void);

void* fat_read_sub_dir(uint32_t firstCluster);

void fat_read_file(uint32_t firstCluster);

void fat_free_entry(void);

void fat_print_entry(void);

#endif

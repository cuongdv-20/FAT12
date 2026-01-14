#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "fat_fs.h"

#define OFFSET_NUMBER_OF_FAT    0x10
#define OFFSET_SIZE_OF_FAT      0x16
#define OFFSET_ROOT_ENTRIES     0x11
#define OFFSET_SEC_PER_CLUS     0x0d
#define OFFSET_BYTES_PER_SEC    0x0b
#define OFFSET_TYPE_OF_FAT      0x39
#define OFFSET_ENTRY_ATTR       0x0b
#define OFFSET_TYPE_FAT         0x0b
#define ENTRY_SIZE              32
#define INIT_YEAR               1980

uint32_t entryAddress;
uint16_t entryIndex = 1;
bool haveLongName = false;
fatEntry *pHead=NULL;
uint32_t subEntryCount = 0;

static uint32_t fat12_get_next_cluster(uint32_t currentCluster);
static uint32_t fat16_get_next_cluster(uint32_t currentCluster);
static uint32_t fat32_get_next_cluster(uint32_t currentCluster);
static uint32_t (*get_next_cluster)(uint32_t currentCluster);

/* Read information from boot sector */
bool fat_read_boot_info(uint8_t *filePath){
    if(kmc_init(filePath)){
        uint8_t buff[512];
        kmc_read_sector(0, buff);
        fatBoot.numberOfFat         = buff[OFFSET_NUMBER_OF_FAT];
        fatBoot.sizeOfFat           = buff[OFFSET_SIZE_OF_FAT]|buff[OFFSET_SIZE_OF_FAT+1]<<8;
        fatBoot.numberOfRootEntries = buff[OFFSET_ROOT_ENTRIES];
        fatBoot.sectorPerCluster    = buff[OFFSET_SEC_PER_CLUS];
        fatBoot.rootLocation        = fatBoot.numberOfFat*fatBoot.sizeOfFat+1;
        fatBoot.sectorSize          = buff[OFFSET_TYPE_FAT+1]<<8|buff[OFFSET_TYPE_FAT];
        fatBoot.numOfSecRoot        = (fatBoot.numberOfRootEntries*ENTRY_SIZE)/fatBoot.sectorSize;
        /* Number of sector in root directory */
        fatBoot.sectorBeforeData    = fatBoot.numOfSecRoot+fatBoot.rootLocation;
        kmc_sector_size(fatBoot.sectorSize);
        fatBoot.typeFat             = buff[OFFSET_TYPE_OF_FAT]<<8|buff[OFFSET_TYPE_OF_FAT+1];
        if(fatBoot.typeFat==0x3132){
            fatBoot.endOfFile = 0xFF;    /* FAT12*/
            get_next_cluster = fat12_get_next_cluster;
        }
        else if(fatBoot.typeFat==0x3136){
            fatBoot.endOfFile = 0xFFFF; /* FAT16*/
            get_next_cluster = fat16_get_next_cluster;
        }
        else if(fatBoot.typeFat==0x3332){
            fatBoot.endOfFile = 0x0FFFFFFF; /* FAT32*/
            get_next_cluster = fat32_get_next_cluster;
        }
        return true;
    }
    return false;
}

/* Function return next cluster from current cluster as parameter */
static uint32_t fat12_get_next_cluster(uint32_t currentCluster){
    uint8_t buff[512];
    uint32_t clusVal = 0;
    uint32_t fatByte = (currentCluster*3)/2;
    uint32_t sectorIndex = fatByte/fatBoot.sectorSize;

    if((fatByte+1)%fatBoot.sectorSize==0){             /* Read 2 sector */
        kmc_read_multi_sector(sectorIndex+1,2,buff);
    }
    else{
        kmc_read_sector(sectorIndex+1,buff);
    }
    if(currentCluster%2==0){
        clusVal = buff[(fatByte-sectorIndex*fatBoot.sectorSize)]|(buff[fatByte-sectorIndex*fatBoot.sectorSize+1]&0xf)<<8;
    }
    else{
        clusVal = buff[(fatByte-sectorIndex*fatBoot.sectorSize)]>>4|buff[fatByte-sectorIndex*fatBoot.sectorSize+1]<<4;
    }
    return clusVal;
}

static uint32_t fat16_get_next_cluster(uint32_t currentCluster){

}
static uint32_t fat32_get_next_cluster(uint32_t currentCluster){

}

/* Read entry information by buffer and entry index */
void fat_read_entry(uint8_t *buff, uint32_t countEntry){
    uint32_t countByte;
    uint32_t countSubEnt;

    fatEntry* newEntry = (fatEntry*)malloc(sizeof(fatEntry));
    for(countByte=countEntry ; countByte<8+countEntry; countByte++){
        newEntry->name[countByte-countEntry] = buff[countByte];
        }
        newEntry->name[8] = '\0';
        for(countByte = 8+countEntry ; countByte<11+countEntry; countByte++){
        newEntry->extension[countByte-countEntry-8] = buff[countByte];
        }
        newEntry->extension[3] = '\0';
        newEntry->entryIndex = entryIndex++;
        newEntry->size  = buff[countEntry+0x1f]<<24|buff[countEntry+0x1e]<<16|buff[countEntry+0x1d]<<8|buff[countEntry+0x1c];
        newEntry->year  = (buff[countEntry+0x19]>>1)+INIT_YEAR;
        newEntry->month = (buff[countEntry+0x19]&0x01)<<3|buff[countEntry+0x18]>>5;
        newEntry->date  = buff[countEntry+0x18]&0x1f;
        newEntry->hrs   = buff[countEntry+0x17]>>3;
        newEntry->min   = (buff[countEntry+0x17]&0x07)<<3|buff[countEntry+0x16]>>5;
        newEntry->address = buff[countEntry+0x1b]<<8|buff[countEntry+0x1a];
        newEntry->attr = buff[countEntry + 0x0b];
        newEntry->longName = haveLongName;
        if(pHead == NULL){
            pHead = newEntry;
            newEntry->pNext = NULL;
        }
        else{
            newEntry->pNext = pHead;
            pHead = newEntry;
        }
        if(newEntry->longName){
            for(countSubEnt=subEntryCount ; countSubEnt>0;countSubEnt--){
                for(countByte= 0x01 ; countByte < 0x0b; countByte+=2){
                    newEntry->longNameBuff[countByte-1+21*(subEntryCount-countSubEnt)]=buff[countEntry+countByte-ENTRY_SIZE*countSubEnt];
                }
                for(countByte= 0x0e ; countByte < 0x1a; countByte+=2){
                    newEntry->longNameBuff[countByte-4+21*(subEntryCount-countSubEnt)]=buff[countEntry+countByte-ENTRY_SIZE*countSubEnt];
                }
            }
        }
}

void* fat_read_root_dir(void){
    uint8_t buff[512];
    uint32_t countEntry;
    uint32_t countSector;
    uint32_t countByte;
    bool endOfEntry = false;

    system("cls");
    printf("Index\tFile Name\tType\tDate Modified\t   Size\t\tLFN\n\n");
    for(countSector = 0; countSector < fatBoot.numOfSecRoot; countSector++){
        kmc_read_sector(fatBoot.rootLocation+countSector,buff);
        for(countEntry = 0 ; countEntry<fatBoot.sectorSize; countEntry+=ENTRY_SIZE){
            if(buff[countEntry+OFFSET_ENTRY_ATTR]==0x0f){
                subEntryCount++;
                haveLongName = true;
            }
            if(buff[countEntry]==0) {
                endOfEntry = true;
            }
            else if(buff[countEntry+OFFSET_ENTRY_ATTR]!=0x0f){
                fat_read_entry(buff,countEntry);
                haveLongName = false;
                subEntryCount = 0;
            }
        }
        if(endOfEntry) break;
    }
    return pHead;
}

void* fat_read_sub_dir(uint32_t firstCluster){
    uint8_t buff[512];
    uint32_t countEntry;
    uint32_t countByte;
    uint32_t currentCluster = firstCluster;
    bool endOfEntry = false;

    system("cls");
    printf("Index\tFile Name\tType\tDate Modified\t   Size\t\tLFN\n\n");
    while(currentCluster!=fatBoot.endOfFile){
        kmc_read_multi_sector(currentCluster+fatBoot.sectorBeforeData-2,fatBoot.sectorPerCluster,buff);
        for(countEntry = 0 ; countEntry < fatBoot.sectorSize; countEntry += ENTRY_SIZE){
            if(buff[countEntry+OFFSET_ENTRY_ATTR]==0x0f){
                subEntryCount++;
                haveLongName = true;
            }
            if(buff[countEntry]==0) {
                endOfEntry = true;
            }
            else if(buff[countEntry+OFFSET_ENTRY_ATTR]!=0x0f){
                fat_read_entry(buff,countEntry);
                haveLongName = false;
                subEntryCount = 0;
            }
        }
        currentCluster = (*get_next_cluster)(currentCluster);
        if(endOfEntry) break;
    }
    return pHead;
}

void fat_read_file(uint32_t firstCluster){
    uint8_t buff[512];
    uint32_t countByte;
    uint32_t currentCluster = firstCluster;

    system("cls");
    while(currentCluster!=0xfff){
        kmc_read_multi_sector(currentCluster+fatBoot.sectorBeforeData-2,fatBoot.sectorPerCluster,buff);
        for(countByte = 0 ; countByte<fatBoot.sectorSize; countByte++){
            if(buff[countByte]!=0x07){
                printf("%c", buff[countByte]);
            }
        }
        currentCluster = (*get_next_cluster)(currentCluster);
    }
    printf("\nPress any key to go back");
}

/* Free linked list memory */
void fat_free_entry(void){
    fatEntry* tmp = pHead;
    while(pHead!=NULL){
        tmp = pHead->pNext;
        free(pHead);
        pHead = tmp;
    }
    entryIndex = 1;
}

/* Print entries of current directory */
void fat_print_entry(void){
    fatEntry* tmp = pHead;
    uint32_t countByte;
    while(tmp!=NULL){
        printf("%d\t",tmp->entryIndex);
        printf("%s\t",tmp->name);
        if(tmp->attr==0x10){
            strcpy(tmp->extension,"Folder");
        }
        printf("%s\t",tmp->extension);
        printf("%d/",tmp->date);
        printf("%d/",tmp->month);
        printf("%d ",tmp->year);
        printf("%d:",tmp->hrs);
        printf("%d\t   ",tmp->min);
        if(tmp->size!=0){
            printf("%d Bytes\t",tmp->size);
        }
        if(tmp->longName){
            for(countByte = 0 ; countByte < 22 ; countByte+=2){
                printf("%c",tmp->longNameBuff[countByte]);
            }
        }
        printf("\n");
        tmp = tmp->pNext;
    }
    printf("\nSelect from 1 to %d, 0 to exit:",entryIndex-1);
}

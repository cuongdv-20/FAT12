#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "fat_fs.h"

void user(void){
    fatEntry *pHead;
    uint32_t pos;
    uint32_t prevLocation;
    uint8_t filePath[50];

    while(false==fat_read_boot_info(filePath)){
        printf("nhap ten : ");
        scanf("%s",&filePath);
    }
    prevLocation = fatBoot.rootLocation;
    pHead = fat_read_root_dir();
    fat_print_entry();
    while(1){
        printf("\n\n");
        scanf("\n%d",&pos);
        fatEntry* tmp = pHead;
        if(pos!=0){
            while(tmp!=NULL){
            if(tmp->entryIndex==pos){
               if(tmp->attr == 0x00){
                    fat_free_entry();
                    fat_read_file(tmp->address);
                    getch();
                    if(prevLocation==fatBoot.rootLocation){
                        pHead=fat_read_root_dir();
                        fat_print_entry();
                    }
                    else{
                        pHead=fat_read_sub_dir(prevLocation);
                        fat_print_entry();
                    }
                }
                else if(tmp->attr == 0x10){
                    fat_free_entry();
                    if(tmp->address==0){
                        pHead=fat_read_root_dir();
                        prevLocation=fatBoot.rootLocation;
                        fat_print_entry();
                    }
                    else{
                        prevLocation = tmp->address;
                        pHead=fat_read_sub_dir(tmp->address);
                        fat_print_entry();
                    }
                }
                break;
            }
            tmp = tmp->pNext;
        }
        }
        else break;
    }
    kmc_deinit();
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "HAL.h"

static FILE *fp;
static uint32_t bytesPerSector = 512;

bool kmc_init(uint8_t *filePath){
    fp = fopen(filePath,"rb");
    if(NULL==fp){
        return false;
    }
    return true;
}

void kmc_sector_size(uint32_t sectorSize){
    bytesPerSector = sectorSize;
}

void kmc_deinit(void){
    fclose(fp);
}

int32_t kmc_read_sector(uint32_t index, uint8_t *buff){
    int32_t countBytes=-1;

    if(fseek(fp, index*bytesPerSector, SEEK_SET)==0){
        countBytes = fread(buff, sizeof(uint8_t), bytesPerSector, fp);
    }
    return countBytes;
}

int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff){
    int32_t countBytes=-1;

    if(fseek(fp, index*bytesPerSector, SEEK_SET)==0){
        countBytes = fread(buff, sizeof(uint8_t), bytesPerSector*num, fp);
    }
    return countBytes;
}


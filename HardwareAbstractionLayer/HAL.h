#ifndef __HAL_H_
#define __HAL_H_

bool kmc_init(uint8_t *filePath);

void kmc_sector_size(uint32_t sectorSize);

void kmc_deinit(void);

int32_t kmc_read_sector(uint32_t index, uint8_t *buff);

int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff);

#endif

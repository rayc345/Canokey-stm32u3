// SPDX-License-Identifier: Apache-2.0
#include "main.h"
#include <fs.h>
#include <stdalign.h>
#include <string.h>

#define LOOKAHEAD_SIZE 16
#define WRITE_SIZE 8
#define READ_SIZE 4
#define FS_BASE (0x080F0000)
#define FLASH_ADDR(b, o) (FS_BASE + (b) * FLASH_PAGE_SIZE + (o))

static struct lfs_config config;
static alignas(8) uint8_t read_buffer[LFS_CACHE_SIZE];
static alignas(8) uint8_t prog_buffer[LFS_CACHE_SIZE];
static alignas(4) uint8_t lookahead_buffer[LOOKAHEAD_SIZE];

int block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
  UNUSED(c);
  // DBG_MSG("blk %d @ %p len %u buf %p\r\n", block, (void*)FLASH_ADDR(block, off), size, buffer);
  memcpy(buffer, (const void *)FLASH_ADDR(block, off), size);
  return 0;
}

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
 * @brief  Gets the bank of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The bank of a given address
 */
static uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;

  if (READ_BIT(FLASH->OPTR, FLASH_OPTR_SWAP_BANK) == 0)
  {
    /* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
    /* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }

  return bank;
}

static int program_space(uint32_t paddr, const void *buffer, lfs_size_t size)
{
  int ret = 0;
  uint32_t typ;
  for (lfs_size_t i = 0; size;)
  {
    // DBG_MSG("%d\n", i);
    // if (size >= 512) {
    //   typ = FLASH_TYPEPROGRAM_FAST;
    // } else if (size >= 256) {
    //   typ = FLASH_TYPEPROGRAM_FAST_AND_LAST;
    // } else {
    typ = FLASH_TYPEPROGRAM_DOUBLEWORD;
    // }
    if (HAL_FLASH_Program(typ, paddr + i, ((uint32_t)buffer + i)) != HAL_OK)
    {
      ret = LFS_ERR_CORRUPT;
      break;
    }
    if (typ == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
      i += 8;
      size -= 8;
    }
    else
    {
      i += 256;
      size -= 256;
    }
  }

  return ret;
}

int block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
  UNUSED(c);

  if (size % WRITE_SIZE != 0 || off % WRITE_SIZE != 0)
    return LFS_ERR_INVAL;

  uint32_t paddr = (uint32_t)FLASH_ADDR(block, off);
  int ret;

  // DBG_MSG("blk %d @ %p len %u buf %p\r\n", block, (void*)paddr, size, buffer);
  // for (size_t i = 0; i < size; i++)
  // {
  //   if(*(uint8_t*)(paddr+i) != 0xFF) {
  //     DBG_MSG("blank check: %p = %x\n", paddr+i, *(uint8_t*)(paddr+i));
  //   }
  // }

  HAL_FLASH_Unlock();
  ret = program_space(paddr, buffer, size);
  HAL_FLASH_Lock();

  // Invalidate cache
  // __HAL_FLASH_DATA_CACHE_DISABLE();
  // // __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
  // __HAL_FLASH_DATA_CACHE_RESET();
  // // __HAL_FLASH_INSTRUCTION_CACHE_RESET();
  // // __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  // __HAL_FLASH_DATA_CACHE_ENABLE();

  // DBG_MSG("verify %d\n", memcmp(buffer, (const void *)FLASH_ADDR(block, off), size));

  return ret;
}

int block_erase(const struct lfs_config *c, lfs_block_t block)
{
  /* Disable instruction cache prior to internal cacheable memory update */
  if (HAL_ICACHE_Disable() != HAL_OK)
  {
    Error_Handler();
  }

  UNUSED(c);
  int ret = 0;
  uint32_t PageError;
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks = GetBank(FLASH_ADDR(block, 0));
  EraseInitStruct.Page = GetPage(FLASH_ADDR(block, 0));
  EraseInitStruct.NbPages = 1;
  // DBG_MSG("block 0x%x\r\n", EraseInitStruct.Page);

  HAL_FLASH_Unlock();

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    ret = LFS_ERR_IO;
    // ERR_MSG("HAL_FLASHEx_Erase %#x failed\n", (unsigned int)PageError);
    goto erase_fail;
  }

  // Invalidate cache
  // __HAL_FLASH_DATA_CACHE_DISABLE();
  // // __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
  // __HAL_FLASH_DATA_CACHE_RESET();
  // // __HAL_FLASH_INSTRUCTION_CACHE_RESET();
  // // __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  // __HAL_FLASH_DATA_CACHE_ENABLE();

erase_fail:
  HAL_FLASH_Lock();
  // DBG_MSG("done\n");

  /* Re-enable instruction cache */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }

  return ret;
}

int block_sync(const struct lfs_config *c)
{
  UNUSED(c);
  return 0;
}

void littlefs_init()
{
  memset(&config, 0, sizeof(config));
  config.read = block_read;
  config.prog = block_prog;
  config.erase = block_erase;
  config.sync = block_sync;
  config.read_size = READ_SIZE;
  config.prog_size = WRITE_SIZE;
  config.block_size = FLASH_PAGE_SIZE;
  config.block_count = 16;
  config.block_cycles = 100000;
  config.cache_size = LFS_CACHE_SIZE;
  config.lookahead_size = LOOKAHEAD_SIZE;
  config.read_buffer = read_buffer;
  config.prog_buffer = prog_buffer;
  config.lookahead_buffer = lookahead_buffer;
  // DBG_MSG("Flash base %p, %u blocks (%u bytes)\r\n", FS_BASE, config.block_count, FLASH_PAGE_SIZE);

  int err;
  for (int retry = 0; retry < 3; retry++)
  {
    err = fs_mount(&config);
    if (!err)
      return;
  }
  // should happen for the first boot
  // DBG_MSG("Formating data area...\r\n");
  fs_format(&config);
  err = fs_mount(&config);
  if (err)
  {
    // ERR_MSG("Failed to mount FS after formating\r\n");
    for (;;)
      ;
  }
}

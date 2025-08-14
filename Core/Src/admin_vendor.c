// SPDX-License-Identifier: Apache-2.0

#include "main.h"
#include <device.h>
#include <admin.h>

#define VENDOR_NFC_SET 0x01
#define VENDOR_NFC_GET 0x02
#define VENDOR_STACK_TEST 0x03

// override functions defined in admin.c

int admin_vendor_hw_variant(const CAPDU *capdu, RAPDU *rapdu)
{
  UNUSED(capdu);

  const char *s = "Canokey-STM32U385KG";
  size_t len = strlen(s);
  memcpy(RDATA, s, len);
  LL = len;
  if (LL > LE)
    LL = LE;

  return 0;
}

int admin_vendor_version(const CAPDU *capdu, RAPDU *rapdu)
{
  UNUSED(capdu);
  const char *GIT_REV = "3.0.3";
  size_t len = strlen(GIT_REV);
  memcpy(RDATA, GIT_REV, len);
  memcpy(RDATA + len, "-O", 2);
  LL = len + 2;
  if (LL > LE)
    LL = LE;

  return 0;
}

int admin_vendor_nfc_enable(const CAPDU *capdu, RAPDU *rapdu, bool pin_validated)
{
  if (P1 != 0x00 && P1 != 0x01)
    EXCEPT(SW_WRONG_P1P2);
  if (P2 != 0x00)
    EXCEPT(SW_WRONG_P1P2);
  if (LC != 0x00)
    EXCEPT(SW_WRONG_LENGTH);

  if (P1 == 0x01 && !pin_validated)
    EXCEPT(SW_SECURITY_STATUS_NOT_SATISFIED);
  if (P1 == 0x00)
  {
    RDATA[0] = check_is_nfc_en();
    LL = 1;
    return 0;
  }

  return 0;
}

int admin_vendor_specific(const CAPDU *capdu, RAPDU *rapdu)
{
  uint16_t addr;

  switch (P1)
  {
  case VENDOR_NFC_SET:
#define NFC_SET_MAX_LEN 18
    if (LC <= 2 || LC > NFC_SET_MAX_LEN)
      EXCEPT(SW_WRONG_LENGTH);

    addr = (DATA[0] << 8) | DATA[1];
    if (addr < 0x000C || addr > 0x03CF)
      EXCEPT(SW_WRONG_DATA);

    fm_write_eeprom(addr, DATA + 2, LC - 2);
    if (P2 == 1)
    { // verification enabled
      uint8_t *readback_buf = DATA + NFC_SET_MAX_LEN;
      device_delay(10);
      fm_read_eeprom(addr, readback_buf, LC - 2);
      if (memcmp(DATA + 2, readback_buf, LC - 2))
        EXCEPT(SW_CHECKING_ERROR);
    }
    break;

  case VENDOR_NFC_GET:
    if (LC != 2)
      EXCEPT(SW_WRONG_LENGTH);

    addr = (DATA[0] << 8) | DATA[1];
    if (addr > 0x03CF)
      EXCEPT(SW_WRONG_DATA);

    fm_read_eeprom(addr, RDATA, LE);
    LL = LE;
    break;


  default:
    EXCEPT(SW_WRONG_P1P2);
  }

  return 0;
}

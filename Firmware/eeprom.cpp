//! @file
//! @date Jun 20, 2019
//! @author Marek Běl

#include "eeprom.h"
#include "Marlin.h"

#include <avr/eeprom.h>
#include <stdint.h>


#include "language.h"

#if 0
template <typename T>
static T eeprom_read(T *address);

template<>
char eeprom_read<char>(char *address)
{
    return eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
}
#endif

template <typename T>
static void eeprom_write(T *address, T value);

template<>
void eeprom_write<char>(char *addres, char value)
{
    eeprom_write_byte(reinterpret_cast<uint8_t*>(addres), static_cast<uint8_t>(value));
}


template <typename T>
static bool eeprom_is_uninitialized(T *address);

template <>
bool eeprom_is_uninitialized<char>(char *address)
{
    return (0xff == eeprom_read_byte(reinterpret_cast<uint8_t*>(address)));
}

bool is_sheet_initialized(){
  return (0xffff != eeprom_read_word(reinterpret_cast<uint16_t*>(&(EEPROM_Sheets_base->
  s[eeprom_read_byte(&(EEPROM_Sheets_base->active_sheet))].z_offset))));
}

void eeprom_init()
{
    if (eeprom_read_byte((uint8_t*)EEPROM_POWER_COUNT) == 0xff) eeprom_write_byte((uint8_t*)EEPROM_POWER_COUNT, 0);
    if (eeprom_read_byte((uint8_t*)EEPROM_CRASH_COUNT_X) == 0xff) eeprom_write_byte((uint8_t*)EEPROM_CRASH_COUNT_X, 0);
    if (eeprom_read_byte((uint8_t*)EEPROM_CRASH_COUNT_Y) == 0xff) eeprom_write_byte((uint8_t*)EEPROM_CRASH_COUNT_Y, 0);
    if (eeprom_read_byte((uint8_t*)EEPROM_FERROR_COUNT) == 0xff) eeprom_write_byte((uint8_t*)EEPROM_FERROR_COUNT, 0);
    if (eeprom_read_word((uint16_t*)EEPROM_POWER_COUNT_TOT) == 0xffff) eeprom_write_word((uint16_t*)EEPROM_POWER_COUNT_TOT, 0);
    if (eeprom_read_word((uint16_t*)EEPROM_CRASH_COUNT_X_TOT) == 0xffff) eeprom_write_word((uint16_t*)EEPROM_CRASH_COUNT_X_TOT, 0);
    if (eeprom_read_word((uint16_t*)EEPROM_CRASH_COUNT_Y_TOT) == 0xffff) eeprom_write_word((uint16_t*)EEPROM_CRASH_COUNT_Y_TOT, 0);
    if (eeprom_read_word((uint16_t*)EEPROM_FERROR_COUNT_TOT) == 0xffff) eeprom_write_word((uint16_t*)EEPROM_FERROR_COUNT_TOT, 0);

    if (eeprom_read_word((uint16_t*)EEPROM_MMU_FAIL_TOT) == 0xffff) eeprom_update_word((uint16_t *)EEPROM_MMU_FAIL_TOT, 0);
    if (eeprom_read_word((uint16_t*)EEPROM_MMU_LOAD_FAIL_TOT) == 0xffff) eeprom_update_word((uint16_t *)EEPROM_MMU_LOAD_FAIL_TOT, 0);
    if (eeprom_read_byte((uint8_t*)EEPROM_MMU_FAIL) == 0xff) eeprom_update_byte((uint8_t *)EEPROM_MMU_FAIL, 0);
    if (eeprom_read_byte((uint8_t*)EEPROM_MMU_LOAD_FAIL) == 0xff) eeprom_update_byte((uint8_t *)EEPROM_MMU_LOAD_FAIL, 0);
    if (eeprom_read_byte(&(EEPROM_Sheets_base->active_sheet)) == 0xff) eeprom_update_byte(&(EEPROM_Sheets_base->active_sheet), 0);
    for (uint_least8_t i = 0; i < (sizeof(Sheets::s)/sizeof(Sheets::s[0])); ++i)
    {
        bool is_uninitialized = true;
        for (uint_least8_t j = 0; j < (sizeof(Sheet::name)/sizeof(Sheet::name[0])); ++j)
        {
            if (!eeprom_is_uninitialized(&(EEPROM_Sheets_base->s[i].name[j]))) is_uninitialized = false;
        }
        if(is_uninitialized)
        {
            eeprom_write(&(EEPROM_Sheets_base->s[i].name[0]), static_cast<char>(i + '1'));
            eeprom_write(&(EEPROM_Sheets_base->s[i].name[1]), '\0');
        }
    }
    check_babystep();
}





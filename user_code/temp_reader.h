#ifndef __TEMP_READER_H
#define __TEMP_READER_H

#include "main.h"

void    temp_init(void);
void    temp_read_all(float *temp, uint8_t *tc_ok);

#endif


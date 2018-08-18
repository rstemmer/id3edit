#ifndef CRC32_H
#define CRC32_H

#include <stdio.h>

int CRC32FromFile(FILE *file, long from, long to, unsigned long *crc);


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


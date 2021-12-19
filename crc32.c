#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <crc32.h>
#include <zlib.h>


int CRC32FromFile(FILE *file, long from, long to, unsigned long *crc)
{
    if(from >= to)
    {
        fprintf(stderr, "Invalid span of data for CRC: from = %li; to = %li!\n", from, to);
        return -1;
    }

    // Create Buffer
    unsigned char *buffer;
    size_t buffersize = to-from;

    buffer = malloc(buffersize * sizeof(unsigned char));
    if(buffer == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return -1;
    }

    // Go to begin of relevant data
    int error;
    error = fseek(file, from, SEEK_SET);
    if(error != 0)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - Setting correct file position failed!\n");
        free(buffer);
        return -1;
    }

    // Read File
    size_t bytesread;
    bytesread = fread(buffer, 1, buffersize, file);
    if(bytesread != buffersize)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - Only %zu of %zu bytes read from file!\n", bytesread, buffersize);
        free(buffer);
        return -1;
    }

    // Calculate CRC32
    if(crc == NULL)
    {
        free(buffer);
        return 0;
    }

    *crc = crc32(0x00000000, buffer, buffersize);

    // Clean up
    free(buffer);
    return 0;    
}




// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


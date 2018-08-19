#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>
#include <rawfile.h>

#ifdef DEBUG
#include <printhex.h>
#endif

int RAWFILE_Read(const char *path, void **rawdata, size_t *size)
{
    FILE *file;
    // open file
    file = fopen(path, "rb");
    if(file == NULL)
    {
        fprintf(stderr, "Opening \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        return RAWFILEERROR_PATHERROR;
    }

    // get size
    long filesize;
    fseek(file, 0, SEEK_END); // go to the end of file
    filesize  =  ftell(file); // get position in file
    fseek(file, 0, SEEK_SET); // go to the begin of file
    if(filesize < 0)
    {
        fprintf(stderr, "Determine size of file \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        return RAWFILEERROR_FATAL;
    }

    // allocate memory
    unsigned char *data;
    data = malloc(filesize);
    if(data == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return RAWFILEERROR_FATAL;
    }

    // read data
    fread(data, 1, filesize, file);

    // close file
    fclose(file);
    file = NULL;

    // return everything
    if(rawdata)
        *rawdata = data;
    if(size)
        *size = (size_t)filesize;

    return RAWFILEERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int RAWFILE_Write(const char *path, const void *rawdata, size_t size)
{
    FILE *file;
    // open file
    file = fopen(path, "wb");
    if(file == NULL)
    {
        fprintf(stderr, "Opening \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        return RAWFILEERROR_PATHERROR;
    }

    // read data
    fwrite(rawdata, 1, size, file);

    // close file
    fclose(file);
    file = NULL;

    return RAWFILEERROR_NOERROR;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


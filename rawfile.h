#ifndef RAWFILE_H
#define RAWFILE_H
    
#define RAWFILEERROR_NOERROR     0
#define RAWFILEERROR_FATAL      -100
#define RAWFILEERROR_PATHERROR  -101

int RAWFILE_Read( const char *path,       void **rawdata, size_t *size);
int RAWFILE_Write(const char *path, const void  *rawdata, size_t  size);

#endif



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <id3v2.h>
#include <id3v2frame.h>
#include <encoding/text.h>
#include <encoding/crc.h>
#include <rawfile.h>
#include <printhex.h>
#include <stdbool.h>



void PrintTest(const char *name)
{
    printf("\e[1;37m[\e[s ]\e[1;34m %s\e[0m ", name);
    fflush(stdout);
}
void PrintTestSucceeded()
{
    printf("\e[u\e[1;32m✔\e[0m\n");
}
void PrintTestFailed()
{
    printf("\e[u\e[1;31m✘\e[0m\n");
}



void CheckDecoderResult(char *text, size_t actualsize)
{
    if(strncmp(text, "Täst 😈", 20) == 0 && actualsize == strlen("Täst 😈") + 1)
        PrintTestSucceeded();
    else
        PrintTestFailed();
}
void CheckEncoderResult(void *result, const void *expected, size_t expectedsize, size_t actualsize)
{
    if(memcmp(result, expected, expectedsize) == 0 && actualsize == expectedsize)
        PrintTestSucceeded();
    else
        PrintTestFailed();
}



int main(int argc, char *argv[])
{
    // Test string: "Täst 😈"
    const unsigned short utf16bombe[] = {0xFFFE, 0x5400, 0xe400, 0x7300, 0x7400, 0x2000, 0x3dd8, 0x08de};
    const unsigned short utf16bomle[] = {0xFEFF, 0x0054, 0x00e4, 0x0073, 0x0074, 0x0020, 0xd83d, 0xde08};
    const unsigned short utf16be[]    = {0x5400, 0xe400, 0x7300, 0x7400, 0x2000, 0x3dd8, 0x08de};
    const unsigned char  utf8[]       = {0x54, 0xc3, 0xa4, 0x73, 0x74, 0x20, 0xf0, 0x9f, 0x98, 0x88};
    const unsigned char  iso8859_1[]  = {0x54, 0xe4, 0x73, 0x74};
    int error;


    printf("\e[1;37mTest set: \e[1;36mEncode/Decode\e[0m\n");

    PrintTest("Decode unsynchronized CRC");
    {
    unsigned char enccrc[5] = {0x0F, 0x7F, 0x3C, 0x5A, 0x7F};
    unsigned int  deccrc;
    unsigned int  expectedcrc = 0x7F2DEFFF;
    // 00001111 01111111 00111100 01011010 01111111
    // 11111111 11101111 00101101 01111111 _BE
    ID3V2_DecodeCRC(&deccrc, enccrc);
    CheckEncoderResult(&deccrc, &expectedcrc, 4, 4);
    }

    PrintTest("Encode unsynchronized CRC");
    {
    unsigned char expectedcrc[5] = {0x0F, 0x7F, 0x3C, 0x5A, 0x7F};
    unsigned char enccrc[5];
    unsigned int  crc = 0x7F2DEFFF;
    ID3V2_EncodeCRC(crc, enccrc);
    CheckEncoderResult(&enccrc, &expectedcrc, 5, 5);
    }


    // Decode
    char text[1024];
    size_t actualsize;

    PrintTest("Decode UTF-16BE with BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BOM, (void*)utf16bombe, 8*2, text, 1024, &actualsize);
    CheckDecoderResult(text, actualsize);

    PrintTest("Decode UTF-16LE with BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BOM, (void*)utf16bomle, 8*2, text, 1024, &actualsize);
    CheckDecoderResult(text, actualsize);

    PrintTest("Decode UTF-16BE without BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BE,  (void*)utf16be,    7*2, text, 1024, &actualsize);
    CheckDecoderResult(text, actualsize);

    PrintTest("Decode UTF-8");
    error = Decode(ID3V2TEXTENCODING_UTF8,      (void*)utf8,       10,  text, 1024, &actualsize);
    CheckDecoderResult(text, actualsize);

    PrintTest("Decode ISO 8859-1");
    error = Decode(ID3V2TEXTENCODING_ISO8859_1, (void*)iso8859_1,  4,   text, 1024, &actualsize);
    if(strncmp(text, "Täst", 6) == 0 && actualsize == 6)
        PrintTestSucceeded();
    else
        PrintTestFailed();

    // Encode
    char code[1024];

    PrintTest("Encode UTF-16LE with BOM");
    error = Encode(ID3V2TEXTENCODING_UTF16_BOM, "Täst 😈", strlen("Täst 😈"), code, 1024, &actualsize);
    CheckEncoderResult(code, utf16bomle, 8*2, actualsize);

    PrintTest("Encode UTF-16BE without BOM");
    error = Encode(ID3V2TEXTENCODING_UTF16_BE, "Täst 😈", strlen("Täst 😈"), code, 1024, &actualsize);
    CheckEncoderResult(code, utf16be, 7*2, actualsize);

    PrintTest("Encode UTF-8");
    error = Encode(ID3V2TEXTENCODING_UTF8, "Täst 😈", strlen("Täst 😈"), code, 1024, &actualsize);
    CheckEncoderResult(code, utf8, 10, actualsize);

    PrintTest("Encode ISO 8859-1");
    error = Encode(ID3V2TEXTENCODING_ISO8859_1, "Täst", strlen("Täst"), code, 1024, &actualsize);
    CheckEncoderResult(code, iso8859_1, 4, actualsize);
    

    return EXIT_SUCCESS;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


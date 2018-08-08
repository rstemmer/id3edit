#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <id3v2.h>
#include <id3v2frame.h>
#include <encoding.h>
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



void CheckDecoderResult(char *text)
{
    if(strncmp(text, "Täst 😈", 20) == 0)
        PrintTestSucceeded();
    else
        PrintTestFailed();
}
void CheckEncoderResult(void *result, const void *expected, size_t numbytes)
{
    if(memcmp(result, expected, numbytes) == 0)
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

    // Decode
    char text[1024];

    PrintTest("Decode UTF-16BE with BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BOM, (void*)utf16bombe, 8*2, text, 1024);
    CheckDecoderResult(text);

    PrintTest("Decode UTF-16LE with BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BOM, (void*)utf16bomle, 8*2, text, 1024);
    CheckDecoderResult(text);

    PrintTest("Decode UTF-16BE without BOM");
    error = Decode(ID3V2TEXTENCODING_UTF16_BE,  (void*)utf16be,    7*2, text, 1024);
    CheckDecoderResult(text);

    PrintTest("Decode UTF-8");
    error = Decode(ID3V2TEXTENCODING_UTF8,      (void*)utf8,       10,  text, 1024);
    CheckDecoderResult(text);

    PrintTest("Decode ISO 8859-1");
    error = Decode(ID3V2TEXTENCODING_ISO8859_1, (void*)iso8859_1,  4,   text, 1024);
    if(strncmp(text, "Täst", 5) == 0)
        PrintTestSucceeded();
    else
        PrintTestFailed();

    // Encode
    char code[1024];

    PrintTest("Encode UTF-16LE with BOM");
    error = Encode(ID3V2TEXTENCODING_UTF16_BOM, "Täst 😈", strlen("Täst 😈") + 1, code, 1024);
    CheckEncoderResult(code, utf16bomle, 8*2);

    PrintTest("Encode UTF-16BE without BOM");
    error = Encode(ID3V2TEXTENCODING_UTF16_BE, "Täst 😈", strlen("Täst 😈") + 1, code, 1024);
    CheckEncoderResult(code, utf16be, 7*2);

    PrintTest("Encode UTF-8");
    error = Encode(ID3V2TEXTENCODING_UTF8, "Täst 😈", strlen("Täst 😈") + 1, code, 1024);
    CheckEncoderResult(code, utf8, 10);

    PrintTest("Encode ISO 8859-1");
    error = Encode(ID3V2TEXTENCODING_ISO8859_1, "Täst", strlen("Täst") + 1, code, 1024);
    CheckEncoderResult(code, iso8859_1, 4);
    

    return EXIT_SUCCESS;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


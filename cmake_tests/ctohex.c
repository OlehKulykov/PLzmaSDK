
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char _original_file[1024] = { 0 };
char _lc_file[1024] = { 0 };

void write_str(FILE * f, const char * str) {
    fwrite(str, strlen(str), 1, f);
}

int main(int argc, const char * argv[]) {
    FILE * inFile = NULL;
    for (int i = 0; i < argc; i++) {
        switch (i) {
            case 1: {
                const char * s1 = argv[i];
                inFile = fopen(s1, "rb");
                char * s2 = _lc_file, * s3 = _original_file;
                while (*s1) {
                    if (*s1 == '.' || *s1 == ' ') {
                        *s2++ = '_';
                    } else {
                        *s2++ = (char)tolower(*s1);
                    }
                    *s3++ = *s1++;
                }
                break;
            }
                
            default:
                break;
        }
    }
    
    char str1[1024];
    char str2[1024];
    sprintf(str1, "file__%s.h", _lc_file);
    fseek(inFile, 0, SEEK_END);
    long long inFileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    FILE * outFile = fopen(str1, "w+b");
    
    char sizeVarStr[128];
    sprintf(sizeVarStr, "FILE__%s_SIZE", _lc_file);
    
    sprintf(str2, "// %s\n", _original_file);
    write_str(outFile, str2);
    
    sprintf(str1, "#ifndef %s\n", sizeVarStr);
    write_str(outFile, str1);
    
    sprintf(str1, "#define %s %lli\n", sizeVarStr, inFileSize);
    write_str(outFile, str1);
    
    write_str(outFile, "#if defined(__cplusplus)\n");
    
    sprintf(str2, "FILE__%s", _lc_file);
    sprintf(str1, "extern \"C\" unsigned char %s[%s];\nextern \"C\" unsigned char * %s_PTR;\n", str2, sizeVarStr, str2);
    write_str(outFile, str1);
    
    sprintf(str1, "extern \"C\" bool %s_write_to_path(const char *);\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "#else\n");
    
    sprintf(str2, "FILE__%s", _lc_file);
    sprintf(str1, "#include <stdbool.h>\nextern unsigned char %s[%s];\nextern unsigned char * %s_PTR;\n", str2, sizeVarStr, str2);
    write_str(outFile, str1);

    sprintf(str1, "extern bool %s_write_to_path(const char *);\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "#endif\n");
    
    write_str(outFile, "#else\n");
    sprintf(str1, "unsigned char %s[%s]={\n", str2, sizeVarStr);
    write_str(outFile, str1);
    
    uint8_t buff[16];
    unsigned long r;
    int first = 1;
    while ( (r = fread(buff, 1, 16, inFile)) > 0) {
        const char * format;
        if (r < 16) {
            if (!first) {
                fwrite(",\n", 2, 1, outFile);
            }
            first = 1;
            for (unsigned long i = 0; i < r; i++) {
                if (first) {
                    format = "0x%02x";
                } else {
                    format = ",0x%02x";
                }
                int strLen = sprintf(str1, format, buff[i]);
                fwrite(str1, strLen, 1, outFile);
                first = 0;
            }
            first = 0;
        } else {
            if (first) {
                format = "0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x";
            } else {
                format = ",\n0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x";
            }
            int strLen = sprintf(str1,
                                 format,
                                 buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],
                                 buff[8],buff[9],buff[10],buff[11],buff[12],buff[13],buff[14],buff[15]);
            fwrite(str1, strLen, 1, outFile);
        }
        first = 0;
    }
    
    sprintf(str2, "FILE__%s", _lc_file);
    sprintf(str1, "};\nunsigned char * %s_PTR=%s;\n", str2, str2);
    write_str(outFile, str1);
    
    sprintf(str1, "bool %s_write_to_path(const char * path) {\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "    FILE * f = fopen(path, \"w+b\");\n    if (f) {\n");
    
    sprintf(str1, "        const unsigned int w = (unsigned int)fwrite(%s_PTR, 1, %s_SIZE, f);\n", str2, str2);
    write_str(outFile, str1);
    
    write_str(outFile, "        fclose(f);\n");
    
    sprintf(str1, "        return w == %s_SIZE;\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "    }\n    return false;\n}\n#endif\n\n");
    
    fclose(inFile);
    fclose(outFile);
    printf("Done\n");
    return 0;
}

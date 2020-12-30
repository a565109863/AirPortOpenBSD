#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char *argv[]) {
 
        FILE *fp;
        int i, c;
        char *name;
 
        if (argc < 2) {
                printf("Usage: %s input_file [array_name] [> output_file]\n", argv[0]);
                return 1;
        }
 
        fp = fopen(argv[1], "rb");
        if (fp == NULL) {
                printf("%s: fopen(%s) failed", argv[0], argv[1]);
                return 1;
        }
 
        if (argc >= 3) name=argv[2];
        else name="filedata";
 
        printf("const unsigned char %s[] = {", name);
        for (i=0;;i++) {
                if ((c = fgetc(fp)) == EOF) break;
                if (i != 0) printf(",");
                if ((i % 12) == 0) printf("\n\t");
                printf("0x%.2X", (unsigned char)c);
        }
        printf("\n};\n");
 
        fclose(fp);
        return 0;
 
}
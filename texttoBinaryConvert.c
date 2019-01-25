#include <stdio.h>
#include <string.h>

  int main() {
        
        FILE *textFile, *binaryFile;
        char ch; 
        int val;

        
        /* input Text file */
        textFile = fopen("inputTextData.txt", "r");
        /* open output file in binary  */
        binaryFile = fopen("OutputFile.bin", "wb");

        /* file error handle */
        if (!textFile) 
        {
                printf("input file opening issues!!\n");
                return 0;
        }


        /* file error handle */
        if (!binaryFile) {
                printf(" output file opening issues!!\n");
                return 0;
        }

        /*  Read input file and store in binary form 
         */
        while (!feof(textFile)) 
         {
                /* reading input file by  one byte of data  at a time */
                fread(&ch, sizeof(char), 1, textFile);
                /* character to ascii integer conversion   */
                val = ch;
                /* writing 4 byte of data to the output file */
                fwrite(&val, sizeof(int), 1, binaryFile);
                
        }

     
        fclose(textFile);
        fclose(binaryFile);
        return 0;
  }

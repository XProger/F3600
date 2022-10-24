#include <stdio.h>

unsigned int swap32(unsigned int x)
{
    return ((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24);
}

int main()
{
    int zero = 0;
    
    char buffer[32 * 1024];
    
    FILE* fout = fopen("wipeout.sdx", "wb");
    
    for (int i = 0; i < 30; i++)
    {
        char path[16];
        sprintf(path, "%d.aifc", i);
        
        FILE* fin = fopen(path, "rb");
        fseek(fin, 0, SEEK_END);
        int size = ftell(fin);
        fseek(fin, 0, SEEK_SET);
        fread(buffer, size, 1, fin);
        fclose(fin);

        size = swap32(*(int*)(buffer + 0x72));
        printf("size = %d\n", size);
        
        fwrite(buffer + 0x72, size + 4, 1, fout);
        while (size & 3)
        {
            fwrite(&zero, 1, 1, fout); 
            size++;
        }
    }
    
    fclose(fout);
}
#include <stdio.h>

int Fileio_OpenFile(char *pFileName)
{
  FILE *pFile;
  
  pFile = fopen(pFileName, "w");
  printf("File created %p", pFile); fflush(stdout);
  
    
  if (pFile) 
  {
    fwrite("abc", 3, 1, pFile);
    fclose(pFile);
    return (1);
  }
  else 
    return (0);
}


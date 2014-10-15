#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "Buffer.h"

void printErr(const char* format, ...)
{
  // http://stackoverflow.com/questions/1056411/how-to-pass-variable-number-of-arguments-to-printf-sprintf
  va_list argptr;
  va_start(argptr, format);
  vfprintf(stderr, format, argptr);
  va_end(argptr);
  fprintf(stderr, ": %s\n", strerror(errno));
}

FILE* openFileReadOnly(const char* name)
{
  FILE* f = nullptr;
  if ((f = fopen(name, "r")) == nullptr) {
    printErr("Could not open '%s'", name);
  }
  return f;
}

int fileSize(FILE* f)
{
  if (f == nullptr)
    return 0;

  fseek(f, 0, SEEK_END);
  int fileSize = ftell(f);
  rewind(f);
  return fileSize;
}

void readFile(const char *name)
{
  unsigned char bufferSize = 16;
  unsigned char buffer[bufferSize];
  FILE* f = nullptr;
  if ((f = openFileReadOnly(name))) {
    int size = fileSize(f);
    printf("file size: %d\n", size);
    for (int i = 0; i < size; i += bufferSize) {
      int numRead = fread(buffer, 1, bufferSize, f);
      if (numRead < 1) {
        printErr("Could not read from file '%s'", name);
        break;
      }
      // handles end of file
      if (numRead < bufferSize) {
        memset(&buffer[numRead], 0, bufferSize-numRead);
      }
      printf(">>>%.*s<<<\n", numRead, buffer);
    }
    fclose(f);
  }
}

char Buffer::nextChar()
{
  return 0;
}

int Buffer::offset()
{
  return 0;
}

char* Buffer::region(const unsigned int offset)
{
  return nullptr;
}

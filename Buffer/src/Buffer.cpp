#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include "Buffer.h"

static void printErr(const char* format, ...)
{
  // http://stackoverflow.com/questions/1056411/how-to-pass-variable-number-of-arguments-to-printf-sprintf
  va_list argptr;
  va_start(argptr, format);
  vfprintf(stderr, format, argptr);
  va_end(argptr);
  fprintf(stderr, ": %s\n", strerror(errno));
}

static int openFileReadOnly(const char* name)
{
  int fd;
  if ((fd = open(name, O_RDONLY | O_DIRECT)) < 0) {
    printErr("Could not open '%s'", name);
  }
  return fd;
}

/**
 * Allocates memory of `alignLen` size and aligns it by `alignLen`.
 */
static void mem(char **buf, unsigned alignLen)
{
  int err = posix_memalign((void **)buf, alignLen, alignLen);
  memset(*buf, 0, alignLen);
  if (err != 0) {
    errno = err;
    printErr("Could not allocate memory");
  }
}

unsigned const Buffer::BUFFER_SIZE = 1024;

Buffer::Buffer(const char *const fileName):
  fileName(fileName),
  off(0),
  offInFile(0),
  eofReached(false),
  fileDescriptor(0),
  lastChar(0),
  line(1),
  column(1)
{
  mem(&curBuffer, BUFFER_SIZE);
  mem(&prevBuffer, BUFFER_SIZE);
  readNext();
}

Buffer::~Buffer()
{
  if (fileDescriptor > 0 && close(fileDescriptor) < 0)
    printErr("Could not close file '%s'", fileName);
  free(prevBuffer);
  free(curBuffer);
}

void Buffer::readNext()
{
  if (eofReached)
    return;
  if (fileDescriptor == 0)
    fileDescriptor = openFileReadOnly(fileName);
  if (fileDescriptor < 0) {
    eofReached = true;
    return;
  }

  // swap buffer pointer
  auto tmp = prevBuffer;
  prevBuffer = curBuffer;
  curBuffer = tmp;

  // read data from file
  errno = 0;
  int sizeRead = read(fileDescriptor, curBuffer, BUFFER_SIZE);
  if (sizeRead == (int) BUFFER_SIZE) {
    offInFile += sizeRead;
  }
  else {
    eofReached = true;
    if (sizeRead < 0)
      printErr("Could not read from file '%s'", fileName);
    else
      offInFile += sizeRead;
  }
}

char Buffer::nextChar()
{
  ++off;
  ++column;
  if (off >= offInFile)
    readNext();

  if (lastChar == '\n') {
    ++line;
    column = 1;
  }
  lastChar = currentChar();
  return lastChar;
}

char Buffer::currentChar()
{
  return curBuffer[off%BUFFER_SIZE];
}

unsigned Buffer::currentLine()
{
  return line;
}

unsigned Buffer::currentColumn()
{
  return column;
}

unsigned Buffer::offset()
{
  return off;
}

char* Buffer::range(char *buffer, const unsigned start, const unsigned len)
{
  auto bufEnd = offInFile%BUFFER_SIZE == 0
    ? offInFile-BUFFER_SIZE
    : offInFile-(offInFile%BUFFER_SIZE);

  // we have to copy only from the last buffer
  if (start > bufEnd)
    strncpy(buffer, curBuffer+start%BUFFER_SIZE, len);
  // we also need to copy from the previous buffer
  else {
    auto readPrev = bufEnd-start;
    strncpy(buffer, prevBuffer+BUFFER_SIZE-readPrev, readPrev);
    strncpy(buffer+readPrev, curBuffer, len-readPrev);
  }
  buffer[len] = 0;
  return buffer;
}

void Buffer::setOffset(const unsigned offset)
{
  int change = abs((int)offset-(int)off);
  if (change < 0) {
    errno = 0;
    printErr("It is not allowed to decrease the offset.");
  } else {
    column += change;
    off += change;

    if (off >= offInFile)
      readNext();
    lastChar = curBuffer[off%BUFFER_SIZE];
  }
}

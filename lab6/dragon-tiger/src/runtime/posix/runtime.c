#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

char c;

__attribute__((noreturn))
static void error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void __print_err(const char *s) {
  fprintf(stderr, "%s", s);
}

void __print(const char *s) {
  fprintf(stdout, "%s", s);
}

void __print_int(const int32_t i) {
  fprintf(stdout, "%d", i);
}

void __flush(void) {
  fflush(stdout);
}

const char *__getchar(void) {
  char c = getchar();
  char * s;
  if (c == EOF){
    s = (char *) malloc(sizeof(char));
    s[0] = '\0';
  }
  else{
    s = (char *) malloc(2*sizeof(char));
    s[0] = c;
    s[1] = '\0';
  }
  return s;
}

int32_t __ord(const char *s) {
  unsigned char a = s[0];
  if (s[0] == '\0')
    return -1;
  return a;
}

const char *__chr(int32_t i) {
  char * s;
  if ( (i <  0) || (i > 255)){
    exit(EXIT_FAILURE);
  }
  else {
    if (i == 0){
      s = (char *) malloc(sizeof(char));
      s[0] = '\0';
    }
    else{
      s = (char *) malloc(2*sizeof(char));
      s[0] = i;
      s[1] = '\0';
    }
  }
  return s;
}

int32_t __size(const char *s) {
  int size = 0;
  while (s[size] != '\0')
    size++;
  return size;
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  error("UNIMPLEMENTED __substring");
}

const char *__concat(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __concat");
}

int32_t __strcmp(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __strcmp");
}

int32_t __streq(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __streq");
}

int32_t __not(int32_t i) {
  error("UNIMPLEMENTED __not");
}

void __exit(int32_t c) {
  exit(c);
}

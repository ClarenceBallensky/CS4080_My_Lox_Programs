//> Chunks of Bytecode main-c
//> Scanning on Demand main-includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//< Scanning on Demand main-includes
#include "common.h"
//> main-include-chunk
#include "chunk.h"
//< main-include-chunk
//> main-include-debug
#include "debug.h"
//< main-include-debug
//> A Virtual Machine main-include-vm
#include "vm.h"
//< A Virtual Machine main-include-vm

ObjString* copyStringLegacy(const char* chars, int length);

static void makeSmallString(int value, int length, char* buffer) {
  for (int i = length - 1; i >= 0; i--) {
    buffer[i] = 'a' + (value % 26);
    value /= 26;
  }
  buffer[length] = '\0';
}

static double benchmarkStringPath(bool legacy, int count, int length) {
  char buffer[32];
  clock_t start = clock();

  for (int i = 0; i < count; i++) {
    makeSmallString(i, length, buffer);
    ObjString* string = legacy ? copyStringLegacy(buffer, length)
                              : copyString(buffer, length);
    push(OBJ_VAL(string));
    pop();
  }

  return (double)(clock() - start) / CLOCKS_PER_SEC;
}

static void runBenchmarks(void) {
  const int count = 100000;
  const int smallLen = 4;
  const int mediumLen = 16;

  printf("=== clox string allocation benchmark ===\n");

  initVM();
  double legacySmall = benchmarkStringPath(true, count, smallLen);
  freeVM();

  initVM();
  double optimizedSmall = benchmarkStringPath(false, count, smallLen);
  freeVM();

  initVM();
  double legacyMedium = benchmarkStringPath(true, count, mediumLen);
  freeVM();

  initVM();
  double optimizedMedium = benchmarkStringPath(false, count, mediumLen);
  freeVM();

  printf("legacy small (%d chars): %f sec\n", smallLen, legacySmall);
  printf("optimized small (%d chars): %f sec\n", smallLen, optimizedSmall);
  printf("legacy medium (%d chars): %f sec\n", mediumLen, legacyMedium);
  printf("optimized medium (%d chars): %f sec\n", mediumLen, optimizedMedium);
  printf("speedup small: %.2fx\n", legacySmall / optimizedSmall);
  printf("speedup medium: %.2fx\n", legacyMedium / optimizedMedium);
}

//> Scanning on Demand repl

static void repl() {
  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }
}
//< Scanning on Demand repl
//> Scanning on Demand read-file
static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
//> no-file
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }
//< no-file

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
//> no-buffer
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }
  
//< no-buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
//> no-read
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
//< no-read
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}
//< Scanning on Demand read-file
//> Scanning on Demand run-file
static void runFile(const char* path) {
  char* source = readFile(path);
  InterpretResult result = interpret(source);
  free(source); // [owner]

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}
//< Scanning on Demand run-file

int main(int argc, const char* argv[]) {
//> A Virtual Machine main-init-vm
  initVM();

//< A Virtual Machine main-init-vm
/* Chunks of Bytecode main-chunk < Scanning on Demand args
  Chunk chunk;
  initChunk(&chunk);
*/
/* Chunks of Bytecode main-constant < Scanning on Demand args

  int constant = addConstant(&chunk, 1.2);
*/
/* Chunks of Bytecode main-constant < Chunks of Bytecode main-chunk-line
  writeChunk(&chunk, OP_CONSTANT);
  writeChunk(&chunk, constant);

*/
/* Chunks of Bytecode main-chunk-line < Scanning on Demand args
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
*/
/* A Virtual Machine main-chunk < Scanning on Demand args

  constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_ADD, 123);

  constant = addConstant(&chunk, 5.6);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_DIVIDE, 123);
*/
/* A Virtual Machine main-negate < Scanning on Demand args
  writeChunk(&chunk, OP_NEGATE, 123);
*/
/* Chunks of Bytecode main-chunk < Chunks of Bytecode main-chunk-line
  writeChunk(&chunk, OP_RETURN);
*/
/* Chunks of Bytecode main-chunk-line < Scanning on Demand args

  writeChunk(&chunk, OP_RETURN, 123);
*/
/* Chunks of Bytecode main-disassemble-chunk < Scanning on Demand args

  disassembleChunk(&chunk, "test chunk");
*/
/* A Virtual Machine main-interpret < Scanning on Demand args
  interpret(&chunk);
*/
//> Scanning on Demand args
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    if (strcmp(argv[1], "--bench") == 0) {
      runBenchmarks();
      return 0;
    }
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: clox [path]\n");
    fprintf(stderr, "       clox --bench\n");
    exit(64);
  }
  
  freeVM();
//< Scanning on Demand args
/* A Virtual Machine main-free-vm < Scanning on Demand args
  freeVM();
*/
/* Chunks of Bytecode main-chunk < Scanning on Demand args
  freeChunk(&chunk);
*/
  return 0;
}

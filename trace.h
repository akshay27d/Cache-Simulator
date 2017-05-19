#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "cache.h"
#include <math.h>

void traceRun(char* filename, int ways, int sets, int tagSize, int indexSize, int cap);
void traceread(char *filename, int len, int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int tagSize, const char* fileto);
void coldF(int input, char* buf,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit, const char* fileto);
void capacityF(int input, char* buf,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit, const char* fileto);
void addToConflict(int input,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit);
void addToCapacity(int input);
void addToCold(int input);
int getWrites();
int getHits();
int getMisses();
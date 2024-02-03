#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "cachelab.h"

typedef struct cacheLine {
    int valid;
    int time;
    uint64_t tag;
} cacheLine;

void parseArgs(int, char **);
void simulate();
cacheLine ** mallocCache(int);
void freeCache(cacheLine** cache);

int verboseFlag = 0;
int s, E, b;
char *trace;
int hit = 0, miss = 0, eviction = 0;

int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    simulate();
    printSummary(hit, miss, eviction);
    return 0;
}

void parseArgs(int argc, char *argv[]) {
    extern char *optarg;
    int errorFlag = argc <= 8;
    char arg;

    while ((arg = getopt(argc, argv, "vs:E:b:t:")) != -1) {
		switch (arg) { 
            case 'v':
                verboseFlag = 1;
                break;           
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
            case 't':
                trace = optarg;
                break;
			default:
				errorFlag = 1;
				break;
		}
	}

    if (errorFlag) {
        printf("Missing arguments");
        exit(errorFlag);
    }
}

void simulate() {
    int sets = 1 << s;
    cacheLine **cache = mallocCache(sets);

    FILE *fp = fopen(trace, "r");
    char op;
    uint64_t address;
    int size;
    int timestamp = 0;
    if (fp == NULL) {
        printf("Opening trace file error");
        exit(1);
    }

    uint64_t setBitMask = sets - 1;
    
    while (fscanf(fp, " %c %lx,%d", &op, &address, &size) == 3) {
        timestamp++;
        uint64_t setId = (address >> b) & setBitMask;
        uint64_t tag = (address >> (b+s));
        if (verboseFlag) {
            printf("%c %lx,%d", op, address, size);
        }
        // check op
        if (op == 'L' || op =='S' || op == 'M') {
            int missed = 1;
            // check each cacheline within the set
            for (int i = 0; i < E; i++) {
                if (cache[setId][i].valid && cache[setId][i].tag == tag) {
                    cache[setId][i].time = timestamp;
                    hit++;
                    missed = 0;
                }
            }
            int full = 1;
            int lru = 0;
            if (missed) {
                miss++;
                // check for empty (non valid) space in set
                for (int i = 0; i < E; i++) {
                    if (!cache[setId][i].valid) {
                        cache[setId][i].valid = 1;
                        cache[setId][i].tag = tag;
                        cache[setId][i].time = timestamp;
                        full = 0;
                        break;
                    } else {
                        if (cache[setId][i].time < cache[setId][lru].time) {
                            lru = i;
                        }
                    }
                }
            }
            if (missed && full) {
                cache[setId][lru].tag = tag;
                cache[setId][lru].time = timestamp;
                eviction++;
            }
            if (op == 'M') hit++;
            if (verboseFlag) {
                if (missed) {
                    printf(" miss");
                    if (full) {
                        printf(" eviction");
                    }
                } else {
                    printf(" hit");
                }
                if (op == 'M') {
                    printf(" hit");
                }
                printf("\n");
            }
        }
    }
    freeCache(cache);
    return;
    
}

cacheLine** mallocCache(int numberOfSets) {
    cacheLine **cache = (cacheLine **) malloc(sizeof(cacheLine *) * numberOfSets);
    if (cache == NULL) {
        printf("Malloc cache error");
        exit(1);
    }
    for (int i = 0; i < numberOfSets; i++) {
        cache[i] = (cacheLine *) malloc(sizeof(cacheLine) * E);
        if (cache[i] == NULL) {
            printf("Malloc cache line error");
            exit(1);
        }
        for (int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].time = 0;
            cache[i][j].tag = 0;
        }
    }
    return cache;
}

void freeCache(cacheLine** cache) {
    for (int i = 0; i < E; i++) {
        free(cache[i]);
    }
    free(cache);
}
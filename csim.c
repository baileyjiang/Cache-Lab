/* Author: Bailey Jiang
 * UID: u0780700
 * CADE USER: baileyj
 * CS4400
 * Simulates cache given inputs.
 */
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

void cache();


int main(int argc, char** argv)
{

    int hflag = 0;
    int vflag = 0;
    char* svalue = NULL;
    char* Evalue = NULL;
    char* bvalue = NULL;
    char* tvalue = NULL;
    int index;
    int gopt;
    opterr = 0;

    while((gopt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(gopt) {
            case 'h':
                hflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
            case 's':
                svalue = optarg;
                break;
            case 'E':
                Evalue = optarg;
                break;
            case 'b':
                bvalue = optarg;
                break;
            case 't':
                tvalue = optarg;
                break;
            case '?':
                if(optopt == gopt)
                    fprintf(stderr, "Option -%c requres an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }
    // Not enough arguments.
    for(index = optind; index < argc; index++) {
        printf ("Non-option argument %s\n", argv[index]);
        return 0;
    }
    // If user didn't specify parameters exit.
    if(svalue == NULL || Evalue == NULL || bvalue == NULL || tvalue == NULL) {
        printf("Inputs incorrect!");
        return 0;
    }
    cache(hflag, vflag, svalue, Evalue, bvalue, tvalue);
    return 0;
}

// Struct that contains data for a single memory address. This will be the container for the various
// parts of the address.
typedef struct {
    unsigned long long tag;
    unsigned long long index;
    unsigned long long block;
    int time;
    int valid;
} data;

// Main function that simulates the cache. Pass in the parameters from the command line.
// Our main simulation will be a 2d array that contains the sets (rows) and columns.
void cache(int h, int v, char *s, char *E, char *b, char *t) {
    unsigned long long sval = strtol(s, NULL, 10);
    unsigned long long eval = strtol(E, NULL, 10);
    unsigned long long bval = strtol(b, NULL, 10);
    // Container for our address.
    unsigned long long num;
    // Create an empty struct to initialize our array with.
    data tempData = {.tag = 0, .index = 0, .block = 0, .time = 0, .valid = 0};
    // Fill our cache array with empty structs.
    data cache[(int) pow(2, sval)][eval];
    for(int i = 0; i < (int) pow(2, sval); i++) {
        for(int j = 0; j < eval; j++) {
            cache[i][j] = tempData;
        }
    }

    // Have a timer that is incremented and set each time we deal with a memory address. This is how we implement the LRU.
    int time = 0;
    // Counter for our hit, miss, evict.
    int hit = 0;
    int miss = 0;
    int evict = 0;

    FILE *in = fopen(t, "r");
    if(in == NULL) {
        printf("Could not open file\n");
        exit(-1);
    } else {
        char c[1000];
        while (1) {
            fgets(c, 1000, in);
            if(feof(in)) {
                break;
            }
            c[999] = '\0';
            if(c[0] != 'I') {
                // If there's an M, we know that it either is a:
                // miss hit
                // hit hit.
                // Regardless, just continue to check if it FIRST hits or miss, and just increment the hit.
                if(c[1] == 'M') {
                    hit++;
                }
                int count = 0;
                for(int i = 3; i < 100; i++) {
                    if(c[i] != ',') {
                        count++;
                    }
                    else {
                        break;
                    }
                }
                sscanf(c+3, "%llx", &num);
                data d;
                // Set our block, index, and tag.
                d.block = num >> bval;
                d.index = (num >> bval ) & ((int)(pow(2, sval) - 1));
                d.tag = num >> (bval + sval);
                d.time = time;
                d.valid = 1;
                bool done = 0;
                for(int j = 0; j < eval; j++) {
                    //Get tag of what's currently in the cache
                    //If it matches and is valid, we have a hit. Increment time, set done to true (to skip to next
                    //address).
                    if(cache[d.index][j].tag == d.tag && cache[d.index][j].valid == 1) {
                        hit++;
                        time++;
                        done = 1;
                        // Update the time since we hit.
                        cache[d.index][j].time = time;
                        break;
                    } 
                }
                //We've missed.
                if(!done) {
                    miss++;
                    //Check if empty, and add it into the cache.
                    bool isThere = 0;
                    for(int j = 0; j < eval; j++) {
                        if (cache[d.index][j].valid == 0) {
                            cache[d.index][j] = d;
                            isThere = 1;
                            time++;
                            break;
                        }
                    }
                    //Do an eviction
                    if(!isThere) {
                        evict++;
                        int minTime = cache[d.index][0].time;
                        int minIndex = 0;
                        //Get the index of the minimum time within the column of our array. This was the least recently used.
                        for(int j = 0; j < eval; j++) {
                            if(cache[d.index][j].time < minTime) {
                                minTime = cache[d.index][j].time;
                                minIndex = j;
                            }
                        }
                        cache[d.index][minIndex] = d;
                        time++;
                    }   
                }
            }
        }
    }
    // Close the file.
    fclose(in);
    printSummary(hit, miss, evict);
}

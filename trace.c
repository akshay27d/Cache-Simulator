#include "trace.h"
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int* capacity;
int* cold;

int Iconflict =0;
int Icapacity =0;
int Icold =0;
int size=0;
int size2=0;
char* response;

int writes = 0;
int reads = 0;

int hits=0;
int misses=0;

int power(int a, int b){
    int i;
    int result=1;
    for(i=0; i<b; i++){
        result *=a;
    }
    return result;

}

int mask(int input, int startPt, int mask){     //function isolates which section of binary we want
    input = input >> (startPt-mask);            //input = instruction
    int maskNum=0;                              //startPt = starting bit I want, counting from right
    int i;
    for (i = 0; i<mask;i++){                //mask = number of bits I want to look at
        maskNum+=power(2, i);
    }

    int retval = input & maskNum;
    return retval;

}

 
void addToCold(int input){
    cold[Icold++] = input;
}

void addToCapacity(int input){
    if(Icapacity == size2){
        for(int i =0; i<size2-1;i++){
            capacity[i] = capacity[i+1];
        }
        capacity[size2-1] = input;
    }
    else{
        capacity[Icapacity] = input;
        Icapacity++;
    }
}

void addToConflict(int input,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit){
    int compare1 = mask(input, len-tagSize, indexSize);       //index tag
    int check =0;
    for(int i =0; i<ways; i++){
        if(conflict[compare1][i][0] == 2147483647){
            conflict[compare1][i][0] = input;
            //if(bit[0]=='s'){
                conflict[compare1][i][1] = 1;
            //}
            check =1;
            break;
        }
   }

    if(check ==0){
        if(conflict[compare1][0][0] == 1){
            writes++;
        }
        for(int i =0; i <ways-1; i++){
            conflict[compare1][i][0] = conflict[compare1][i+1][0];
        }
        conflict[compare1][ways-1][0] = input;
        if(bit[0]=='s'){
                conflict[compare1][ways-1][1] = 1;
         }
    }
}

void capacityF(int input, char* buf,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit, const char* fileto){
    char resp[100] = "";
    int l = strlen(buf);

    int i =0;
    int check=0;
    for(i =Icapacity-1; i >= 0; i--){
        if (capacity[i] == input){
            check =1;
            FILE *file = fopen(fileto, "a");
            char buf2[100] = "";
            strncpy(buf2, buf, l-1);
            strcat(resp, buf2);
            strcat(resp, " conflict\n\0");
            fputs(resp, file);
            fclose(file);
            addToConflict(input,sets, ways, dirty, conflict, indexSize, len, tagSize, bit);

            break;
        }
     }

    if(check ==0){
        addToCapacity(input);
        addToConflict(input,sets, ways, dirty, conflict, indexSize, len, tagSize, bit);
        
        FILE *file = fopen(fileto, "a");
        char buf2[100]="";
        strncpy(buf2, buf, l-1);
        strcat(resp, buf2);
        strcat(resp, " capacity\n\0");
        fputs(resp, file);
        fclose(file);
    }
}

void coldF(int input, char* buf,int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int len, int tagSize, char* bit, const char* fileto){
    char resp[100] = "";
    int l = strlen(buf);

    int i =0;
    int check=0;
    for(i =Icold-1; i >= 0; i--){
        if (cold[i] == input){
            check =1;
            capacityF(input, buf,sets, ways, dirty, conflict, indexSize, len, tagSize, bit, fileto);
            break;
        }
     }

    if(check ==0){
        addToCold(input);
        addToCapacity(input);
        addToConflict(input,sets, ways, dirty, conflict, indexSize, len,tagSize, bit);
       
        FILE *file = fopen(fileto, "a");
        char buf2[100]="";
        strncpy(buf2, buf, l-1);
        strcat(resp, buf2);
        strcat(resp, " compulsory\n\0");
        fputs(resp, file);
        fclose(file);
    }

}

void traceread(char *filename, int len, int sets, int ways, int dirty, int conflict[sets][ways][dirty], int indexSize, int tagSize, const char* fileto){   
    FILE *input;
    input = fopen(filename, "r");
    char buf[1000];

    if(!input){
        printf("Could Not Find File\n");
        return;
    }
    while(fgets(buf,1000,input)!=NULL){
       char* input, *found, *done, *bit;
        input = strdup(buf);
        int i=-1;
        
        while((found = strsep(&input, " ")) !=NULL){    //parse
            ++i;
            if(i == 0) bit = found;
            if(i == 1) done= found;
        }

        int ret = (int)strtol(done, NULL , 16);        //hex string into int
        ret = mask(ret, 32, len);
        char resp[100] = "";
        int l = strlen(buf);
        int compare1 = mask(ret, len-tagSize, indexSize);       //index tag

        int check =0;
        for (int i =0; i<ways;i++){
            if(conflict[compare1][i][0] == ret){
                hits++;
                conflict[compare1][i][1] = 1;               //WRITEBACK

                check = 1;
        
                FILE *file = fopen(fileto, "a");
                char buf2[100]="";
                strncpy(buf2, buf, l-1);
                strcat(resp, buf2);
                strcat(resp, " hit\n\0");
                fputs(resp, file);
                fclose(file);

                int ii=0;
                int hit1 = 0;
                for(ii =Icapacity-1; ii >= 0; ii--){
                    if(capacity[ii] == ret){
                        hit1 =1;
                        break;
                    }
                }
                if(hit1==0){
                    addToCapacity(ret);
                }


                break;
            }
        }
        if (check==0){
            misses++;
            coldF(ret, buf,sets, ways, dirty, conflict, indexSize, len, tagSize, bit, fileto);
        }
    }
}

void traceRun(char* filename, int ways, int sets, int tagSize, int indexSize, int cap){
    cold =(int*)malloc(sizeof(int)*1000000);
    capacity = (int*)malloc(sizeof(int)*1000000);
    response = (char*)malloc(sizeof(char)*200);
    FILE *file;
    char resp[100] = "./";
    int l = strlen(filename);
    char buf2[100]="";
    strncpy(buf2, filename, l);
    strcat(resp, buf2);
    strcat(resp, ".simulated\0");

    const char* fileto = (char*)malloc(sizeof(char)*200);
    fileto = resp;
    file = fopen(resp, "w");
    fclose(file);
    int dirty=2;

    int conflict[sets][ways][dirty];

    for (int i =0; i<sets; i++){
        for (int p = 0; p < ways; p++)
        {
            conflict[i][p][0]=2147483647;
            conflict[i][p][1]= 0;
        }
    }

    int len = tagSize+indexSize;
    size = sets;
    size2 = ways*sets;


    traceread(filename, len, sets, ways, dirty, conflict,indexSize, tagSize, fileto);
}

int getWrites(){
    return writes;
}
int getHits(){
    return hits;
}
int getMisses(){
    return misses;
}


//
//  vmpager.c
//  vmpager
//
//  Created by Ze Li on 10/7/14.
//  Copyright (c) 2014 Ze Li. All rights reserved.
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#define N 256

//Data file intepreted as an array of structs as follows
typedef struct{
    uint8_t pid;
    uint8_t page;
} MemoryAccess;

//one for each process, stored when process was not running and
//restored when the process recommenced
typedef struct{
    int frame;
    int pageHits;
    int pageMisses;
} PageTableEntry;

typedef struct{
    uint8_t pid;
    uint8_t page;
    int vacant;
} FrameTableEntry;

//forward declaration of functions
void init_one_frame(FrameTableEntry*);
void init_one_page(PageTableEntry*);
void init_frame_tbl(int);
void init_page_tbl();
void page_process( MemoryAccess);
void infinite_memory(MemoryAccess);
void fifo(MemoryAccess);
void initialize(int);
//end of forward declaration
//#define N 256 //# of running processes, # of pages in each running process

//globals
int frame_index = 0;
int hits = 0;
int misses = 0;
int frame_table_size = N;
//the table for all processes simulated as a 2d array
FrameTableEntry *frame_tables;
PageTableEntry page_tables[N][N];//pid, page
//main
int main(int argc, char**argv){
    int i = 0,index = 0,num_memory_access = 0;
    //to get the size from fstat
    struct stat file_stat;
    long int size,masize;
    MemoryAccess * maarray;
    printf("\nThis is hw3 done by Ze Li, zli48, 676755673\n");
    printf("This program is to simulate a memory paging system.\n\n");
    printf("You entered: ");
    for(i = 0; i < argc; i++){
        printf("%s ",argv[i]);
    }
    printf("\n\n");
    if(argc < 2){
        printf("No file name entered. \n");
        return 1;
    }
    if((i = open(argv[1],O_RDONLY))==-1){
        printf("Fail to open %s.\n",argv[1]);
        return 1;
    }
    if(argc >= 3)
        num_memory_access = atoi(argv[2]);
//        printf("%d\n",num_memory_access);
    if(argc == 4){
        frame_table_size = atoi(argv[3]);
//        printf("%d\n",frame_table_size);
    }
    if(frame_table_size == 0){
        frame_table_size = N;
    }
//    printf("frame table size: %d\n",frame_table_size);
    //initialize the frame table entry and pagetableentry
    initialize(frame_table_size);
    
    if(fstat(i,&file_stat) < 0){
        printf("fsat error. \n");
        return 1;
    }
    if(num_memory_access){
        size = sizeof(MemoryAccess)*num_memory_access;
    }
    else{
        size = file_stat.st_size;
    }
    masize = size/sizeof(MemoryAccess);
    maarray = mmap(NULL,size,PROT_READ,MAP_SHARED,i,0);
    
    while(index < masize){
        infinite_memory(maarray[index++]);
//        printf("%d,%d\n",maarray[index].pid,maarray[index].page);
    }
    printf("Infinite Memory:\nHits: %d \nMisses: %d\n\n", hits, misses);
    index = 0;
    initialize(frame_table_size);
    while(index < masize){
        fifo(maarray[index++]);
    }
    printf("FIFO:\nHits: %d \nMisses: %d\n", hits, misses);
    munmap(maarray,masize);
    free(frame_tables);
    return 1;
}



void init_one_frame(FrameTableEntry* fte){
    fte->page = 0;
    fte->pid = 0;
    fte->vacant = 1;
    //    printf("%d\n",fte->vacant);
}

void init_one_page(PageTableEntry* pte){
    pte->frame = -1;
    pte->pageHits = 0;
    pte->pageMisses = 0;
}

/*
 to initialize the frametableentry given size
 */
void init_frame_tbl(int size){
    frame_tables = malloc(sizeof(FrameTableEntry)*size);
    int i;
    for(i = 0; i < size; i++){
        init_one_frame(&frame_tables[i]);
//                        printf("pid: %d page: %d vacant: %d\n",frame_tables[i].page,frame_tables[i].pid,frame_tables[i].vacant);
    }
}
/*
 to initialize the pagetableentry given the size
 */
void init_page_tbl(){
    int i,j;
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            init_one_page(&page_tables[i][j]);
            //                        printf("frame: %d pageHit: %d pageMiss: %d\n",page_tables[i][j].frame,page_tables[i][j].pageHits,page_tables[i][j].pageMisses);
        }
    }
}

/*to deal with infinite memory case*/
void infinite_memory(MemoryAccess ma){
    if(page_tables[ma.pid][ma.page].frame == -1){
        page_tables[ma.pid][ma.page].frame = 1;
        page_tables[ma.pid][ma.page].pageMisses++;
        misses++;
    }
    else{
        page_tables[ma.pid][ma.page].pageHits++;
        hits++;
    }
}
/*to deal with first in first out algorithm*/
void fifo(MemoryAccess ma){
    if(page_tables[ma.pid][ma.page].frame == -1){
        page_tables[ma.pid][ma.page].frame = 1;
        page_tables[ma.pid][ma.page].pageMisses++;
        misses++;
        if(!frame_tables[frame_index].vacant){//to make room for the new frame
            page_tables[frame_tables[frame_index].pid][frame_tables[frame_index].page].frame = -1;
        }
        frame_tables[frame_index].pid = ma.pid;
        frame_tables[frame_index].page = ma.page;
        frame_tables[frame_index++].vacant = 0;//to increment index
        frame_index%=frame_table_size;
//        printf("frame index: %d\n",frame_index);
    }
    else{
        page_tables[ma.pid][ma.page].pageHits++;
        hits++;
    }
}

void initialize(int size){
    init_page_tbl();
    init_frame_tbl(size);
    hits = 0;
    misses = 0;
}






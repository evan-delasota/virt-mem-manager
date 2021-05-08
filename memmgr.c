//
//  memmgr.c
//  memmgr
//
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE  256
#define PAGE_SIZE 256
#define TLB_SIZE 16


//-------------------------------------------------------------------
struct tlb {
  unsigned char logical_add;
  unsigned char physical_add;
};

int tlb_index = 0;
struct tlb tlb_struct[TLB_SIZE];

void tlb_add(unsigned char logical_add, unsigned char physical_add) {
  struct tlb *candidate = *tlb_struct[tlb_index * TLB_SIZE];
  
  tlb_index++;
  candidate->logical_add = logical_add;
  candidate->physical_add = physical_add;

}

unsigned getpage(unsigned x) { return (0xff00 & x) >> 8; }

unsigned getoffset(unsigned x) { return (0xff & x); }

void getpage_offset(unsigned x) {
  unsigned  page   = getpage(x);
  unsigned  offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, paddress: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}

int main(int argc, const char* argv[]) {
  FILE* fadd = fopen("addresses.txt", "r");    // open file addresses.txt  (contains the logical addresses)
  if (fadd == NULL) { fprintf(stderr, "Could not open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");     // contains the logical and physical address, and its value
  if (fcorr == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  char buf[BUFLEN];
  unsigned   page, offset, physical_add, frame = 0;
  unsigned   logic_add;                  // read from file address.txt
  unsigned   virt_add, phys_add, value;  // read from file correct.txt
  // not quite correct -- should search page table before creating a new entry
  //   e.g., address # 25 from addresses.txt will fail the assertion
  // TODO:  add page table code
  // TODO:  add TLB code
  int page_table[BUFLEN]; 
  int page_frames[FRAME_SIZE];
  int used_frame[FRAME_SIZE];
  int total_add = 0;
  int hits = 0;
  int page_faults = 0;

  for (auto i = 0; i < BUFLEN; ++i) {
    page_table[i] = -1;
    page_frames[i] = -1;
    used_frame[i] = 0;
  }


  while (frame < FRAME_SIZE) {

    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page   = getpage(  logic_add);
    offset = getoffset(logic_add);
    
    physical_add = frame++ * FRAME_SIZE + offset;
    
    assert(physical_add == phys_add);
    
    // todo: read BINARY_STORE and confirm value matches read value from correct.txt
    int backing = open("BACKING_STORE.bin", O_RDONLY);
    unsigned char bin[BUFLEN];
    FILE *ptr;

    ptr = fopen("BACKING_STORE", "rb");
    fread(bin, sizeof(bin), 1, ptr);

    printf("logical: %5u (page: %3u, offset: %3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
    if (frame % 5 == 0) { printf("\n"); }
  }
  fclose(fcorr);
  fclose(fadd);
  
  page_faults =  page_faults / (sizeof page_table);
  printf("Page fault rate: %d, TLB Hit Rate: %d", page_faults, hits);
  printf("ALL logical ---> physical assertions PASSED!\n");
  printf("!!! This doesn't work passed entry 24 in correct.txt, because of a duplicate page table entry\n");
  printf("--- you have to implement the PTE and TLB part of this code\n");
  printf("\n\t\t...done.\n");
  return 0;
}

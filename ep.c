#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//SENAC - BCC - 2022

//referencias
//https://gist.github.com/glen-mac/8ba1ee9b96767fed74db24defabc1d7e

#define MBR_SIZE 512
#define BOOT_SIG_START 510
#define NUM_PARTITIONS 4
#define PARTITION_START 446

typedef struct Partitions {
  uint32_t start_block;
  uint32_t end_block;
  uint32_t sector;
  uint8_t type;
  uint8_t bootable;
} partition;

int is_little_endian(void);
int is_mbr_valid(uint8_t*);
int is_bootable(uint32_t, uint8_t *);
uint32_t get_disk_size(uint32_t, uint8_t*);
char *str_sector(uint32_t);
char *get_format(uint8_t);

int main(void) {
  uint8_t mbr[MBR_SIZE];
  uint32_t disk_size;

  //usa o struct para armazenar os dados das particoes
  partition *partitions = malloc(NUM_PARTITIONS * sizeof(partition));

  FILE *file = fopen("mbr.bin", "rb");
  if (!file) {
    printf("\nError opening mbr.bin file!");
    exit(-1);
  }

  int result = fread(&mbr, MBR_SIZE, 1, file);

  //checa o boot signature
  if (!is_mbr_valid(mbr)) {
    printf("\nMRB Invalido. Abortando..");
    exit(1);
  }

  //coleta a informacao buscando nos enderecos.
  for (int i=0; i<NUM_PARTITIONS; i++) {
    uint32_t start = PARTITION_START + i * 16;
    partitions[i].start_block = *(uint32_t*) &mbr[start + 8];
    partitions[i].end_block = *(uint32_t*) &mbr[start + 12];
    partitions[i].sector = *(uint32_t*) &mbr[start + 12];
    partitions[i].type = *(uint8_t*) &mbr[start + 4];
    partitions[i].bootable = is_bootable(start, mbr);
  }

  disk_size = *(uint32_t*) &mbr[PARTITION_START + 12];

  printf("\nDisk /dev/sda: %s, %u bytes, %u sectors \n", str_sector(disk_size), disk_size, &mbr[PARTITION_START + 12]);
  printf("\n\x1B[1mDevice    Start        End          Sectors    Size     Type\x1B[0m");

  for (int i=0; i<NUM_PARTITIONS; i++) {
    printf("\n/dev/sda%d %-12d %-12d %-10d %-8s %-10s", i+1,
    partitions[i].start_block, 
    partitions[i].end_block, 
    partitions[i].sector,
    str_sector(partitions[i].sector),
    get_format(partitions[i].type));
  }

  free(partitions);
  return 0;
}

uint32_t get_disk_size(uint32_t start, uint8_t *mbr) {
  uint32_t size = *(uint32_t*) &mbr[start];
  if (size) {
    return size * 512;
  }
  return 0;
}

//pega o formato da particao
char * get_format(uint8_t form) {
  char *format = malloc(20);

  switch (form) {
  case 0x0: sprintf(format, "Empty");
    break;
  case 0xbf: sprintf(format, "Solaris");
    break;
  case 0x82: sprintf(format, "Linux Swap");
    break;
  case 0x83: sprintf(format, "Linux");
    break;
  case 0x84: sprintf(format, "Win HP");
    break;
  case 0x85: sprintf(format, "Linux Extended");
    break;
  case 0x86: sprintf(format, "NTFS");
    break;
  case 0x87: sprintf(format, "NTFS");
    break;
  case 0xee: sprintf(format, "MS GPT");
    break;
  case 0x7: sprintf(format, "HPFS");
    break;
  default: sprintf(format, "Unkown");
  }

  return format;
}

int is_mbr_valid(uint8_t* mbr) {
  if (mbr[BOOT_SIG_START] == 0x55 && mbr[BOOT_SIG_START + 1] == 0xaa) {
    return 1;
  }
  return 0;
}

int is_bootable(uint32_t pos, uint8_t* mbr) {
  if (mbr[pos] == 0x80) { 
    return 1;
  }
  return 0;
}

char * str_sector(uint32_t size) {
  double kb, mb, gb;
  char *f_size = malloc(10);
  kb = size / 2;
  mb = kb / 1024;
  gb = mb / 1024;

  if (gb > 1) {
    sprintf(f_size, "%2.fGB", gb);
  }
  else if (mb >= 1) {
    sprintf(f_size, "%2.fMB", mb);
  }
  else {
    sprintf(f_size, "%2.fKB", kb);
  }
  return f_size;
}

int is_little_endian() {
  unsigned int x = 0x76543210;
  char *c = (char*) &x;
  if (*c == 0x10) {
    printf("LITTLE ENDIAN");
    return 1;
  }
  else {
    printf("BIG ENDIAN");
    return 0;
  }
}

#ifndef CONFIG_h
#define CONFIG_h

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#define FIFO_PATH "/tmp/fifo"
#define IMAGE_PATH "imagem.pgm"
#define NEGATIVO 0
#define SLICE 1

typedef struct PGM{
  int w, h, maxv;      //maxv = 255
  unsigned char* data; //w*h bytes
}PGM;

typedef struct Header{
  int w, h, maxv; //metadados da imagem
  int mode;       //0 = NEGATIVO ; 1 = SLICE
  int t1, t2;     //valido se modo = SLICE
}Header;

typedef struct Task{
  int row_start; //linha inicial
  int row_end;   //linha final
}Task;

int read_PGM(const char* path, PGM* img){
  FILE* file = fopen(path, "rb");
  if(!file){
    perror("Erro ao abrir o arquivo.");
    return -1;
  }

  char magic[4]; //DEVE ACHAR "P5" NA PRIMEIRA LINHA SEMPRE
  /*Aqui o numero mágico é P5 que está na primeira fila mas a linha é P5\n ai precisamos deixar espaço para 3 +1
  o +1 é por que o fgets coloca '\0' no final do vetor, entao são 3 espaços para P5\n +1 para o \0
  por isso fica magic[4] */
  if(!fgets(magic, sizeof(magic), file)){
    printf("Arquivo vazio.\n");
    fclose(file);
    return -1;
  }else{
    if(magic[0] != 'P' || magic[1] != '5'){ //verifica se tem o P5 na primeira linha
      printf("Não é um arquivo PGM.");
      fclose(file);
      return -1;
    }
  }

  //retira os comentários
  int ch;
  while((ch = fgetc(file)) == '#'){
    while(fgetc(file) != '\n');
  }
  ungetc(ch, file);

  fscanf(file, "%d %d %d", &img->w, &img->h, &img->maxv);

  fgetc(file);

  //Aloca memoria para o data e então lê a imagem
  img->data = (unsigned char*)malloc(img->w * img->h * sizeof(unsigned char));
  fread(img->data, 1, img->w * img->h, file);

  fclose(file);
  return 0;
}

int write_PGM(const char* path, const PGM* img){
  FILE* file = fopen(path, "wb");
  fprintf(file, "P5\n%d %d\n%d\n", img->w, img->h, img->maxv);

  fwrite(img->data, 1, img->w * img->h, file);

  fclose(file);
  return 0;
}

#endif
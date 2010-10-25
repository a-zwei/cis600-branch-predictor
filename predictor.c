#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "predictor.h"

void initialize_btb(bool *btb, size_t size)
{
  for(int i = 0; i < size; i++) {
    btb[i] = true;
  }
}

void initialize_tag(uint32_t *tag, size_t size)
{
  for(int i = 0; i < size; i++) {
    tag[i] = 0;
  }
}

predictor make_predictor(uint32_t entries)
{
  predictor result;

  result.entries = entries;
  result.recent2 = 0;
  result.btb = malloc(4 * entries * sizeof(int));
  initialize_btb(result.btb, 4 * entries);
  result.tag = malloc(4 * entries * sizeof(uint32_t));
  initialize_tag(result.tag, 4 * entries);

  return result;
}

void free_predictor(predictor p)
{
  free(p.btb);
  free(p.tag);
}

size_t btb_index(predictor p, uint32_t addr)
{
  return (addr % p.entries << 2) | (p.recent2 & 4);
}

bool hit(predictor p, uint32_t addr)
{
  return p.tag[btb_index(p, addr)] == addr;
}

bool predict(predictor p, uint32_t addr)
{
  return p.btb[btb_index(p, addr)];
}

void record_branch(predictor p, uint32_t addr, uint32_t target, bool taken)
{
  // write stats
  printf("addr: %#x\ttarget: %#x\t", addr, target);
  printf("prediction: %d\ttaken: %d", predict(p, addr), taken);
  if(hit(p, addr)) {
    printf("\thit");
  }
  printf("\n");

  // update the predictor
  p.btb[btb_index(p, addr)] = taken;
  p.tag[btb_index(p, addr)] = addr;
  p.recent2 <<= 1;
  p.recent2 &= taken;
}

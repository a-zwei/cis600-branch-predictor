#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "predictor.h"

void initialize_btb(int *btb, size_t size)
{
  for(int i = 0; i < size; i++) {
    btb[i] = -1;
  }
}

predictor make_predictor(uint32_t entries)
{
  predictor result;

  result.entries = entries;
  result.btb = malloc(4 * entries * sizeof(int));
  initialize_btb(result.btb, 4 * entries);
  result.recent2 = 0;

  return result;
}

void free_predictor(predictor p)
{
  free(p.btb);
}

size_t btb_index(predictor p, uint32_t addr)
{
  return (addr % p.entries << 2) | (p.recent2 & 4);
}

bool hit(predictor p, uint32_t addr)
{
  return p.btb[btb_index(p, addr)] > -1;
}

bool predict(predictor p, uint32_t addr)
{
  return p.btb[btb_index(p, addr)] ? 1 : 0;
}

void record_branch(predictor p, uint32_t addr, uint32_t target, bool taken)
{
  // write stats
  printf("addr: %x\ttarget: %x\t", addr, target);
  printf("prediction: %d\ttaken: %d", predict(p, addr), taken);
  if(hit(p, addr)) {
    printf("\thit\n");
  }
  printf("\n");

  // update the predictor
  p.btb[btb_index(p, addr)] = taken;
  p.recent2 <<= 1;
  p.recent2 &= taken;
}

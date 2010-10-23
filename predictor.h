#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdbool.h>
#include <stdint.h>

struct predictor
{
  uint32_t entries;
  int *btb;
  uint8_t recent2;
};

typedef struct predictor predictor;

predictor make_predictor(uint32_t entries);
void free_predictor(predictor p);
void record_branch(predictor p, uint32_t addr, uint32_t target, bool taken);

#endif // PREDICTOR_H
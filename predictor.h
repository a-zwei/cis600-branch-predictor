#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct branch
{
  uint32_t addr;
  uint32_t occurrences;
  uint32_t correct;
  struct branch *next;
};
typedef struct branch branch;

struct stats
{
  uint32_t branches;
  uint32_t hits;
  uint32_t mispredictions;
  branch *history;
};
typedef struct stats stats;

struct entry
{
  uint32_t address, target;
};
typedef struct entry entry;

struct predictor
{
  uint32_t entries;
  unsigned int recent : 2; // 2 bits of recent branch state
  entry *btb;
  stats stats;
};
typedef struct predictor predictor;

predictor make_predictor(uint32_t entries);
void free_predictor(predictor p);
void record_branch(predictor *p, uint32_t addr, uint32_t target, bool taken);
branch most_significant_branch(predictor p);
size_t history_count(predictor p);

#endif // PREDICTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "predictor.h"

void initialize_btb(entry *btb, size_t size)
{
  for(int i = 0; i < size; i++) {
    btb[i] = (entry) {0, 0};
  }
}

void initialize_stats(stats *s)
{
  s->branches = s->hits = s->mispredictions = 0;
  s->history = NULL;
}

predictor make_predictor(uint32_t entries)
{
  predictor result;

  result.entries = entries;
  result.recent = 0;
  result.btb = malloc(entries * sizeof(entry));

  initialize_btb(result.btb, entries);
  initialize_stats(&result.stats);

  return result;
}

void free_history(branch *b)
{
  branch *cur = b, *next;
  while(cur != NULL) {
    next = cur->next;
    free(cur);
    cur = next;
  }
}

void free_predictor(predictor p)
{
  free(p.btb);
  free_history(p.stats.history);
}

size_t btb_index(predictor p, uint32_t addr)
{
  return (addr % (p.entries / 4) << 2) | p.recent;
}

bool hit(predictor p, uint32_t addr)
{
  return p.btb[btb_index(p, addr)].address == addr;
}

uint32_t predict(predictor p, uint32_t addr)
{
  return hit(p, addr) ? p.btb[btb_index(p, addr)].target : 0;
}

branch *find_or_add_history_node(predictor *p, uint32_t addr)
{
  branch *cur = p->stats.history;
  if(cur == NULL) {
    cur = malloc(sizeof(branch));
    p->stats.history = cur;
    return cur;
  }
  while(cur != NULL) {
    if(cur->addr == addr) break;

    if(cur->next == NULL) {
      cur->next = malloc(sizeof(branch));
      *(cur->next) = (branch) {addr, 0, 0, NULL};
    }
    cur = cur->next;
  }
  return cur;
}

void add_history(predictor *p, uint32_t addr, bool correct)
{
  branch *b = find_or_add_history_node(p, addr);
  b->occurrences++;
  if(correct) b->correct++;
}

size_t history_count(predictor p)
{
  size_t result = 0;
  branch *cur = p.stats.history;
  while(cur != NULL) {
    result++;
    cur = cur->next;
  }
  return result;
}

branch most_significant_branch(predictor p)
{
  branch *cur = p.stats.history, result;
  uint32_t occurrences = 0;
  while(cur != NULL) {
    if(cur->occurrences > occurrences) {
      occurrences = cur->occurrences;
      result = *cur;
    }
    cur = cur->next;
  }
  return result;
}

void erase(predictor p, uint32_t addr)
{
  p.btb[btb_index(p, addr)] = (entry) {0, 0};
}

void record_branch(predictor *p, uint32_t addr, uint32_t target, bool taken)
{
  uint32_t prediction = predict(*p, addr);
  bool correct = taken ? prediction == target : prediction == 0;

  // record stats
  p->stats.branches++;
  if(hit(*p, addr)) p->stats.hits++;
  if(!correct) p->stats.mispredictions++;
  add_history(p, addr, correct);

  // update the predictor
  if(taken) {
    p->btb[btb_index(*p, addr)] = (entry) {addr, target};
  } else {
    if(hit(*p, addr)) erase(*p, addr);
  }
  p->recent <<= 1;
  p->recent |= taken;
}

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
  result.btb = malloc(4 * entries * sizeof(bool));
  result.tag = malloc(4 * entries * sizeof(uint32_t));

  initialize_btb(result.btb, 4 * entries);
  initialize_tag(result.tag, 4 * entries);
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
  free(p.tag);
  free_history(p.stats.history);
}

size_t btb_index(predictor p, uint32_t addr)
{
  return (addr % p.entries << 2) | (p.recent & 4);
}

bool hit(predictor p, uint32_t addr)
{
  return p.tag[btb_index(p, addr)] == addr;
}

bool predict(predictor p, uint32_t addr)
{
  return p.btb[btb_index(p, addr)];
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
      cur->next->addr = addr;
      cur->next->next = NULL;
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

void record_branch(predictor *p, uint32_t addr, uint32_t target, bool taken)
{
  // record stats
  p->stats.branches++;
  if(hit(*p, addr)) p->stats.hits++;
  bool correct = predict(*p, addr) == taken;
  if(!correct) p->stats.mispredictions++;
  add_history(p, addr, correct);

  // update the predictor
  p->btb[btb_index(*p, addr)] = taken;
  p->tag[btb_index(*p, addr)] = addr;
  p->recent <<= 1;
  p->recent &= taken;
}

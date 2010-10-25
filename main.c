#define _GNU_SOURCE // for getline

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "predictor.h"

int main(int argc, char **argv)
{
  if(argc != 2) {
    fprintf(stderr, "Please specify the number of BTB entries.\n");
    return 1;
  }
  uint32_t entries = strtoul(argv[1], NULL, 0);
  predictor p = make_predictor(entries);

  char *line = NULL;
  size_t line_size = 0;
  while(getline(&line, &line_size, stdin) > -1) {
    uint32_t addr = strtoul(strtok(line, "\t"), NULL, 0);
    uint32_t target = strtoul(strtok(NULL, "\t"), NULL, 0);
    bool taken = strtoul(strtok(NULL, "\t"), NULL, 0);
    record_branch(&p, addr, target, taken);
  }
  free(line);

  printf("Note: I initialized the BTB to predict all branches taken.\n\n");
  printf("(a) Number of hits: %d. Total branches: %d. Hit rate: %.1f%%.\n",
      p.stats.hits, p.stats.branches, (float)p.stats.hits/p.stats.branches*100);
  printf("(b) Incorrect predictions: %d of %d, or %.1f%%.\n",
      p.stats.mispredictions, p.stats.branches,
      (float)p.stats.mispredictions/p.stats.branches*100);

  branch msb = most_significant_branch(p);
  printf("(c) Most significant branch (%0x) seen %d times of %d, or %.1f%%\n",
      msb.addr, msb.occurrences, p.stats.branches,
      (float)msb.occurrences/p.stats.branches*100);     
  printf("    Correct predictions: %d of %d, or %.1f%%\n", msb.correct,
      p.stats.branches - p.stats.mispredictions,
      (float)msb.correct/(p.stats.branches - p.stats.mispredictions)*100);
  printf("(d) Unique branches: %d\n    Misses reported: %d\n"
      "    Capacity misses = %d\n", history_count(p),
      p.stats.branches - p.stats.hits,
      p.stats.branches - p.stats.hits - history_count(p));

  free_predictor(p);

  return 0;
}

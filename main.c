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
    record_branch(p, addr, target, taken);
  }
  free(line);
  free_predictor(p);

  return 0;
}

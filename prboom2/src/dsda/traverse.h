#include "r_defs.h"

// Path in sector graph
struct tpath
{
  // Vertex on path
  vertex_t* v;
  // Line leading to vertex (NULL for start of traversal)
  line_t* line;
  // Prev vertex on path
  struct tpath* prev;
  // Next vertex on path
  struct tpath* next;
};

typedef void (*path_cb)(sector_t* sector, struct tpath* path, void* data);
typedef void (*cycle_cb)(sector_t* sector, struct tpath* cycle, void* data);

struct traverse_ctx
{
  sector_t* sector;
  path_cb pcb;
  cycle_cb ccb;
  void* data;
};

void dsda_TraverseSectorGraph(struct traverse_ctx* ctx);

#include "r_defs.h"

//
// Sector traversal
//
// Traverses the graph structure of a sector, invoking callbacks on discovery
// of paths and cycles.

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

// Path callback
typedef void (*path_cb)(sector_t* sector, struct tpath* path, void* data);
// Cycle callback
typedef void (*cycle_cb)(sector_t* sector, struct tpath* cycle, void* data);

// Context for traversal
struct traverse_ctx
{
  path_cb pcb;
  cycle_cb ccb;
  void* data;
  dboolean incomplete;
};

// Traverse a sector
void dsda_TraverseSectorGraph(struct traverse_ctx* ctx, sector_t* sector);

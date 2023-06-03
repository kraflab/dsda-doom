#ifndef __dsda_ajbsp_glue__
#define __dsda_ajbsp_glue__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

typedef struct
{
  void* vertexes;
  void* segs;
  void* subsectors;
  void* nodes;
  unsigned int vsize;
  unsigned int ssize;
  unsigned int sssize;
  unsigned int nsize;
} dsda_ajbsp_glnodes_t;

bool dsda_ajbsp_LoadGLNodes(const char* inwad, const char* outwad,
                            const char* map, dsda_ajbsp_glnodes_t* nodes);

#ifdef __cplusplus
}
#endif

#endif

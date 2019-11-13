#include "vmcommon.h"

struct YatcVector_s {
  YatcVariable** mem;
  unsigned maxLength;
};
typedef struct YatcVector_s YatcVector;

// YatcVector will by itself allocate its nested context

unsigned yatc_vector_length(YatcVector* vc) {
  if (!vc || !vc->mem)
    return 0;
  unsigned count = 0;
  for (unsigned i = 0; i < vc->maxLength; i++) {
    if (vc->mem[i])
      count += 1;
  }
  return count;
}

YatcVector* yatc_vector_create(unsigned max) {
  YatcVector* vc = malloc(sizeof(YatcVector));
  vc->mem = calloc(max, sizeof(YatcVariable*));
  vc->maxLength = max;
  return vc;
}

YatcVariable* yatc_variable_clone(YatcVariable* orig) {
  if (!orig)
    return NULL;
  return yatc_variable_create("", yatc_variable_get_type(orig), yatc_variable_get(orig), 255);
}

unsigned yatc_vector_lastPopulatedIndex(YatcVector* vc) {
  if (!vc)
    return 0;
  unsigned lastOne = 0;
  for (unsigned i = 0; i < vc->maxLength; i++) {
    if (vc->mem[i])
      lastOne = i;
  }
  return lastOne;
}

unsigned yatc_vector_append(YatcVector* vc, YatcVariable* vl) {
  if (!vc || !vc->mem || !vl)
    return 0;
  YatcVariable* clone = yatc_variable_clone(vl);
  if (!clone)
    return 0;
  char* nameForV = calloc(8, sizeof(char));
  sprintf(nameForV, "%d", yatc_vector_length(vc));
  yatc_variable_set_name(vl, nameForV);
  free(nameForV);
  unsigned where = yatc_vector_lastPopulatedIndex(vc);
  if (!vc->mem[0])
    where = 0;
  if (where >= (vc->maxLength - 1))
    return 2;
  vc->mem[where] = clone;
  return 1;
}


YatcVariable* yatc_vector_get(YatcVector* vc, unsigned index) {
  if (!vc)
    return NULL;
  char* indx = calloc(8, sizeof(char));
  sprintf(indx, "%d", index);
  YatcVariable* meetingReq = NULL;
  for (unsigned i = 0 ; i < vc->maxLength; i++) {
    if (vc->mem[i] && yatc_variable_get_name(vc->mem[i]) && strcmp(yatc_variable_get_name(vc->mem[i]), indx) == 0)
      meetingReq = vc->mem[i];
  }
  return meetingReq;
}

void yatc_vector_copy(YatcVector* v1, YatcVector* v2) {
  if (!v1 || !v2)
    return;
  unsigned index = 0;
  for (unsigned i = 0; i < v2->maxLength; i++) {
    if (v2->mem[i] && strcmp(yatc_variable_get_name(v2->mem[i]), "") != 0)
      v1->mem[index] = yatc_variable_clone(v2->mem[i]);
  }
}

void yatc_vector_goodbye(YatcVector* vc) {
  for (unsigned i = 0; i < vc->maxLength; i++) {
    free(vc->mem[i]);
    vc->mem[i] = NULL;
  }
  free(vc->mem);
  vc->mem = NULL;
  free(vc);
}


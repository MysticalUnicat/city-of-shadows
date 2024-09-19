#ifndef inline_list_h_INCLUDED
#define inline_list_h_INCLUDED

#include <stdbool.h>

typedef struct InlineList {
  struct InlineList * prev;
  struct InlineList * next;
} InlineList;

#define INLINE_LIST_INIT(X) { .prev = &X, .next = &X }

#define INLINE_LIST_CONTAINER(ITEM, TYPE, MEMBER) ((TYPE *)((unsigned char *)(ITEM)) - offsetof(TYPE, MEMBER))

#define INLINE_LIST_EACH(LIST, ITEM) \
  for(ITEM = (LIST)->next; ITEM != (LIST); ITEM = ITEM->next)

#define INLINE_LIST_EACH_CONTAINER(LIST, ITEM, MEMBER)                        \
  for( ITEM = INLINE_LIST_CONTAINER((LIST)->next, typeof(*ITEM), MEMBER)      \
     ; &ITEM->MEMBER != (LIST)                                                      \
     ; ITEM = INLINE_LIST_CONTAINER(ITEM->MEMBER.next, typeof(*ITEM), MEMBER) \
     )

#define INLINE_LIST_EACH_CONTAINER_SAFE(LIST, ITEM, NEXT, MEMBER)             \
  for( ITEM = INLINE_LIST_CONTAINER((LIST)->next, typeof(*ITEM), MEMBER)      \
     , NEXT = INLINE_LIST_CONTAINER(ITEM->MEMBER.next, typeof(*ITEM), MEMBER) \
     ; &ITEM->MEMBER != (LIST)                                                      \
     ; ITEM = NEXT                                                                  \
     , NEXT = INLINE_LIST_CONTAINER(NEXT->MEMBER.next, typeof(*ITEM), MEMBER) \
     )

static inline void InlineList_init(InlineList * list) {
  list->next = list;
  list->prev = list;
}

static inline bool InlineList_is_empty(InlineList * list) {
  return list->next == list->prev && list->next == list;
}

static inline void InlineList_remove_self(InlineList * item) {
  item->next->prev = item->prev;
  item->prev->next = item->next;
}

static inline void InlineList_push(InlineList * list, InlineList * item) {
  InlineList * prev = list->prev, * next = list;
  next->prev = item;
  prev->next = item;
  item->next = next;
  item->prev = prev;
}

static inline void InlineList_unshift(InlineList * list, InlineList * item) {
  InlineList * prev = list, * next = list->next;
  next->prev = item;
  prev->next = item;
  item->next = next;
  item->prev = prev;
}

static inline InlineList * InlineList_pop(InlineList * list) {
  InlineList * item = list->prev;
  InlineList_remove_self(item);
  InlineList_init(item);
  return item;
}

static inline InlineList * InlineList_shift(InlineList * list) {
  InlineList * item = list->next;
  InlineList_remove_self(item);
  InlineList_init(item);
  return item;
}

static inline void InlineList_push_list(InlineList * list, InlineList * other) {
  InlineList * prev = list->prev;
  InlineList * next = list;
  InlineList * frst = other->next;
  InlineList * last = other->prev;
  frst->prev = prev;
  prev->next = frst;
  last->next = next;
  next->prev = last;
}

static inline void InlineList_unshift_list(InlineList * list, InlineList * other) {
  InlineList * prev = list;
  InlineList * next = list->next;
  InlineList * frst = other->next;
  InlineList * last = other->prev;
  prev->next = frst;
  frst->prev = prev;
  next->prev = last;
  last->next = next;
}

#endif // inline_list_h_INCLUDED

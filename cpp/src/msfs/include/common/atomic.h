#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#define ATOMIC_ADD(src_ptr, v)            (void)__sync_add_and_fetch(src_ptr, v)
#define ATOMIC_SUB_AND_FETCH(src_ptr, v)  __sync_sub_and_fetch(src_ptr, v)
#define ATOMIC_ADD_AND_FETCH(src_ptr, v)  __sync_add_and_fetch(src_ptr, v)
#define ATOMIC_FETCH(src_ptr)             __sync_add_and_fetch(src_ptr, 0)
#define ATOMIC_SET(src_ptr, v)            (void)__sync_bool_compare_and_swap(src_ptr, *(src_ptr), v)

typedef  volatile long atomic_t;

#endif


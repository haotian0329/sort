#ifndef INCLUDE_QSORT_ARG_H_
#define INCLUDE_QSORT_ARG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))

typedef int (*qsort_arg_comparator)(const void *a, const void *b, void *arg);

void
qsort_arg(void *a, size_t n, size_t es, qsort_arg_comparator cmp, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_QSORT_ARG_H_ */

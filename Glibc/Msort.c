/* An alternative to qsort, with an identical interface.
   This file is part of the GNU C Library.
   Copyright (C) 1992,95-97,99,2000,01,02,04,07 Free Software Foundation, Inc.
   Written by Mike Haertel, September 1988.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <alloca.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memcopy.h>
#include <errno.h>

struct msort_param
{
  size_t s;
  size_t var;
  __compar_d_fn_t cmp;
  void *arg;
  char *t;
};
static void msort_with_tmp (const struct msort_param *p, void *b, size_t n);

static void
msort_with_tmp (const struct msort_param *p, void *b, size_t n)
{
  char *b1, *b2;
  size_t n1, n2;

  if (n <= 1)
    return;

  n1 = n / 2;
  n2 = n - n1;
  b1 = b;
  b2 = (char *) b + (n1 * p->s);

  msort_with_tmp (p, b1, n1);
  msort_with_tmp (p, b2, n2);

  char *tmp = p->t;
  const size_t s = p->s;
  __compar_d_fn_t cmp = p->cmp;
  void *arg = p->arg;
  switch (p->var)
    {
    case 0:
      while (n1 > 0 && n2 > 0)
	{
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      *(uint32_t *) tmp = *(uint32_t *) b1;
	      b1 += sizeof (uint32_t);
	      --n1;
	    }
	  else
	    {
	      *(uint32_t *) tmp = *(uint32_t *) b2;
	      b2 += sizeof (uint32_t);
	      --n2;
	    }
	  tmp += sizeof (uint32_t);
	}
      break;
    case 1:
      while (n1 > 0 && n2 > 0)
	{
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      *(uint64_t *) tmp = *(uint64_t *) b1;
	      b1 += sizeof (uint64_t);
	      --n1;
	    }
	  else
	    {
	      *(uint64_t *) tmp = *(uint64_t *) b2;
	      b2 += sizeof (uint64_t);
	      --n2;
	    }
	  tmp += sizeof (uint64_t);
	}
      break;
    case 2:
      while (n1 > 0 && n2 > 0)
	{
	  unsigned long *tmpl = (unsigned long *) tmp;
	  unsigned long *bl;

	  tmp += s;
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      bl = (unsigned long *) b1;
	      b1 += s;
	      --n1;
	    }
	  else
	    {
	      bl = (unsigned long *) b2;
	      b2 += s;
	      --n2;
	    }
	  while (tmpl < (unsigned long *) tmp)
	    *tmpl++ = *bl++;
	}
      break;
    case 3:
      while (n1 > 0 && n2 > 0)
	{
	  if ((*cmp) (*(const void **) b1, *(const void **) b2, arg) <= 0)
	    {
	      *(void **) tmp = *(void **) b1;
	      b1 += sizeof (void *);
	      --n1;
	    }
	  else
	    {
	      *(void **) tmp = *(void **) b2;
	      b2 += sizeof (void *);
	      --n2;
	    }
	  tmp += sizeof (void *);
	}
      break;
    default:
      while (n1 > 0 && n2 > 0)
	{
	  if ((*cmp) (b1, b2, arg) <= 0)
	    {
	      tmp = (char *) __mempcpy (tmp, b1, s);
	      b1 += s;
	      --n1;
	    }
	  else
	    {
	      tmp = (char *) __mempcpy (tmp, b2, s);
	      b2 += s;
	      --n2;
	    }
	}
      break;
    }

  if (n1 > 0)
    memcpy (tmp, b1, n1 * s);
  memcpy (b, p->t, (n - n2) * s);
}


void
qsort_r (void *b, size_t n, size_t s, __compar_d_fn_t cmp, void *arg)
{
  size_t size = n * s;
  char *tmp = NULL;
  struct msort_param p;

  /* For large object sizes use indirect sorting.  */
  if (s > 32)
    size = 2 * n * sizeof (void *) + s;
  //如果单个元素很大(>32bytes)，则用间接的排序，需要内存更多
  //间接排序的意思是，交换的时候只交换指针，最后再把元素按照
  //排好的指针顺序拷贝。这样可以减少大量的拷贝时间。

  if (size < 1024)//对于小数组，用alloca，在栈上分配临时内存
    /* The temporary array is small, so put it on the stack.  */
    p.t = __alloca (size);
  else
    {
      //获取一些系统内存的参数。如果消耗太多空间，可能会用
      //到交换空间的虚拟内存，使得性能急剧下降
      /* We should avoid allocating too much memory since this might
	 have to be backed up by swap space.  */
      static long int phys_pages;//物理内存页数
      static int pagesize; //页大小(字节/页)

      if (phys_pages == 0)
	{
	  phys_pages = __sysconf (_SC_PHYS_PAGES);
	  //__sysconf函数在sysdeps/posix/sysconf.c中
	  //_SC_PHYS_PAGES对应到函数__get_phys_pages()
	  //位于文件sysdeps/unix/sysv/linux/getsysstats.c中
	  //通过phys_pages_info()打开/proc/meminfo来读取内存信息
	  //(这就定位到了qsort打开文件的问题)

	  if (phys_pages == -1)
	    /* Error while determining the memory size.  So let's
	       assume there is enough memory.  Otherwise the
	       implementer should provide a complete implementation of
	       the `sysconf' function.  */
	    phys_pages = (long int) (~0ul >> 1);

	  /* The following determines that we will never use more than
	     a quarter of the physical memory.  */
	  phys_pages /= 4;

	  pagesize = __sysconf (_SC_PAGESIZE);
	}
	  //注意，上面这一段if会产生竞争，出现线程安全安全：
	  //如果两个线程都调用qsort，当线程1获取了phys_pages之后，线程2
	  //才到达if，线程2就会跳过这一段代码，直接执行下面的if语句——
	  //而此时pagesize尚未初始化（=0），于是就会出现除零错误，导致
	  //core dump

      /* Just a comment here.  We cannot compute
	   phys_pages * pagesize
	   and compare the needed amount of memory against this value.
	   The problem is that some systems might have more physical
	   memory then can be represented with a `size_t' value (when
	   measured in bytes.  */

      /* If the memory requirements are too high don't allocate memory.  */
	  //如果所需的内存页数大于总的可用内存，则不分配内存(防止swap降低性能)
      if (size / pagesize > (size_t) phys_pages)
	{
	 //直接使用stdlib/qsort.c中的 _quicksort 进行排序
	  _quicksort (b, n, s, cmp, arg);
	  return;
	}

      /* It's somewhat large, so malloc it.  */
	  //内存大于阈值（1024），小于可用内存，因此应当用malloc，在堆上分配
      int save = errno;
      tmp = malloc (size);
      __set_errno (save);
      if (tmp == NULL)
	{
	  /* Couldn't get space, so use the slower algorithm
	     that doesn't need a temporary array.  */
	  //分配内存失败，使用 _quicksort 进行排序
	  _quicksort (b, n, s, cmp, arg);
	  return;
	}
      p.t = tmp;
    }

  p.s = s;
  p.var = 4;
  //这个属性决定在排序时如何最大化利用CPU来拷贝单个元素，默认是逐字节拷贝。
  p.cmp = cmp;
  p.arg = arg;

  if (s > 32)
    {
      /* Indirect sorting.  */
	  //间接排序：先把各个元素的指针进行排序，然后再将元素逐个拷贝
      char *ip = (char *) b;
      void **tp = (void **) (p.t + n * sizeof (void *));
      void **t = tp;
      void *tmp_storage = (void *) (tp + n);

      while ((void *) t < tmp_storage)
	{
	  *t++ = ip;
	  ip += s;
	}
      p.s = sizeof (void *);
      p.var = 3;
      msort_with_tmp (&p, p.t + n * sizeof (void *), n);

      /* tp[0] .. tp[n - 1] is now sorted, copy around entries of
	 the original array.  Knuth vol. 3 (2nd ed.) exercise 5.2-10.  */
      char *kp;
      size_t i;
      for (i = 0, ip = (char *) b; i < n; i++, ip += s)
	if ((kp = tp[i]) != ip)
	  {
	    size_t j = i;
	    char *jp = ip;
	    memcpy (tmp_storage, ip, s);

	    do
	      {
		size_t k = (kp - (char *) b) / s;
		tp[j] = jp;
		memcpy (jp, kp, s);
		j = k;
		jp = kp;
		kp = tp[k];
	      }
	    while (kp != ip);

	    tp[j] = jp;
	    memcpy (jp, tmp_storage, s);
	  }
    }
  else
    {
      if ((s & (sizeof (uint32_t) - 1)) == 0
	  && ((char *) b - (char *) 0) % __alignof__ (uint32_t) == 0)
	{
	//如果元素是sizeof(uint32_t)的倍数，且数组是uint32_t对其的
    //那么可以充分利用内存总线，一次拷贝4个、8个字节。
	  if (s == sizeof (uint32_t))
	    p.var = 0;//使用uint32_t来赋值拷贝，一次4个字节
	  else if (s == sizeof (uint64_t)
		   && ((char *) b - (char *) 0) % __alignof__ (uint64_t) == 0)
	    p.var = 1;//uint64_t，一次64个字节
	  else if ((s & (sizeof (unsigned long) - 1)) == 0
		   && ((char *) b - (char *) 0)
		      % __alignof__ (unsigned long) == 0)
	    p.var = 2; //一次sizeof(ulong)个字节, x86=4, x86_64=8
	}
      msort_with_tmp (&p, b, n);//递归地排序
    }
  free (tmp);
}
libc_hidden_def (qsort_r)


void
qsort (void *b, size_t n, size_t s, __compar_fn_t cmp)
{
  return qsort_r (b, n, s, (__compar_d_fn_t) cmp, NULL);
}
libc_hidden_def (qsort)

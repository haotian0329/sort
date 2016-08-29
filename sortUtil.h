#ifndef SORTUTIL_H_
#define SORTUTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//冒泡排序
void bubbleSort(int* array, int num);

//选择排序
void selectionSort(int* array, int num);

//插入排序
void insertSort(int* array, int num);

//归并排序
void mergeSort(int* array, int start, int end);

//快速排序
void quickSort(int* array , int start, int end);

//堆排序
void heapSort(int* array, int heapSize);

#endif /* SORTUTIL_H_ */

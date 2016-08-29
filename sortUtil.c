#include "sortUtil.h"

//交换data1和data2所指向的整形
void dataSwap(int* data1, int* data2) {
	int temp = *data1;
	*data1 = *data2;
	*data2 = temp;
}

/********************************************************
 *函数名称：bubbleSort
 *参数说明：array 无序数组
 *       num 无序数据个数
 *说明：    冒泡排序
 *********************************************************/
void bubbleSort(int* array, int num) {
	int i,j;
	for(i=0;i<num;i++){
		for(j=0;j<num-i-1;j++){
			if(array[j]>array[j+1])//始终比较相邻的2个值
				dataSwap(&array[j],&array[j+1]);
		}
	}
}

/********************************************************
 *函数名称：selectionSort
 *参数说明：array 无序数组
 *       num 无序数据个数
 *说明：    选择排序
 *********************************************************/
void selectionSort(int* array, int num){
	int i,j,index;
	for(i=0;i<num;i++){
		index=i;
		for(j=i+1;j<num;j++){//寻找最小的数据索引
			if(array[j]<array[index])
				index=j;
		}
		if(index!=i)//如果最小数位置变化则交换
			dataSwap(&array[index],&array[i]);
	}
}

/********************************************************
 *函数名称：insertSort
 *参数说明：array 无序数组
 *       num 无序数据个数
 *说明：    插入排序
 *********************************************************/
void insertSort(int* array, int num){
	int i,j,tmp;
	for(i=1;i<num;i++){//从第2个数据开始插入
		j=i-1;
		tmp=array[i];
		while(j>=0 && array[j]>tmp){//i位置之前，有比array[i]大的数，则集体后移
			array[j+1]=array[j];
			j--;
		}
		array[j+1]=tmp;//插入
	}
}

/********************************************************
 *函数名称：merge
 *参数说明：array 有序数组；
 *       start 需要合并的序列1的起始位置
 *       middle 需要合并的序列1的结束位置,并且作为序列2的起始位置
 *       end 需要合并的序列2的结束位置
 *说明：    将数组中连续的两个有序子序列合并为一个有序序列
 *********************************************************/
void merge(int* array, int start, int middle, int end){
	int *tmpArray=(int *)malloc(sizeof(int)*(end-start+1));
	int i=start,j=middle+1,k=0;

	while(i<=middle&&j<=end){//两两比较合并
		if(array[i]<array[j]){
			tmpArray[k]=array[i];
			i++;
		}else{
			tmpArray[k]=array[j];
			j++;
		}
		k++;
	}

	while(i<=middle){//第一个序列还有剩余
		tmpArray[k]=array[i];
		k++;
		i++;
	}

	while(j<=end){//第二个序列还有剩余
		tmpArray[k]=array[j];
		k++;
		j++;
	}

	for(i=0;i<end-start+1;i++){//更新合并后的有序序列
		array[start+i]=tmpArray[i];
	}
	free(tmpArray);

}

/********************************************************
 *函数名称：mergeSort
 *参数说明：array 无序数组
 *       start 无序数组开始下标
 *       end 无序数组结束下标
 *说明：    自底向上的归并排序
 *********************************************************/
void mergeSort(int* array, int start, int end){
	int middle;
	if(start<end){
		middle=(start+end)/2;
		mergeSort(array,start,middle);
		mergeSort(array,middle+1,end);
		merge(array,start,middle,end);
	}
}

/********************************************************
 *函数名称：partion
 *参数说明：array 无序数组
 *		 start为array需要快速排序的起始位置
 *       end为array需要快速排序的结束位置
 *函数返回：分割后的主元位置
 *说明：    以end处的数值key作为主元，使其前半段小于key，后半段大于key
 *********************************************************/
int partion(int* array , int start, int end){
	int key=array[end];//取最后一个值为主元
	int index;
	int small=start-1;//标记小于主元的最后一个元素的下标索引
	for(index=start;index<=end-1;index++){
		if(array[index]<=key){
			small++;
			dataSwap(&array[index],&array[small]);
		}
	}
	dataSwap(&array[end],&array[small+1]);
	return small+1;
}

/********************************************************
 *函数名称：quickSort
 *参数说明：array 无序数组
 *		 start为array需要快速排序的起始位置
 *       end为array需要快速排序的结束位置
 *说明：    快速排序递归函数
 *********************************************************/
void quickSort(int* array , int start, int end){
	int middle;
	if(start<end){
		middle=partion(array,start,end);
		quickSort(array,start,middle-1);//注意不包含middle
		quickSort(array,middle+1,end);
	}
}

/********************************************************
 *函数名称：maxHeapify
 *参数说明：array 无序数组
 *		 index  要维护堆的下标位置
 *       heapSize 堆的大小
 *说明：    维护最大堆的性质
 *********************************************************/
void maxHeapify(int* array, int index, int heapSize){
	int left=2*index+1;//左节点
	int right=left+1;//右节点
	int max=index;
	if(left<heapSize&&array[left]>array[max])
		max=left;
	if(right<heapSize&&array[right]>array[max])
		max=right;
	if(max!=index){
		dataSwap(&array[max],&array[index]);
		maxHeapify(array,max,heapSize);//递归检查
	}
}

/********************************************************
 *函数名称：bulidHeap
 *参数说明：array 无序数组
 *       heapSize 堆的大小
 *说明：    建立最大堆
 *********************************************************/
void bulidHeap(int* array, int heapSize){
	int index;
	for(index=heapSize/2-1;index>=0;index--){//只需从最下面的非叶子节点开始向上维护即可
		maxHeapify(array,index,heapSize);
	}
}

/********************************************************
 *函数名称：heapSort
 *参数说明：array 无序数组
 *       heapSize 堆的大小
 *说明：    堆排序
 *********************************************************/
void heapSort(int* array, int heapSize){
	int index;
	bulidHeap(array,heapSize);//建立最大堆
	for(index=heapSize-1;index>0;index--){
		dataSwap(&array[index],&array[0]);//第一个元素(当前堆最大值)与最后一个元素交换
		heapSize--;//堆大小减一
		maxHeapify(array,0,heapSize);//重新维护堆，只有第一个值不满足条件
	}
}


int main() {
	int array[]={2,4,6,7,1,5,3,10,8,9};
	int num=10,i;
//	bubbleSort(array,num);
//	selectionSort(array,num);
//	insertSort(array,num);
//	mergeSort(array,0,6);
//	quickSort(array,0,6);
	heapSort(array,num);
	for(i=0;i<num;i++){
		printf("%d ",array[i]);
	}
	return 0;
}

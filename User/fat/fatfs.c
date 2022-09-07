﻿#include "fatfs.h"

FATFS fs;													/* FatFs文件系统对象 */
FIL fnew;													/* 文件对象 */
FRESULT res_sd; 			   /* 文件操作结果 */
UINT fnum;								  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};		  /* 读缓冲区 */
BYTE WriteBuffer[] =			  /* 写缓冲区*/
"欢迎使用野火STM32 开发板 今天是个好日子，新建文件系统测试文件\r\n";  

void Fatfs_Init(void){

	FF_INFO("\r\n****** 这是一个SD卡 文件系统实验 ******\r\n");
	//在外部SD卡挂载文件系统，文件系统挂载时会对SDIO设备初始化
	res_sd = f_mount(&fs,"0:",1);
		
	/*----------------------- 格式化测试 ---------------------------*/  
	/* 如果没有文件系统就格式化创建创建文件系统 */
	if(res_sd == FR_NO_FILESYSTEM)
	{
		FF_INFO("》SD卡还没有文件系统，即将进行格式化...\r\n");
		/* 格式化 */
		res_sd=f_mkfs("0:",0,0);							
	
		if(res_sd == FR_OK)
		{
			FF_INFO("》SD卡已成功格式化文件系统。\r\n");
		  /* 格式化后，先取消挂载 */
			res_sd = f_mount(NULL,"0:",1);			
		  /* 重新挂载	*/			
			res_sd = f_mount(&fs,"0:",1);
		} else {
			LED_RED;
			FF_ERROR("《《格式化失败。》》\r\n");
			//while(1);
		}
	} else if (res_sd!=FR_OK)
	{
		FF_ERROR("！！SD卡挂载文件系统失败。(%d)\r\n",res_sd);
		FF_ERROR("！！可能原因：SD卡初始化不成功。\r\n");
		//while(1);
	} else {
		FF_INFO("》文件系统挂载成功，可以进行读写测试\r\n");
	}
}

void Fatfs_Write_Test(void){

	/*----------------------- 文件系统测试：写测试 -----------------------------*/
		/* 打开文件，如果文件不存在则创建它 */
	FF_INFO("\r\n****** 即将进行文件写入测试... ******\r\n");	
	res_sd = f_open(&fnew, "0:FatFs读写测试文件.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_sd == FR_OK )
	{
		FF_INFO("》打开/创建FatFs读写测试文件.txt文件成功，向文件写入数据。\r\n");
	/* 将指定存储区内容写入到文件内 */
		res_sd=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
		if(res_sd==FR_OK)
		{
		  FF_INFO("》文件写入成功，写入字节数据：%d\n",fnum);
		  FF_INFO("》向文件写入的数据为：\r\n%s\r\n",WriteBuffer);
		} else {
			FF_ERROR("！！文件写入失败：(%d)\n",res_sd);
		}	 
		/* 不再读写，关闭文件 */
		f_close(&fnew);
	}
	else {	
		LED_RED;
		FF_ERROR("！！打开/创建文件失败。\r\n");
	}
}

void Fatfs_Read_Test(void){

	/*------------------- 文件系统测试：读测试 ------------------------------------*/
	FF_INFO("****** 即将进行文件读取测试... ******\r\n");
	res_sd = f_open(&fnew, "0:FatFs读写测试文件.txt", FA_OPEN_EXISTING | FA_READ);	 
	if(res_sd == FR_OK)
	{
		LED_GREEN;
		FF_INFO("》打开文件成功。\r\n");
		res_sd = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
		if (res_sd==FR_OK)
		{
		  FF_INFO("》文件读取成功,读到字节数据：%d\r\n",fnum);
		  FF_INFO("》读取得的文件数据为：\r\n%s \r\n", ReadBuffer);	
		} else {
		  FF_ERROR("！！文件读取失败：(%d)\n",res_sd);
		}		
	} else {
		LED_RED;
		FF_ERROR("！！打开文件失败。\r\n");
	}
	/* 不再读写，关闭文件 */
	f_close(&fnew);  
	/* 不再使用文件系统，取消挂载文件系统 */
	f_mount(NULL,"0:",1);
	//while(1);

}


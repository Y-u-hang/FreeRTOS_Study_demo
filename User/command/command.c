#include "FreeRTOS.h"	
#include "task.h"
#include "command.h"

typedef struct Command_Function{
	char* Command;
	void (*pFunction)();
	
}Com_Fun;
	
	
#define COM_MAX 1
const Com_Fun gCom_Fun[COM_MAX] = {
	// COMMAND FUNCTION
	"CPU", CPU_Percent,
};


extern char Usart_Rx_Buf[USART_RBUFF_SIZE];

bool RunComFun(){
	int i = 0;
	for(i = 0; i < COM_MAX; i++) {	
		if(strcmp(gCom_Fun[i].Command, Usart_Rx_Buf) == 0){
			gCom_Fun[i].pFunction();
			memset(Usart_Rx_Buf, 0, USART_RBUFF_SIZE);/* 清零 */
			return true;
		}
	}
	printf("NO this command !!\n");
	return false;
}


void CPU_Percent(void)
{
	
	uint8_t CPU_RunInfo[400];	  //保存任务运行时间信息
	
	memset(CPU_RunInfo,0,400);				//信息缓冲区清零
		
	vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
	
	printf("---------------------------------------------\r\n");
	printf("任务名	   任务状态 优先级	剩余栈 任务序号\r\n");
	printf("%s", CPU_RunInfo);
	printf("---------------------------------------------\r\n");
	
	memset(CPU_RunInfo,0,400);				//信息缓冲区清零
	
	vTaskGetRunTimeStats((char *)&CPU_RunInfo);
	
	printf("任务名		运行计数		 使用率\r\n");
	printf("%s", CPU_RunInfo);
	printf("---------------------------------------------\r\n\n");


}



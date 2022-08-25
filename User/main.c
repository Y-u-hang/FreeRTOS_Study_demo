/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-霸道 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "stm32f10x.h"
#include "bsp_led.h"
#include "FreeRTOS.h"					//FreeRTOS使用		  
#include "task.h" 
#include "bsp_usart.h"
#include "bsp_key.h" 
#include "queue.h"
#include "semphr.h"
/******************************* 宏定义 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些宏定义。
 */
#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
// /* 创建任务句柄 */
//static TaskHandle_t AppTaskCreate_Handle;
///* LED任务句柄 */
//static TaskHandle_t LED_Task_Handle;		

 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* LED任务句柄 */
static TaskHandle_t LED_Task_Handle = NULL;

static TaskHandle_t LED_Task_Handle2 = NULL;

static TaskHandle_t KEY_Task_Handle = NULL;/* KEY任务句柄 */

static TaskHandle_t Receive_Task_Handle = NULL;

static TaskHandle_t Send_Task_Handle = NULL;

static TaskHandle_t Take_Task_Handle = NULL;

static TaskHandle_t Give_Task_Handle = NULL;





/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */
QueueHandle_t Test_Queue =NULL;
SemaphoreHandle_t BinarySem_Handle =NULL;
SemaphoreHandle_t CountSem_Handle =NULL;


/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */
//	静态任务创建
///* AppTaskCreate任务任务堆栈 */
//static StackType_t AppTaskCreate_Stack[128];
///* LED任务堆栈 */
//static StackType_t LED_Task_Stack[128];
//
///* AppTaskCreate 任务控制块 */
//static StaticTask_t AppTaskCreate_TCB;
///* LED 任务控制块 */
//static StaticTask_t LED_Task_TCB;
//
//
///* 空闲任务任务堆栈 */
//static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
///* 定时器任务堆栈 */
//static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];
//
///* 空闲任务控制块 */
//static StaticTask_t Idle_Task_TCB;	
///* 定时器任务控制块 */
//static StaticTask_t Timer_Task_TCB;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/

static void AppTaskCreate(void);
static void LED_Task(void* parameter);
static void LED_Task2(void* parameter);
static void BSP_Init(void);
static void KEY_Task(void* pvParameters);/* KEY_Task任务实现 */
static void Receive_Task(void* parameter);
static void Send_Task(void* parameter);
static void Take_Task(void* parameter);
static void Give_Task(void* parameter);


/**
	* 使用了静态分配内存，以下这两个函数是由用户实现，函数在task.c文件中有引用
	*	当且仅当 configSUPPORT_STATIC_ALLOCATION 这个宏定义为 1 的时候才有效
	*/
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
																		StackType_t **ppxTimerTaskStackBuffer, 
																		uint32_t *pulTimerTaskStackSize);

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
																	 StackType_t **ppxIdleTaskStackBuffer, 
																	 uint32_t *pulIdleTaskStackSize);

int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

  /* 开发板硬件初始化 */
  BSP_Init();
   /* 创建 AppTaskCreate 任务 */
// 	静态创建任务
//	AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t	)AppTaskCreate,		//任务函数
//															(const char* 	)"AppTaskCreate",		//任务名称
//															(uint32_t 		)128,	//任务堆栈大小
//															(void* 		  	)NULL,				//传递给任务函数的参数
//															(UBaseType_t 	)3, 	//任务优先级
//															(StackType_t*   )AppTaskCreate_Stack,	//任务堆栈
//															(StaticTask_t*  )&AppTaskCreate_TCB);	//任务控制块   
//
//															
//	if(NULL != AppTaskCreate_Handle)/* 创建成功 */
//    vTaskStartScheduler();   /* 启动任务，开启调度 */
	

	
	/* 开发板硬件初始化 */
	BSP_Init();
	//printf("这是一个[野火]-STM32全系列开发板-FreeRTOS-动态创建任务!\r\n");
	printf("这是一个野火开发板!r\n");
	 /* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
						  (const char*	  )"AppTaskCreate",/* 任务名字 */
						  (uint16_t 	  )512,  /* 任务栈大小 */
						  (void*		  )NULL,/* 任务入口函数参数 */
						  (UBaseType_t	  )1, /* 任务的优先级 */
						  (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 

	/* 启动任务调度 */		   
	// https://blog.csdn.net/zhoutaopower/article/details/107034995 啓動流程可以參照該鏈接
	if(pdPASS == xReturn)
	  vTaskStartScheduler();   /* 启动任务，开启调度 */
	else
	  return -1;  
	
	while(1);   /* 正常不会执行到这里 */    
}


/*********************************************END OF FILE**********************/

/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED 初始化 */
	LED_GPIO_Config();

	/* 串口初始化	*/
	USART_Config();

	Key_GPIO_Config();
	//LED1_ON;
	//printf(" yuahng usart init ok!\n");
}

/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */

  taskENTER_CRITICAL();           //进入临界区

  /* 创建LED_Task任务 */
//  静态创建任务
//	LED_Task_Handle = xTaskCreateStatic((TaskFunction_t	)LED_Task,		//任务函数
//															(const char* 	)"LED_Task",		//任务名称
//															(uint32_t 		)128,					//任务堆栈大小
//															(void* 		  	)NULL,				//传递给任务函数的参数
//															(UBaseType_t 	)4, 				//任务优先级
//															(StackType_t*   )LED_Task_Stack,	//任务堆栈
//															(StaticTask_t*  )&LED_Task_TCB);	//任务控制块   
//	
//	if(NULL != LED_Task_Handle)/* 创建成功 */
//		printf("LED_Task任务创建成功!\n");
//	else
//		printf("LED_Task任务创建失败!\n");

  /* 创建 BinarySem */
  BinarySem_Handle = xSemaphoreCreateBinary();	 
  if(NULL != BinarySem_Handle)
    printf("BinarySem_Handle二值信号量创建成功!\r\n");

  xReturn = xTaskCreate((TaskFunction_t )LED_Task, /* 任务入口函数 */
                        (const char*    )"LED_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )5,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&LED_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建LED_Task任务成功!\r\n");

  xReturn = xTaskCreate((TaskFunction_t )LED_Task2, /* 任务入口函数 */
                        (const char*    )"LED_Task2",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )5,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&LED_Task_Handle2);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建LED_Task2任务成功!\r\n");
  /* 创建KEY_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* 任务入口函数 */
                        (const char*    )"KEY_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )2, /* 任务的优先级 */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建KEY_Task任务成功!\r\n");

  /* 创建Test_Queue */
   Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
							 (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
   if(NULL != Test_Queue)
	 printf("创建Test_Queue消息队列成功!\r\n");
   
   /* 创建Receive_Task任务 */
   xReturn = xTaskCreate((TaskFunction_t )Receive_Task, /* 任务入口函数 */
						 (const char*	 )"Receive_Task",/* 任务名字 */
						 (uint16_t		 )512,	 /* 任务栈大小 */
						 (void* 		 )NULL,  /* 任务入口函数参数 */
						 (UBaseType_t	 )2,	 /* 任务的优先级 */
						 (TaskHandle_t*  )&Receive_Task_Handle);/* 任务控制块指针 */
   if(pdPASS == xReturn)
	 printf("创建Receive_Task任务成功!\r\n");
   
   /* 创建Send_Task任务 */
   xReturn = xTaskCreate((TaskFunction_t )Send_Task,  /* 任务入口函数 */
						 (const char*	 )"Send_Task",/* 任务名字 */
						 (uint16_t		 )512,	/* 任务栈大小 */
						 (void* 		 )NULL,/* 任务入口函数参数 */
						 (UBaseType_t	 )3, /* 任务的优先级 */
						 (TaskHandle_t*  )&Send_Task_Handle);/* 任务控制块指针 */ 
   if(pdPASS == xReturn)
	 printf("创建Send_Task任务成功!\n\n");
  
  /* 创建Test_Queue */
  CountSem_Handle = xSemaphoreCreateCounting(5,5);	 
  if(NULL != CountSem_Handle)
    printf("CountSem_Handle计数信号量创建成功!\r\n");

  /* 创建Take_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Take_Task, /* 任务入口函数 */
                        (const char*    )"Take_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )4,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Take_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建Take_Task任务成功!\r\n");
  
  /* 创建Give_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Give_Task,  /* 任务入口函数 */
                        (const char*    )"Give_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )5, /* 任务的优先级 */
                        (TaskHandle_t*  )&Give_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建Give_Task任务成功!\n\n");
  

  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  vTaskDelete(LED_Task_Handle2);
  vTaskDelete(LED_Task_Handle);
  
  taskEXIT_CRITICAL();            //退出临界区
}

/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void LED_Task(void* parameter)
{	
    while (1)
    {
        LED1_ON;
        vTaskDelay(500);   /* 延时500个tick */
        printf("LED_Task Running,LED1_ON\r\n");
		printf("time : %d\r\n", xTaskGetTickCount());
        
        LED1_OFF;     
        vTaskDelay(500);   /* 延时500个tick */		 		
        printf("LED_Task Running,LED1_OFF\r\n");
		printf("time : %d\r\n", xTaskGetTickCount());
    }
}

static void LED_Task2(void* parameter)
{	
	static TickType_t pxPreviousWakeTime;	// 延时参数
	static TickType_t const xTimeIncrement = pdMS_TO_TICKS(1000);
	pxPreviousWakeTime = xTaskGetTickCount();
    while (1)
    {
        LED2_ON;
        // vTaskDelay(1000);   /* 延时500个tick */
		vTaskDelayUntil(&pxPreviousWakeTime, xTimeIncrement);
        printf("LED_Task2 Running,LED2_ON\r\n");
		printf("time : %d\r\n", xTaskGetTickCount());
        
        LED2_OFF;     
        //vTaskDelay(1000);   /* 延时500个tick */
		vTaskDelayUntil(&pxPreviousWakeTime, xTimeIncrement);
        printf("LED_Task2 Running,LED2_OFF\r\n");
		printf("time : %d\r\n", xTaskGetTickCount());
    }
}

static void KEY_Task(void* parameter)
{	
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )
    {/* K1 被按下 */
      printf("挂起LED 任务！\r\n");
      vTaskSuspend(LED_Task_Handle2);/* 挂起LED任务 */
      printf("挂起LED任务成功！\r\n");
	  /*-----------------------------------------------------*/
	  
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {/* K2 被按下 */
      printf("恢复LED任务！\r\n");
      vTaskResume(LED_Task_Handle2);/* 恢复LED任务！ */
      printf("恢复LED任务成功！\r\n");
    }
    vTaskDelay(20);/* 延时20个tick */
  }
}

/**********************************************************************
  * @ 函数名  ： Receive_Task
  * @ 功能说明： Receive_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Receive_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
  uint32_t r_queue;	/* 定义一个接收消息的变量 */
  while (1)
  {
    //获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(BinarySem_Handle,/* 二值信号量句柄 */
                              portMAX_DELAY); /* 等待时间 */

    xReturn = xQueueReceive( Test_Queue,    /* 消息队列的句柄 */
                             &r_queue,      /* 发送的消息内容 */
                             portMAX_DELAY); /* 等待时间 一直等 */
    if(pdTRUE == xReturn){
      
		printf("本次接收到的数据是%d\r\n\n",r_queue);
		//printf("receive time : %d\r\n", xTaskGetTickCount());
    }
    else
      	printf("数据接收出错,错误代码0x%lx\n",xReturn);
  }
}

/**********************************************************************
  * @ 函数名  ： Send_Task
  * @ 功能说明： Send_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Send_Task(void* parameter)
{	 
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  uint32_t send_data1 = 1;
  uint32_t send_data2 = 2;
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )
    {/* K1 被按下 */
      printf("发送消息data1\r\n");
	  for (send_data1 = 0; send_data1 < 4; send_data1++)
		  {
			  xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
									&send_data1,/* 发送的消息内容 */
									0 );		/* 等待时间 0 */
			  if(pdPASS == xReturn)
				printf("消息send_data1 %d发送成功!\n\n",send_data1);
			  //printf("send time : %d\r\n", xTaskGetTickCount());

		  }
	 xReturn = xSemaphoreGive( BinarySem_Handle );//给出二值信号量

    } 

    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {/* K2 被按下 */
      printf("发送消息send_data2！\r\n");
      xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
                            &send_data2,/* 发送的消息内容 */
                            0 );        /* 等待时间 0 */
      if(pdPASS == xReturn) {
        	printf("消息send_data2发送成功!\r\n\n");
	  	    xReturn = xSemaphoreGive( BinarySem_Handle );//给出二值信号量
      }
	  //printf("send time : %d\r\n", xTaskGetTickCount());
    }
	
    vTaskDelay(20);/* 延时20个tick */
  }
}


/**********************************************************************
  * @ 函数名  ： Take_Task
  * @ 功能说明： Take_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Take_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
  /* 任务都是一个无限循环，不能返回 */
  while (1)
  {
    //如果KEY1被单击
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )       
		{
			/* 获取一个计数信号量 */
      xReturn = xSemaphoreTake(CountSem_Handle,	/* 计数信号量句柄 */
                             0); 	/* 等待时间：0 */
			if ( pdTRUE == xReturn ) 
				printf( "KEY1被按下，成功申请到停车位。\n" );
			else
				printf( "KEY1被按下，不好意思，现在停车场已满！\n" );							
		}
		vTaskDelay(20);     //每20ms扫描一次		
  }
}

/**********************************************************************
  * @ 函数名  ： Give_Task
  * @ 功能说明： Give_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Give_Task(void* parameter)
{	 
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
  /* 任务都是一个无限循环，不能返回 */
  while (1)
  {
    //如果KEY2被单击
		if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )       
		{
			/* 获取一个计数信号量 */
      xReturn = xSemaphoreGive(CountSem_Handle);//给出计数信号量                  
			if ( pdTRUE == xReturn ) 
				printf( "KEY2被按下，释放1个停车位。\n" );
			else
				printf( "KEY2被按下，但已无车位可以释放！\n" );							
		}
		vTaskDelay(20);     //每20ms扫描一次	
  }
}

// 静态创建你任务时需要	

///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}
//
///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}

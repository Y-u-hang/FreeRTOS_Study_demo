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
#include "limits.h"
#include "event_groups.h"
// #include "timers.h"
/******************************* 宏定义 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些宏定义。
 */
#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */
/*
 * 当我们在写应用程序的时候，可能需要用到一些宏定义。
 */
#define  USE_CHAR  1  /* 测试字符串的时候配置为 1 ，测试变量配置为 0  */

#define KEY1_EVENT  (0x01 << 0)//设置事件掩码的位0
#define KEY2_EVENT  (0x01 << 1)//设置事件掩码的位1

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


static TaskHandle_t LowPriority_Task_Handle = NULL;
static TaskHandle_t MidPriority_Task_Handle = NULL;
static TaskHandle_t HighPriority_Task_Handle = NULL;

static TaskHandle_t Receive1_Task_Handle = NULL;/* Receive1_Task任务句柄 */
static TaskHandle_t Receive2_Task_Handle = NULL;/* Receive2_Task任务句柄 */
static TaskHandle_t Send_Task_Notify_Handle = NULL;/* Send_Task任务句柄 */


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
SemaphoreHandle_t BinarySem_Handle =NULL;	// 二值信号量
SemaphoreHandle_t CountSem_Handle =NULL;	// 计数信号量
SemaphoreHandle_t MuxSem_Handle =NULL;		// 互斥信号量
static EventGroupHandle_t Event_Handle =NULL;

static TimerHandle_t Swtmr1_Handle =NULL;   /* 软件定时器句柄 */
static TimerHandle_t Swtmr2_Handle =NULL;   /* 软件定时器句柄 */


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

static uint32_t TmrCb_Count1 = 0; /* 记录软件定时器1回调函数执行次数 */
static uint32_t TmrCb_Count2 = 0; /* 记录软件定时器2回调函数执行次数 */

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
static void MuxSemHandleDemo(void);
static void LowPriority_Task(void* parameter);
static void MidPriority_Task(void* parameter);
static void HighPriority_Task(void* parameter);
static void Receive1_Task(void* pvParameters);/* Receive1_Task任务实现 */
static void Receive2_Task(void* pvParameters);/* Receive2_Task任务实现 */
static void Send_Task_Notify(void* pvParameters);/* Send_Task任务实现 */

static void xTaskNotifyDemo(void);

static void xTimerDemo(void);
static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);


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
  /* 创建 Event_Handle */
  Event_Handle = xEventGroupCreate();	 
  if(NULL != Event_Handle)
    printf("Event_Handle 事件创建成功!\r\n");

  xReturn = xTaskCreate((TaskFunction_t )LED_Task, /* 任务入口函数 */
                        (const char*    )"LED_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )9,	    /* 任务的优先级 */
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
                        (UBaseType_t    )10, /* 任务的优先级 */
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
  
	// MuxSemHandleDemo();
	xTaskNotifyDemo();
	xTimerDemo();

	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	vTaskDelete(LED_Task_Handle2);
	// vTaskDelete(LED_Task_Handle);
  
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
	EventBits_t r_event;  /* 定义一个事件接收变量 */
	  /* 任务都是一个无限循环，不能返回 */
	  while (1)
		{
		/*******************************************************************
		 * 等待接收事件标志 
		 * 
		 * 如果xClearOnExit设置为pdTRUE，那么在xEventGroupWaitBits()返回之前，
		 * 如果满足等待条件（如果函数返回的原因不是超时），那么在事件组中设置
		 * 的uxBitsToWaitFor中的任何位都将被清除。 
		 
		 * 如果xClearOnExit设置为pdFALSE，
		 * 则在调用xEventGroupWaitBits()时，不会更改事件组中设置的位。
		 *
		 * xWaitForAllBits如果xWaitForAllBits设置为pdTRUE，则当uxBitsToWaitFor中
		 * 的所有位都设置或指定的块时间到期时，xEventGroupWaitBits()才返回。 
		 * 如果xWaitForAllBits设置为pdFALSE，则当设置uxBitsToWaitFor中设置的任何
		 * 一个位置1 或指定的块时间到期时，xEventGroupWaitBits()都会返回。 
		 * 阻塞时间由xTicksToWait参数指定。		   
		  *********************************************************/
		r_event = xEventGroupWaitBits(Event_Handle,  /* 事件对象句柄 */
									  KEY1_EVENT|KEY2_EVENT,/* 接收线程感兴趣的事件 */
									  pdTRUE,	/* 退出时清除事件位 */
									  pdTRUE,	/* 满足感兴趣的所有事件 */
									  portMAX_DELAY);/* 指定超时事件,一直等 */
							
		if((r_event & (KEY1_EVENT|KEY2_EVENT)) == (KEY1_EVENT|KEY2_EVENT)) 
		{
		  /* 如果接收完成并且正确 */
		  printf ( "KEY1与KEY2都按下\n");		
		  LED1_TOGGLE;		 //LED1 反转
		}
		else
		  printf ( "事件错误！\n");	
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
      //printf("挂起LED 任务！\r\n");
      //vTaskSuspend(LED_Task_Handle2);/* 挂起LED任务 */
      //printf("挂起LED任务成功！\r\n");
	  /* 触发一个事件1 */
	  xEventGroupSetBits(Event_Handle,KEY1_EVENT);		  

	  /*-----------------------------------------------------*/
	  
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {	/* K2 被按下 */
      	//printf("恢复LED任务！\r\n");
      	//vTaskResume(LED_Task_Handle2);/* 恢复LED任务！ */
      	//printf("恢复LED任务成功！\r\n");
		/* 触发一个事件2 */
		xEventGroupSetBits(Event_Handle,KEY2_EVENT);		

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
static void MuxSemHandleDemo(void)
{
	BaseType_t xReturn = pdPASS;
	/* 创建MuxSem */
	MuxSem_Handle = xSemaphoreCreateMutex();	 
	if(NULL != MuxSem_Handle)
		printf("MuxSem_Handle互斥量创建成功!\r\n");
	
	xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量
	if( xReturn == pdTRUE )
		  printf("释放信号量!\r\n");
		
	/* 创建LowPriority_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )LowPriority_Task, /* 任务入口函数 */
							(const char*	)"LowPriority_Task",/* 任务名字 */
							(uint16_t		)512,	/* 任务栈大小 */
							(void*			)NULL,	/* 任务入口函数参数 */
							(UBaseType_t	)2, 	/* 任务的优先级 */
							(TaskHandle_t*	)&LowPriority_Task_Handle);/* 任务控制块指针 */
	if(pdPASS == xReturn)
		printf("创建LowPriority_Task任务成功!\r\n");
	  
	  /* 创建MidPriority_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )MidPriority_Task,	/* 任务入口函数 */
							(const char*	)"MidPriority_Task",/* 任务名字 */
							(uint16_t		)512,  /* 任务栈大小 */
							(void*			)NULL,/* 任务入口函数参数 */
							(UBaseType_t	)3, /* 任务的优先级 */
							(TaskHandle_t*	)&MidPriority_Task_Handle);/* 任务控制块指针 */ 
	if(pdPASS == xReturn)
		printf("创建MidPriority_Task任务成功!\n");
	  
	  /* 创建HighPriority_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )HighPriority_Task,  /* 任务入口函数 */
							(const char*	)"HighPriority_Task",/* 任务名字 */
							(uint16_t		)512,  /* 任务栈大小 */
							(void*			)NULL,/* 任务入口函数参数 */
							(UBaseType_t	)4, /* 任务的优先级 */
							(TaskHandle_t*	)&HighPriority_Task_Handle);/* 任务控制块指针 */ 
	if(pdPASS == xReturn)
		printf("创建HighPriority_Task任务成功!\n\n");
	  

}
static void xTaskNotifyDemo(void){

	/*任务通知代替消息队列、事件组*/ 
	// xTaskNotify()
	//xTaskNotifyWait()
	
	/*任务通知代替二值信号量、计数信号量*/ 
	// xTaskNotifyGive()
	// ulTaskNotifyTake()
	
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	
	/* 创建Receive1_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )Receive1_Task, /* 任务入口函数 */
						  (const char*	  )"Receive1_Task",/* 任务名字 */
						  (uint16_t 	  )512,   /* 任务栈大小 */
						  (void*		  )NULL,  /* 任务入口函数参数 */
						  (UBaseType_t	  )5,	  /* 任务的优先级 */
						  (TaskHandle_t*  )&Receive1_Task_Handle);/* 任务控制块指针 */
	if(pdPASS == xReturn)
	  printf("创建Receive1_Task任务成功!\r\n");
	
	/* 创建Receive2_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )Receive2_Task, /* 任务入口函数 */
						  (const char*	  )"Receive2_Task",/* 任务名字 */
						  (uint16_t 	  )512,   /* 任务栈大小 */
						  (void*		  )NULL,  /* 任务入口函数参数 */
						  (UBaseType_t	  )6,	  /* 任务的优先级 */
						  (TaskHandle_t*  )&Receive2_Task_Handle);/* 任务控制块指针 */
	if(pdPASS == xReturn)
	  printf("创建Receive2_Task任务成功!\r\n");
	
	/* 创建Send_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t )Send_Task_Notify,  /* 任务入口函数 */
						  (const char*	  )"Send_Task",/* 任务名字 */
						  (uint16_t 	  )512,  /* 任务栈大小 */
						  (void*		  )NULL,/* 任务入口函数参数 */
						  (UBaseType_t	  )7, /* 任务的优先级 */
						  (TaskHandle_t*  )&Send_Task_Notify_Handle);/* 任务控制块指针 */ 
	if(pdPASS == xReturn)
	  printf("创建Send_Task任务成功!\n\n");
	

}


/**********************************************************************
  * @ 函数名  ： LowPriority_Task
  * @ 功能说明： LowPriority_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void LowPriority_Task(void* parameter)
{	
  static uint32_t i;
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  while (1)
  {
    printf("LowPriority_Task 获取互斥量\n");
    //获取互斥量 MuxSem,没获取到则一直等待
		xReturn = xSemaphoreTake(MuxSem_Handle,/* 互斥量句柄 */
                              portMAX_DELAY); /* 等待时间 */
    if(pdTRUE == xReturn)
    printf("LowPriority_Task Runing\n\n");
    
    for(i=0;i<2000000;i++)//模拟低优先级任务占用互斥量
		{
			taskYIELD();//发起任务调度
		}
    
    printf("LowPriority_Task 释放互斥量!\r\n");
    xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量
      
		LED1_TOGGLE;
    
    vTaskDelay(1000);
  }
}

/**********************************************************************
  * @ 函数名  ： MidPriority_Task
  * @ 功能说明： MidPriority_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void MidPriority_Task(void* parameter)
{	 
  while (1)
  {
   printf("MidPriority_Task Runing\n");
   vTaskDelay(1000);
  }
}

/**********************************************************************
  * @ 函数名  ： HighPriority_Task
  * @ 功能说明： HighPriority_Task 任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void HighPriority_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
  while (1)
  {
    printf("HighPriority_Task 获取互斥量\n");
    //获取互斥量 MuxSem,没获取到则一直等待
		xReturn = xSemaphoreTake(MuxSem_Handle,/* 互斥量句柄 */
                              portMAX_DELAY); /* 等待时间 */
    if(pdTRUE == xReturn)
      printf("HighPriority_Task Runing\n");
		LED1_TOGGLE;
    
    printf("HighPriority_Task 释放互斥量!\r\n");
    xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量

  
    vTaskDelay(1000);
  }
}


/**********************************************************************
  * @ 函数名  ： Receive_Task
  * @ 功能说明： Receive_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Receive1_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
#if USE_CHAR
  char *r_char;
#else
  uint32_t r_num;
#endif
  while (1)
  {
    /* BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, 
                                  uint32_t ulBitsToClearOnExit, 
                                  uint32_t *pulNotificationValue, 
                                  TickType_t xTicksToWait ); 
     * ulBitsToClearOnEntry：当没有接收到任务通知的时候将任务通知值与此参数的取
       反值进行按位与运算，当此参数为Oxfffff或者ULONG_MAX的时候就会将任务通知值清零。
     * ulBits ToClearOnExit：如果接收到了任务通知，在做完相应的处理退出函数之前将
       任务通知值与此参数的取反值进行按位与运算，当此参数为0xfffff或者ULONG MAX的时候
       就会将任务通知值清零。
     * pulNotification Value：此参数用来保存任务通知值。
     * xTick ToWait：阻塞时间。
     *
     * 返回值：pdTRUE：获取到了任务通知。pdFALSE：任务通知获取失败。
     */
    //获取任务通知 ,没获取到则一直等待
		xReturn=xTaskNotifyWait(0x0,			//进入函数的时候不清除任务bit
                            ULONG_MAX,	  //退出函数的时候清除所有的bit
#if USE_CHAR
                            (uint32_t *)&r_char,		  //保存任务通知值
#else
                            &r_num,		  //保存任务通知值
#endif                        
                            portMAX_DELAY);	//阻塞时间
    if( pdTRUE == xReturn )
#if USE_CHAR
      printf("Receive1_Task 任务通知消息为 %s \n",r_char);                      
#else
      printf("Receive1_Task 任务通知消息为 %d \n",r_num);                      
#endif  
     
    
		LED1_TOGGLE;
  }
}

/**********************************************************************
  * @ 函数名  ： Receive_Task
  * @ 功能说明： Receive_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Receive2_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
#if USE_CHAR
  char *r_char;
#else
  uint32_t r_num;
#endif
  while (1)
  {
    /* BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, 
                                  uint32_t ulBitsToClearOnExit, 
                                  uint32_t *pulNotificationValue, 
                                  TickType_t xTicksToWait ); 
     * ulBitsToClearOnEntry：当没有接收到任务通知的时候将任务通知值与此参数的取
       反值进行按位与运算，当此参数为Oxfffff或者ULONG_MAX的时候就会将任务通知值清零。
     * ulBits ToClearOnExit：如果接收到了任务通知，在做完相应的处理退出函数之前将
       任务通知值与此参数的取反值进行按位与运算，当此参数为0xfffff或者ULONG MAX的时候
       就会将任务通知值清零。
     * pulNotification Value：此参数用来保存任务通知值。
     * xTick ToWait：阻塞时间。
     *
     * 返回值：pdTRUE：获取到了任务通知。pdFALSE：任务通知获取失败。
     */
    //获取任务通知 ,没获取到则一直等待
		xReturn=xTaskNotifyWait(0x0,			//进入函数的时候不清除任务bit
                            ULONG_MAX,	  //退出函数的时候清除所有的bit
#if USE_CHAR
                            (uint32_t *)&r_char,		  //保存任务通知值
#else
                            &r_num,		  //保存任务通知值
#endif                        
                            portMAX_DELAY);	//阻塞时间
    if( pdTRUE == xReturn )
#if USE_CHAR
      printf("Receive2_Task 任务通知消息为 %s \n",r_char);                      
#else
      printf("Receive2_Task 任务通知消息为 %d \n",r_num);                      
#endif  
		LED2_TOGGLE;
  }
}

/**********************************************************************
  * @ 函数名  ： Send_Task
  * @ 功能说明： Send_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Send_Task_Notify(void* parameter)
{	 
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
#if USE_CHAR
  char test_str1[] = "this is a mail test 1";/* 邮箱消息test1 */
  char test_str2[] = "this is a mail test 2";/* 邮箱消息test2 */
#else
  uint32_t send1 = 1;
  uint32_t send2 = 2;
#endif
  

  
  while (1)
  {
    /* KEY1 被按下 */
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON )
    {
      /* 原型:BaseType_t xTaskNotify( TaskHandle_t xTaskToNotify, 
                                      uint32_t ulValue, 
                                      eNotifyAction eAction ); 
       * eNoAction = 0，通知任务而不更新其通知值。
       * eSetBits，     设置任务通知值中的位。
       * eIncrement，   增加任务的通知值。
       * eSetvaluewithoverwrite，覆盖当前通知
       * eSetValueWithoutoverwrite 不覆盖当前通知
       * 
       * pdFAIL：当参数eAction设置为eSetValueWithoutOverwrite的时候，
       * 如果任务通知值没有更新成功就返回pdFAIL。
       * pdPASS: eAction 设置为其他选项的时候统一返回pdPASS。
      */
      xReturn = xTaskNotify( Receive1_Task_Handle, /*任务句柄*/
#if USE_CHAR 
                             (uint32_t)&test_str1, /* 发送的数据，最大为4字节 */
#else
                              send1, /* 发送的数据，最大为4字节 */
#endif
                             eSetValueWithOverwrite );/*覆盖当前通知*/
      
      if( xReturn == pdPASS )
        printf("Receive1_Task_Handle 任务通知消息发送成功!\r\n");
    } 
    /* KEY2 被按下 */
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON )
    {
      xReturn = xTaskNotify( Receive2_Task_Handle, /*任务句柄*/
#if USE_CHAR 
                             (uint32_t)&test_str2, /* 发送的数据，最大为4字节 */
#else
                              send2, /* 发送的数据，最大为4字节 */
#endif
                             eSetValueWithOverwrite );/*覆盖当前通知*/
      /* 此函数只会返回pdPASS */
      if( xReturn == pdPASS )
        printf("Receive2_Task_Handle 任务通知消息发送成功!\r\n");
    }
    vTaskDelay(20);
  }
}



/***********************************************************************
  * @ 函数名  ： Swtmr1_Callback
  * @ 功能说明： 软件定时器1 回调函数，打印回调函数信息&当前系统时间
  *              软件定时器请不要调用阻塞函数，也不要进行死循环，应快进快出
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void Swtmr1_Callback(void* parameter)
{		
  TickType_t tick_num1;

  TmrCb_Count1++;						/* 每回调一次加一 */

  tick_num1 = xTaskGetTickCount();	/* 获取滴答定时器的计数值 */
  
  LED1_TOGGLE;
  
  printf("Swtmr1_Callback函数执行 %d 次\n", TmrCb_Count1);
  printf("滴答定时器数值=%d\n", tick_num1);
}

/***********************************************************************
  * @ 函数名  ： Swtmr2_Callback
  * @ 功能说明： 软件定时器2 回调函数，打印回调函数信息&当前系统时间
  *              软件定时器请不要调用阻塞函数，也不要进行死循环，应快进快出
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void Swtmr2_Callback(void* parameter)
{	
  TickType_t tick_num2;

  TmrCb_Count2++;						/* 每回调一次加一 */

  tick_num2 = xTaskGetTickCount();	/* 获取滴答定时器的计数值 */

  printf("Swtmr2_Callback函数执行 %d 次\n", TmrCb_Count2);
  printf("滴答定时器数值=%d\n", tick_num2);
}

static void xTimerDemo(void){
	/************************************************************************************
	 * 创建软件周期定时器
	 * 函数原型
	 * TimerHandle_t xTimerCreate(	  const char * const pcTimerName,
								  const TickType_t xTimerPeriodInTicks,
								  const UBaseType_t uxAutoReload,
								  void * const pvTimerID,
				  TimerCallbackFunction_t pxCallbackFunction )
	  * @uxAutoReload : pdTRUE为周期模式，pdFALS为单次模式
	 * 单次定时器，周期(1000个时钟节拍)，周期模式
	*************************************************************************************/

	if(Swtmr1_Handle != NULL)						   
	{
	  /***********************************************************************************
	   * xTicksToWait:如果在调用xTimerStart()时队列已满，则以tick为单位指定调用任务应保持
	   * 在Blocked(阻塞)状态以等待start命令成功发送到timer命令队列的时间。 
	   * 如果在启动调度程序之前调用xTimerStart()，则忽略xTicksToWait。在这里设置等待时间为0.
	   **********************************************************************************/
	  xTimerStart(Swtmr1_Handle,0);   //开启周期定时器
	}							 
	/************************************************************************************
	 * 创建软件周期定时器
	 * 函数原型
	 * TimerHandle_t xTimerCreate(	  const char * const pcTimerName,
								  const TickType_t xTimerPeriodInTicks,
								  const UBaseType_t uxAutoReload,
								  void * const pvTimerID,
				  TimerCallbackFunction_t pxCallbackFunction )
	  * @uxAutoReload : pdTRUE为周期模式，pdFALS为单次模式
	 * 单次定时器，周期(5000个时钟节拍)，单次模式
	 *************************************************************************************/
	  Swtmr2_Handle=xTimerCreate((const char*		  )"OneShotTimer",
							   (TickType_t			  )5000,/* 定时器周期 5000(tick) */
							   (UBaseType_t 		  )pdFALSE,/* 单次模式 */
							   (void*					)2,/* 为每个计时器分配一个索引的唯一ID */
							   (TimerCallbackFunction_t)Swtmr2_Callback); 
	if(Swtmr2_Handle != NULL)
	{
	 /***********************************************************************************
	 * xTicksToWait:如果在调用xTimerStart()时队列已满，则以tick为单位指定调用任务应保持
	 * 在Blocked(阻塞)状态以等待start命令成功发送到timer命令队列的时间。 
	 * 如果在启动调度程序之前调用xTimerStart()，则忽略xTicksToWait。在这里设置等待时间为0.
	 **********************************************************************************/   
	  xTimerStart(Swtmr2_Handle,0);   //开启周期定时器
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

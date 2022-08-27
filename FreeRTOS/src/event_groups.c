/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Standard includes. */
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"

/* Lint e961 and e750 are suppressed as a MISRA exception justified because the
MPU ports require MPU_WRAPPERS_INCLUDED_FROM_API_FILE to be defined for the
header files above, but not in this file, in order to generate the correct
privileged Vs unprivileged linkage and placement. */
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE /*lint !e961 !e750. */

/* The following bit fields convey control information in a task's event list
item value.  It is important they don't clash with the
taskEVENT_LIST_ITEM_VALUE_IN_USE definition. */
#if configUSE_16_BIT_TICKS == 1
	#define eventCLEAR_EVENTS_ON_EXIT_BIT	0x0100U
	#define eventUNBLOCKED_DUE_TO_BIT_SET	0x0200U
	#define eventWAIT_FOR_ALL_BITS			0x0400U
	#define eventEVENT_BITS_CONTROL_BYTES	0xff00U
#else
	#define eventCLEAR_EVENTS_ON_EXIT_BIT	0x01000000UL	// 时间退出时 是否需要清除标志位
	#define eventUNBLOCKED_DUE_TO_BIT_SET	0x02000000UL	// 标记该任务事件位设置了，已解除阻塞
	#define eventWAIT_FOR_ALL_BITS			0x04000000UL	// 表示设置的所有事件位都为1才有效 
	#define eventEVENT_BITS_CONTROL_BYTES	0xff000000UL
#endif

typedef struct xEventGroupDefinition
{
	EventBits_t uxEventBits;	/* 储存事件标志组中的事件位 */
	List_t xTasksWaitingForBits;		/*< List of tasks waiting for a bit to be set. */

	#if( configUSE_TRACE_FACILITY == 1 )	/* 启用可视化跟踪调试 */
		UBaseType_t uxEventGroupNumber;
	#endif

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucStaticallyAllocated; /*< Set to pdTRUE if the event group is statically allocated to ensure no attempt is made to free the memory. */
	#endif
} EventGroup_t;

/*-----------------------------------------------------------*/

/*
 * Test the bits set in uxCurrentEventBits to see if the wait condition is met.
 * The wait condition is defined by xWaitForAllBits.  If xWaitForAllBits is
 * pdTRUE then the wait condition is met if all the bits set in uxBitsToWaitFor
 * are also set in uxCurrentEventBits.  If xWaitForAllBits is pdFALSE then the
 * wait condition is met if any of the bits set in uxBitsToWait for are also set
 * in uxCurrentEventBits.
 */
static BaseType_t prvTestWaitCondition( const EventBits_t uxCurrentEventBits, const EventBits_t uxBitsToWaitFor, const BaseType_t xWaitForAllBits ) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

#if( configSUPPORT_STATIC_ALLOCATION == 1 )

	EventGroupHandle_t xEventGroupCreateStatic( StaticEventGroup_t *pxEventGroupBuffer )
	{
	EventGroup_t *pxEventBits;

		/* A StaticEventGroup_t object must be provided. */
		configASSERT( pxEventGroupBuffer );

		/* The user has provided a statically allocated event group - use it. */
		pxEventBits = ( EventGroup_t * ) pxEventGroupBuffer; /*lint !e740 EventGroup_t and StaticEventGroup_t are guaranteed to have the same size and alignment requirement - checked by configASSERT(). */

		if( pxEventBits != NULL )
		{
			pxEventBits->uxEventBits = 0;
			vListInitialise( &( pxEventBits->xTasksWaitingForBits ) );

			#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
			{
				/* Both static and dynamic allocation can be used, so note that
				this event group was created statically in case the event group
				is later deleted. */
				pxEventBits->ucStaticallyAllocated = pdTRUE;
			}
			#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

			traceEVENT_GROUP_CREATE( pxEventBits );
		}
		else
		{
			traceEVENT_GROUP_CREATE_FAILED();
		}

		return ( EventGroupHandle_t ) pxEventBits;
	}

#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )

	EventGroupHandle_t xEventGroupCreate( void )
	{
	EventGroup_t *pxEventBits;

		/* Allocate the event group. */
		pxEventBits = ( EventGroup_t * ) pvPortMalloc( sizeof( EventGroup_t ) );

		if( pxEventBits != NULL )
		{
			pxEventBits->uxEventBits = 0;
			vListInitialise( &( pxEventBits->xTasksWaitingForBits ) );

			#if( configSUPPORT_STATIC_ALLOCATION == 1 )
			{
				/* Both static and dynamic allocation can be used, so note this
				event group was allocated statically in case the event group is
				later deleted. */
				pxEventBits->ucStaticallyAllocated = pdFALSE;
			}
			#endif /* configSUPPORT_STATIC_ALLOCATION */

			traceEVENT_GROUP_CREATE( pxEventBits );
		}
		else
		{
			traceEVENT_GROUP_CREATE_FAILED();
		}

		return ( EventGroupHandle_t ) pxEventBits;
	}

#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*-----------------------------------------------------------*/

//而对于函数xEventGroupSync()即使任务B和任务C事件位被删除，但记录了原有的事件位uxOriginalBitValue，通过变量uxOriginalBitValue判断达到任务A同步。
EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, const EventBits_t uxBitsToWaitFor, TickType_t xTicksToWait )
{
EventBits_t uxOriginalBitValue, uxReturn;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
BaseType_t xAlreadyYielded;
BaseType_t xTimeoutOccurred = pdFALSE;

	configASSERT( ( uxBitsToWaitFor & eventEVENT_BITS_CONTROL_BYTES ) == 0 );
	configASSERT( uxBitsToWaitFor != 0 );
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif

	vTaskSuspendAll();
	{
		uxOriginalBitValue = pxEventBits->uxEventBits;

		( void ) xEventGroupSetBits( xEventGroup, uxBitsToSet );

		// 所有事件位 都设置了 全部满足了
		if( ( ( uxOriginalBitValue | uxBitsToSet ) & uxBitsToWaitFor ) == uxBitsToWaitFor )
		{
			/* All the rendezvous bits are now set - no need to block. */
			uxReturn = ( uxOriginalBitValue | uxBitsToSet );

			/* Rendezvous always clear the bits.  They will have been cleared
			already unless this is the only task in the rendezvous. */
			pxEventBits->uxEventBits &= ~uxBitsToWaitFor;	// 清除事件位

			xTicksToWait = 0;
		}
		else
		{
			if( xTicksToWait != ( TickType_t ) 0 )
			{
				traceEVENT_GROUP_SYNC_BLOCK( xEventGroup, uxBitsToSet, uxBitsToWaitFor );

				/* Store the bits that the calling task is waiting for in the
				task's event list item so the kernel knows when a match is
				found.  Then enter the blocked state. */
				vTaskPlaceOnUnorderedEventList( &( pxEventBits->xTasksWaitingForBits ), ( uxBitsToWaitFor | eventCLEAR_EVENTS_ON_EXIT_BIT | eventWAIT_FOR_ALL_BITS ), xTicksToWait );

				/* This assignment is obsolete as uxReturn will get set after
				the task unblocks, but some compilers mistakenly generate a
				warning about uxReturn being returned without being set if the
				assignment is omitted. */
				uxReturn = 0;
			}
			else
			{
				/* The rendezvous bits were not set, but no block time was
				specified - just return the current event bit value. */
				uxReturn = pxEventBits->uxEventBits;
			}
		}
	}
	xAlreadyYielded = xTaskResumeAll();

	if( xTicksToWait != ( TickType_t ) 0 )
	{
		if( xAlreadyYielded == pdFALSE )
		{
			portYIELD_WITHIN_API();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* The task blocked to wait for its required bits to be set - at this
		point either the required bits were set or the block time expired.  If
		the required bits were set they will have been stored in the task's
		event list item, and they should now be retrieved then cleared. */
		uxReturn = uxTaskResetEventItemValue();

		if( ( uxReturn & eventUNBLOCKED_DUE_TO_BIT_SET ) == ( EventBits_t ) 0 )
		{
			/* The task timed out, just return the current event bit value. */
			taskENTER_CRITICAL();
			{
				uxReturn = pxEventBits->uxEventBits;

				/* Although the task got here because it timed out before the
				bits it was waiting for were set, it is possible that since it
				unblocked another task has set the bits.  If this is the case
				then it needs to clear the bits before exiting. */
				if( ( uxReturn & uxBitsToWaitFor ) == uxBitsToWaitFor )
				{
					pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			taskEXIT_CRITICAL();

			xTimeoutOccurred = pdTRUE;
		}
		else
		{
			/* The task unblocked because the bits were set. */
		}

		/* Control bits might be set as the task had blocked should not be
		returned. */
		uxReturn &= ~eventEVENT_BITS_CONTROL_BYTES;
	}

	traceEVENT_GROUP_SYNC_END( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred );

	return uxReturn;
}
/*-----------------------------------------------------------*/
/*
	事件标志组是实现多任务同步的有效机制之一。也许有不理解的初学者会问采用事件标志组多麻烦，搞个全局变量不是更简单？其实不然，在裸机编程时，使用全局变量的确比较方便，但是在加上 RTOS 后就是另一种情况了。使用全局变量相比事件标志组主要有如下三个问题：　　

 使用事件标志组可以让 RTOS 内核有效地管理任务，而全局变量是无法做到的，任务的超时等机制需要用户自己去实现。
 使用了全局变量就要防止多任务的访问冲突，而使用事件标志组则处理好了这个问题，用户无需担心。
 使用事件标志组可以有效地解决中断服务程序和任务之间的同步问题。

 xEventGroupCreate()
 xEventGroupCreateStatic()
 vEventGroupDelete()
 xEventGroupWaitBits()
 xEventGroupSetBits()
 xEventGroupSetBitsFromISR()
 xEventGroupClearBits()
 xEventGroupClearBitsFromISR()
 xEventGroupGetBits()
 xEventGroupGetBitsFromISR()

 xEventGroupSync()

*/

// 返回value 值
EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
										const EventBits_t uxBitsToWaitFor,	// 等待被设置的事件标志位 
										const BaseType_t xClearOnExit,		// 退出时 是否清除事件位 T\F
										const BaseType_t xWaitForAllBits,	// 选择是否等待所有标志位都被设置才响应
										TickType_t xTicksToWait )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn, uxControlBits = 0;
BaseType_t xWaitConditionMet, xAlreadyYielded;
BaseType_t xTimeoutOccurred = pdFALSE;

	/* Check the user is not attempting to wait on the bits used by the kernel
	itself, and that at least one bit is being requested. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToWaitFor & eventEVENT_BITS_CONTROL_BYTES ) == 0 );
	configASSERT( uxBitsToWaitFor != 0 );
	#if ( ( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ) )
	{
		configASSERT( !( ( xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED ) && ( xTicksToWait != 0 ) ) );
	}
	#endif

	vTaskSuspendAll();
	{
		const EventBits_t uxCurrentEventBits = pxEventBits->uxEventBits;

		/* Check to see if the wait condition is already met or not. */
		xWaitConditionMet = prvTestWaitCondition( uxCurrentEventBits, uxBitsToWaitFor, xWaitForAllBits );

		if( xWaitConditionMet != pdFALSE )	// 满足了 需要开始
		{
			/* The wait condition has already been met so there is no need to
			block. */
			uxReturn = uxCurrentEventBits;	
			xTicksToWait = ( TickType_t ) 0;

			/* Clear the wait bits if requested to do so. */
			if( xClearOnExit != pdFALSE )	// 退出时 需要清除标志位
			{
				pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else if( xTicksToWait == ( TickType_t ) 0 )	// 不满足 且不等待
		{
			/* The wait condition has not been met, but no block time was
			specified, so just return the current value. */
			uxReturn = uxCurrentEventBits;
		}
		else	// 事件阻塞 需要等待
		{
			/* The task is going to block to wait for its required bits to be
			set.  uxControlBits are used to remember the specified behaviour of
			this call to xEventGroupWaitBits() - for use when the event bits
			unblock the task. */
			if( xClearOnExit != pdFALSE )	// 响应之后需要进行清除标志wei
			{
				uxControlBits |= eventCLEAR_EVENTS_ON_EXIT_BIT;	// 更新控制位
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( xWaitForAllBits != pdFALSE )	// 需要等待全部的事件完成 才能响应
			{
				uxControlBits |= eventWAIT_FOR_ALL_BITS;	// 更新控制位
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			/* Store the bits that the calling task is waiting for in the
			task's event list item so the kernel knows when a match is
			found.  Then enter the blocked state. */
			// 添加到延时队列 和 事件队列中等待
			vTaskPlaceOnUnorderedEventList( &( pxEventBits->xTasksWaitingForBits ), ( uxBitsToWaitFor | uxControlBits ), xTicksToWait );

			/* This is obsolete as it will get set after the task unblocks, but
			some compilers mistakenly generate a warning about the variable
			being returned without being set if it is not done. */
			uxReturn = 0;

			traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor );
		}
	}
	xAlreadyYielded = xTaskResumeAll();

	// 当前阻塞时间不为0
	if( xTicksToWait != ( TickType_t ) 0 )
	{
		if( xAlreadyYielded == pdFALSE )	// 恢复调度器时没有进行任务切换
		{
			portYIELD_WITHIN_API();	// 进行任务切换
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* The task blocked to wait for its required bits to be set - at this
		point either the required bits were set or the block time expired.  If
		the required bits were set they will have been stored in the task's
		event list item, and they should now be retrieved then cleared. */
		uxReturn = uxTaskResetEventItemValue();
		// 该任务还没有被执行 没有解挂
		if( ( uxReturn & eventUNBLOCKED_DUE_TO_BIT_SET ) == ( EventBits_t ) 0 )	
		{
			taskENTER_CRITICAL();
			{
				/* The task timed out, just return the current event bit value. */
				uxReturn = pxEventBits->uxEventBits;

				/* It is possible that the event bits were updated between this
				task leaving the Blocked state and running again. */
				// 需要进行任务切换
				if( prvTestWaitCondition( uxReturn, uxBitsToWaitFor, xWaitForAllBits ) != pdFALSE )
				{
					if( xClearOnExit != pdFALSE ) // 
					{
						pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			taskEXIT_CRITICAL();

			/* Prevent compiler warnings when trace macros are not used. */
			xTimeoutOccurred = pdFALSE;
		}
		else
		{
			/* The task unblocked because the bits were set. */
		}

		/* The task blocked so control bits may have been set. */
		uxReturn &= ~eventEVENT_BITS_CONTROL_BYTES; 	// 清除控制位
	}
	traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred );

	return uxReturn;
}
/*-----------------------------------------------------------*/
// 清除事件组的某一位 并返回事件组
EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn;

	/* Check the user is not attempting to clear the bits used by the kernel
	itself. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToClear & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

	taskENTER_CRITICAL();
	{
		traceEVENT_GROUP_CLEAR_BITS( xEventGroup, uxBitsToClear );

		/* The value returned is the event group value prior to the bits being
		cleared. */
		uxReturn = pxEventBits->uxEventBits;

		/* Clear the bits. */
		pxEventBits->uxEventBits &= ~uxBitsToClear;
	}
	taskEXIT_CRITICAL();

	return uxReturn;
}
/*-----------------------------------------------------------*/

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( INCLUDE_xTimerPendFunctionCall == 1 ) && ( configUSE_TIMERS == 1 ) )

	BaseType_t xEventGroupClearBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
	{
		BaseType_t xReturn;

		traceEVENT_GROUP_CLEAR_BITS_FROM_ISR( xEventGroup, uxBitsToClear );
		xReturn = xTimerPendFunctionCallFromISR( vEventGroupClearBitsCallback, ( void * ) xEventGroup, ( uint32_t ) uxBitsToClear, NULL );

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

EventBits_t xEventGroupGetBitsFromISR( EventGroupHandle_t xEventGroup )
{
UBaseType_t uxSavedInterruptStatus;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
EventBits_t uxReturn;

	uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		uxReturn = pxEventBits->uxEventBits;
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

	return uxReturn;
}
/*-----------------------------------------------------------*/
/*
1. 使用前一定要保证事件标志组已经通过函数 xEventGroupCreate 创建了。
2. 此函数是用于任务代码中调用的，故不可以在中断服务程序中调用此函数，中断服务程序中使用的是xEventGroupSetBitsFromISR
3. 用户通过参数 uxBitsToSet 设置的标志位并不一定会保留到此函数的返回值中，下面举两种情况：
　　a. 调用此函数的过程中，其它高优先级的任务就绪了，并且也修改了事件标志，此函数返回的事件标志位会发生变化。
　　b. 调用此函数的任务是一个低优先级任务，通过此函数设置了事件标志后，让一个等待此事件标志的高优先级任务就绪了，会立即切换到高优先级任务去执行，相应的事件标志位会被函数xEventGroupWaitBits 清除掉，等从高优先级任务返回到低优先级任务后，函数xEventGroupSetBits 的返回值已经被修改。

*/
EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup,
										const EventBits_t uxBitsToSet )	// 设置后24BIT用的 设置1
{
ListItem_t *pxListItem, *pxNext;
ListItem_t const *pxListEnd;
List_t *pxList;
EventBits_t uxBitsToClear = 0, uxBitsWaitedFor, uxControlBits;
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
BaseType_t xMatchFound = pdFALSE;

	/* Check the user is not attempting to set the bits used by the kernel
	itself. */
	configASSERT( xEventGroup );
	configASSERT( ( uxBitsToSet & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

	pxList = &( pxEventBits->xTasksWaitingForBits );	// 获取事件标志组对应的事件列表
	pxListEnd = listGET_END_MARKER( pxList ); 			// /* 获取列表中最后一个列表项，即Mini列表项 */
	/*lint !e826 !e740 The mini list structure is used as the list end to save RAM.  This is checked and valid. */
	vTaskSuspendAll();
	{
		traceEVENT_GROUP_SET_BITS( xEventGroup, uxBitsToSet );

		pxListItem = listGET_HEAD_ENTRY( pxList );	/* 得到第一个列表项，即第一个因事件而阻塞的任务 */

		/* Set the bits. */
		pxEventBits->uxEventBits |= uxBitsToSet;

		/* See if the new bit value should unblock any tasks. */
		/* 如果事件列表中有列表项，即有阻塞任务，则进入循环中 */
		while( pxListItem != pxListEnd )
		{
			pxNext = listGET_NEXT( pxListItem );
			uxBitsWaitedFor = listGET_LIST_ITEM_VALUE( pxListItem );	///* 获取对应任务的项值 */
			xMatchFound = pdFALSE;

			/* Split the bits waited for from the control bits. */
			uxControlBits = uxBitsWaitedFor & eventEVENT_BITS_CONTROL_BYTES;	// 高8位 控制位
			uxBitsWaitedFor &= ~eventEVENT_BITS_CONTROL_BYTES;					// 低24位 事件标志位
			/* 如果不要等待所有的事件位都为1才有效 */
			if( ( uxControlBits & eventWAIT_FOR_ALL_BITS ) == ( EventBits_t ) 0 )	// 0000 0100 
			{
				/* Just looking for single bit being set. */
				if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) != ( EventBits_t ) 0 )
				{	// 事件标志位 后24位 & 之后 位 不等于0 就是有毛病  就是至少有一个事件达到了
					xMatchFound = pdTRUE;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) == uxBitsWaitedFor )
			{	// 事件标志位 都被设置了 uxBitsWaitedFor 
				/* All bits are set. */
				xMatchFound = pdTRUE;
			}
			else
			{	// 没有 事件标志需要设置
				/* Need all bits to be set, but not all the bits were set. */
			}

			if( xMatchFound != pdFALSE )
			{
				/* The bits match.  Should the bits be cleared on exit? */
				if( ( uxControlBits & eventCLEAR_EVENTS_ON_EXIT_BIT ) != ( EventBits_t ) 0 )	// 0000 0001
				{
					// 控制位的最后一位 为1 后 退出后删除标志位
					// 将这个位 和 标志位进行拼接 更新uxBitsToClear 获得需要清除的标志位
					uxBitsToClear |= uxBitsWaitedFor;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}

				/* Store the actual event flag value in the task's event list
				item before removing the task from the event list.  The
				eventUNBLOCKED_DUE_TO_BIT_SET bit is set so the task knows
				that is was unblocked due to its required bits matching, rather
				than because it timed out. */
				/*
				在从事件列表中删除任务之前，将实际事件标志值存储在任务的事件列表项中。
				设置了 eventUNBLOCKED_DUE_TO_BIT_SET 位，因此任务知道由于其所需的位匹配而不是因为它超时而被解除阻塞。
				*/
				( void ) xTaskRemoveFromUnorderedEventList( pxListItem, pxEventBits->uxEventBits | eventUNBLOCKED_DUE_TO_BIT_SET );
			}

			/* Move onto the next list item.  Note pxListItem->pxNext is not
			used here as the list item may have been removed from the event list
			and inserted into the ready/pending reading list. */
			pxListItem = pxNext;
		}

		/* Clear any bits that matched when the eventCLEAR_EVENTS_ON_EXIT_BIT
		bit was set in the control word. */
		pxEventBits->uxEventBits &= ~uxBitsToClear;	//删除事件位
	}
	( void ) xTaskResumeAll();

	return pxEventBits->uxEventBits;	// 返回事件组标志
}
/*-----------------------------------------------------------*/

void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
const List_t *pxTasksWaitingForBits = &( pxEventBits->xTasksWaitingForBits );

	vTaskSuspendAll();
	{
		traceEVENT_GROUP_DELETE( xEventGroup );

		while( listCURRENT_LIST_LENGTH( pxTasksWaitingForBits ) > ( UBaseType_t ) 0 )	/* 当事件列表中存在列表项，即存在阻塞的任务 */
		{
			/* Unblock the task, returning 0 as the event list is being deleted
			and	cannot therefore have any bits set. */
			configASSERT( pxTasksWaitingForBits->xListEnd.pxNext != ( ListItem_t * ) &( pxTasksWaitingForBits->xListEnd ) );
			/* 将任务从事件列表中移除，添加到就绪列表中 */
			( void ) xTaskRemoveFromUnorderedEventList( pxTasksWaitingForBits->xListEnd.pxNext, eventUNBLOCKED_DUE_TO_BIT_SET );
		}

		#if( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 0 ) )
		{
			/* The event group can only have been allocated dynamically - free
			it again. */
			vPortFree( pxEventBits );
		}
		#elif( ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
		{
			/* The event group could have been allocated statically or
			dynamically, so check before attempting to free the memory. */
			if( pxEventBits->ucStaticallyAllocated == ( uint8_t ) pdFALSE )
			{
				vPortFree( pxEventBits );
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
	}
	( void ) xTaskResumeAll();
}
/*-----------------------------------------------------------*/

/* For internal use only - execute a 'set bits' command that was pended from
an interrupt. */
void vEventGroupSetBitsCallback( void *pvEventGroup, const uint32_t ulBitsToSet )
{
	( void ) xEventGroupSetBits( pvEventGroup, ( EventBits_t ) ulBitsToSet );
}
/*-----------------------------------------------------------*/

/* For internal use only - execute a 'clear bits' command that was pended from
an interrupt. */
void vEventGroupClearBitsCallback( void *pvEventGroup, const uint32_t ulBitsToClear )
{
	( void ) xEventGroupClearBits( pvEventGroup, ( EventBits_t ) ulBitsToClear );
}
/*-----------------------------------------------------------*/
// 根据 事件列表的信息 和 是否需要全部与否 更具返回值 是否需要进行调度切换
static BaseType_t prvTestWaitCondition( const EventBits_t uxCurrentEventBits,
												const EventBits_t uxBitsToWaitFor,
												const BaseType_t xWaitForAllBits )
{
BaseType_t xWaitConditionMet = pdFALSE;

	if( xWaitForAllBits == pdFALSE )	// 不需要 全部标志位都响应
	{
		/* Task only has to wait for one bit within uxBitsToWaitFor to be
		set.  Is one already set? */
		// 至少有一个事件得到满足
		if( ( uxCurrentEventBits & uxBitsToWaitFor ) != ( EventBits_t ) 0 )
		{
			xWaitConditionMet = pdTRUE;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	else	// 需要全部标志满足才行
	{
		/* Task has to wait for all the bits in uxBitsToWaitFor to be set.
		Are they set already? */
		if( ( uxCurrentEventBits & uxBitsToWaitFor ) == uxBitsToWaitFor )
		{
			xWaitConditionMet = pdTRUE;
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}

	return xWaitConditionMet;// 通过这个返回值决定是否需要进行调度
}
/*-----------------------------------------------------------*/

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( INCLUDE_xTimerPendFunctionCall == 1 ) && ( configUSE_TIMERS == 1 ) )

	BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet, BaseType_t *pxHigherPriorityTaskWoken )
	{
	BaseType_t xReturn;

		traceEVENT_GROUP_SET_BITS_FROM_ISR( xEventGroup, uxBitsToSet );
		xReturn = xTimerPendFunctionCallFromISR( vEventGroupSetBitsCallback, ( void * ) xEventGroup, ( uint32_t ) uxBitsToSet, pxHigherPriorityTaskWoken );

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

	UBaseType_t uxEventGroupGetNumber( void* xEventGroup )
	{
	UBaseType_t xReturn;
	EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;

		if( xEventGroup == NULL )
		{
			xReturn = 0;
		}
		else
		{
			xReturn = pxEventBits->uxEventGroupNumber;
		}

		return xReturn;
	}

#endif


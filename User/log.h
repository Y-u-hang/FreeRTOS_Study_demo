#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

#define LOG_RANK 0	//LOG 等级设定
#define DEBUG 0
#define INFO 1
#define WARNING 2
#define ERROR 3

#if (LOG_RANK == DEBUG)
#define log_debug(fmt, ...) do{printf("[DEBUG][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_info(fmt, ...) do{printf("[INFO][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_warn(fmt, ...) do{printf("[WARNING][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_error(fmt, ...) do{printf("[ERROR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#elif (LOG_RANK == INFO)
#define log_debug(...)
#define log_info(fmt, ...) do{printf("[INFO][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_warn(fmt, ...) do{printf("[WARNING][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_error(fmt, ...) do{printf("[ERROR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#elif (LOG_RANK == WARNING)
#define log_debug(...)
#define log_info(...)
#define log_warn(fmt, ...) do{printf("[WARNING][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define log_error(fmt, ...) do{printf("[ERROR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#elif (LOG_RANK == ERROR)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(fmt, ...) do{printf("[ERROR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#endif

#define SDIO_DEBUG log_debug
#define SDIO_INFO log_info
#define SDIO_WARNING log_warn
#define SDIO_ERROR log_error

#define DISKIO_DEBUG log_debug
#define DISKIO_INFO log_info
#define DISKIO_WARNING log_warn
#define DISKIO_ERROR log_error

#define FF_DEBUG log_debug
#define FF_INFO log_info
#define FF_WARNING log_warn
#define FF_ERROR log_error

			

#ifdef __cplusplus
}
#endif

#endif

/*
 * rtu_log.h
 *
 *  Created on: Jan 7, 2015
 *      Author: ygz
 */

#ifndef COMMON_RTU_LOG_H_
#define COMMON_RTU_LOG_H_


extern int log_init();
extern void log_printf(const char* format, ...);
extern void info_printf(const char* format, ...);

#define log_error(format, arg...) log_printf("err %s %d: "format" %d %s\r\n", \
	__FUNCTION__, __LINE__, ##arg, errno, strerror(errno))

#define log_msg(format, arg...) log_printf("msg %s %d: "format"\r\n", __FUNCTION__, __LINE__, ##arg)

#define log_warning(format, arg...) //log_printf("war %s %d: "format"\r\n", __FUNCTION__, __LINE__, ##arg)

#define log_debug(format, arg...) log_printf("bug %s %d: "format"\r\n", __FUNCTION__, __LINE__, ##arg)

#if 0
#define printf(format, arg...) fprintf(stdout, "%s %d: "format, __FUNCTION__, __LINE__, ##arg)
#define perror(format, arg...) \
		fprintf(stderr, "%s %d: "format" ERROR %d %s\n", __FUNCTION__, __LINE__, ##arg, errno, strerror(errno))
#endif

#if 0
#define perror(format, arg...) log_error(format, ##arg)
#define printf(format, arg...) log_debug(format, ##arg)
#else
#define perror(format, arg...) log_error(format, ##arg);\
		fprintf(stderr, "%s %d: "format" ERROR %d %s\n", __FUNCTION__, __LINE__, ##arg, errno, strerror(errno))
#endif

#if 0
#define printf(format, arg...) log_debug(format, ##arg);\
		fprintf(stdout, "%s %d: "format, __FUNCTION__, __LINE__, ##arg)
#else
#define printf(format, arg...) fprintf(stdout, "%s %d: "format, __FUNCTION__, __LINE__, ##arg)
#endif


#endif /* COMMON_RTU_LOG_H_ */

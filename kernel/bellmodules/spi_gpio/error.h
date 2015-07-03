/*
 * Header for WSPI module
 *
 *
 */	
			                     
#ifndef _ERROR_WSPI_H_
#define _ERROR_WSPI_H_

#ifdef __KERNEL__

typedef enum {
	OK = 0,
    ERROR_WSPI_FIXED_BUSY,
	ERROR_WSPI_ASYNC_TIMEOUT,
	ERROR_OS_DMA_ALLOC,
	ERROR_OS_ALLOC_MEM,
	ERROR_SPI_GET_REQUEST,
	SPI_PENDING,
	
} returnVal;

#endif // __KERNEL__ 
#endif // _ERROR_WSPI_H_


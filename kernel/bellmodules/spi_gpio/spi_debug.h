#ifndef _SPI_DEBUG_H_
#define _SPI_DEBUG_H_


#define spi_err(format, arg...)   printk(KERN_ERR MODULE_NAME " ERROR: " format "\n",  ## arg )
#define spi_info(format, arg...)  printk(KERN_INFO MODULE_NAME ": " format "\n",  ## arg )
//#define spi_err(format, arg...) 	NULL
//#define spi_info(format, arg...)	NULL

//#define CONFIG_SPI_DEBUG

#undef DEBUG
#ifdef CONFIG_SPI_DEBUG	

enum DEBUG_LEVELS
{
	LEVEL0 = 0,
	LEVEL1
};

#define MAX_WAIT_LOOP_COUNT 1000
extern int spi_davinci_debug;
extern int spi_davinci_dbg_print;

#define dbg_printk(args...)\
	if (spi_davinci_dbg_print) printk(args);
#define DEBUG(n, fmt, args...)\
	if (n <=  spi_davinci_debug) printk(KERN_INFO "%s() " fmt,__FUNCTION__ , ## args);

#else

#define dbg_printk(args...)
#define DEBUG(n, args...)

#endif /* CONFIG_SPI_DEBUG */


#endif /* _SPI_DEBUG_H_    */

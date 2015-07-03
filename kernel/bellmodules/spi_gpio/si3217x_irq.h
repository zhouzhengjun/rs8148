#ifndef _TOUCH_IRQ_H
#define _TOUCH_IRQ_H

/* DAVINCI GPIO */
#define TNETW_GPIO_IRQ          6
#define DAVINCI_GPIO_IRQ        58
#define DEV_IRQ_NAME    "touch"
#define DEV_IRQ_ID      "touch"
#define DAVINCI_GPIO                            IO_ADDRESS(DAVINCI_GPIO_BASE)
#define DAVINCI_GPIO_BINTEN                     (unsigned int *)(DAVINCI_GPIO + 0x08)
#define DAVINCI_GPIO_DIR_REG        (unsigned int *)(DAVINCI_GPIO + 0x38)
#define DAVINCI_GPIO_OUT_DATA       (unsigned int *)(DAVINCI_GPIO + 0x3c)
#define DAVINCI_GPIO_SET_RIS_TRIG   (unsigned int *)(DAVINCI_GPIO + 0x4c)
#define DAVINCI_GPIO_CLR_RIS_TRIG   (unsigned int *)(DAVINCI_GPIO + 0x50)
#define DAVINCI_GPIO_SET_FAL_TRIG   (unsigned int *)(DAVINCI_GPIO + 0x54)
#define DAVINCI_GPIO_CLR_FAL_TRIG   (unsigned int *)(DAVINCI_GPIO + 0x58)
#define DAVINCI_GPIO_INSTAT         (unsigned int *)(DAVINCI_GPIO + 0x5c)
#define DAVINCI_GPIO_IN_DATA        (unsigned int *)(DAVINCI_GPIO + 0x48)
#define DRV_GPIO_DELAY  1                                                  /* Delay to make sure GPIOs are stable */
void GPIO_Hardware_Init( void );

void touch_interrupt (int irq, void *dev_id, struct pt_regs *regs);
                                                                                                   
int touch_irq_init(void);
void touch_irq_release(void);

#endif



#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMD_LED1		0x1
#define CMD_LED2	0x22
#define CMD_LED3		0x3

int main(int argc, char *argv[])
{
	int led_fd;
	int led_enable = 1;
	
	led_fd = open("/dev/davinci_led" , O_RDWR);
	
	// sleep 1 second
	usleep(3 * 1000 * 1000);
	led_enable = 1;
	//printf("enable led1\n");
	ioctl(led_fd, CMD_LED1, &led_enable);
	
	// sleep 1 second
	usleep(3 * 1000 * 1000);
	led_enable = 1;
	//printf("enable led2\n");
	ioctl(led_fd, CMD_LED2, &led_enable);
	
	// sleep 1 second
	usleep(3 * 1000 * 1000);
	led_enable = 1;
	//printf("enable led3\n");
	ioctl(led_fd, CMD_LED3, &led_enable);
	
	
	//diable led
	led_enable = 0;
	
	 //sleep 1 second
	usleep(3 * 1000 * 1000);
	//printf("disable led1\n");
	ioctl(led_fd, CMD_LED1, &led_enable);
	
	// sleep 1 second
	led_enable = 0;
	usleep(3 * 1000 * 1000);
	//printf("disable led2\n");
	ioctl(led_fd, CMD_LED2, &led_enable);
	
	// sleep 1 second
	led_enable = 0;
	usleep(3 * 1000 * 1000);
	//printf("disable led3\n");
	ioctl(led_fd, CMD_LED3, &led_enable);
	
	close(led_fd);
	return 0;
}

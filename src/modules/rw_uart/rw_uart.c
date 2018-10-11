#include <px4_config.h>
#include <px4_posix.h>
#include <px4_tasks.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <drivers/drv_hrt.h>
#include <systemlib/err.h>
#include <fcntl.h>

static bool thread_should_exit = false;
static bool thread_running = false;
static int daemon_task;


__EXPORT int rw_uart_main(int argc, char *argv[]);
int rw_uart_thread_main(int argc, char *argv[]);

static int uart_init(const char * uart_name);    //
static int set_uart_baudrate(const int fd, unsigned int baud); //static
static void usage(const char *reason);            //static
static int shoot_total;


int set_uart_baudrate(const int fd, unsigned int baud)
{
    int speed;

    switch (baud) {
        case 9600:   speed = B9600;   break;
        case 19200:  speed = B19200;  break;
        case 38400:  speed = B38400;  break;
        case 57600:  speed = B57600;  break;
        case 115200: speed = B115200; break;
        default:
            warnx("ERR: baudrate: %d\n", baud);
            return -EINVAL;
    }

    struct termios uart_config;

    int termios_state;


    tcgetattr(fd, &uart_config);

    uart_config.c_oflag &= ~ONLCR;

    uart_config.c_cflag &= ~(CSTOPB | PARENB);

    if ((termios_state = cfsetispeed(&uart_config, speed)) < 0) {
        warnx("ERR: %d (cfsetispeed)\n", termios_state);
        return false;
    }

    if ((termios_state = cfsetospeed(&uart_config, speed)) < 0) {
        warnx("ERR: %d (cfsetospeed)\n", termios_state);
        return false;
    }

    if ((termios_state = tcsetattr(fd, TCSANOW, &uart_config)) < 0) {
        warnx("ERR: %d (tcsetattr)\n", termios_state);
        return false;
    }

    return true;
}


int uart_init(const char * uart_name)
{
    int serial_fd = open(uart_name, O_RDWR | O_NOCTTY);

    if (serial_fd < 0) {
        err(1, "failed to open port: %s", uart_name);
        return false;
    }
    return serial_fd;
}

static void usage(const char *reason)
{
    if (reason) {
        fprintf(stderr, "%s\n", reason);
    }

    fprintf(stderr, "usage: position_estimator_inav {start|stop|status} [param]\n\n");
    exit(1);
}

int rw_uart_main(int argc, char *argv[])
{
    if (argc < 2) {
        usage("[YCM]missing command");
    }

    if (!strcmp(argv[1], "start")) {
        if (thread_running) {
            warnx("[YCM]already running\n");
            exit(0);
        }

        thread_should_exit = false;
        daemon_task = px4_task_spawn_cmd("rw_uart",
                         SCHED_DEFAULT,
                         SCHED_PRIORITY_MAX - 5,
                         3000,
                         rw_uart_thread_main,
                         (argv) ? (char * const *)&argv[2] : (char * const *)NULL);
        exit(0);
    }

    if (!strcmp(argv[1], "stop")) {
        thread_should_exit = true;
        exit(0);
    }

    if (!strcmp(argv[1], "status")) {
        if (thread_running) {
            warnx("[YCM]running");

        } else {
            warnx("[YCM]stopped");
        }

        exit(0);
    }

    usage("unrecognized command");
    exit(1);
}

int rw_uart_thread_main(int argc, char *argv[])
{

    if (argc < 1) {
        errx(1, "[YCM]need a serial port name as argument");
        usage("eg:");
    }

    const char *uart_name = "/dev/ttyS6";

    warnx("[YCM]opening port %s", uart_name);
    //char outdata1=0xe8;
    //char outdata2=0x02;
    //char outdata3=0xbc;
    char data= '0';
    char buffer[2] = "";
    // int result=0;

    warnx("data: %s\n", buffer);




    int uart_read = uart_init(uart_name);
    if(false == uart_read)return -1;
    if(false == set_uart_baudrate(uart_read,57600)){
        printf("[YCM]set_uart_baudrate is failed\n");
        return -1;
    }
    printf("[YCM]uart init is successful\n");
    shoot_total = 0;
    thread_running = true;



    //int j=0;

    // while(true)
    // {
    //     printf("fffffff \n");
    //     read(uart_read,&data,1);
    //    printf("data: %s\n",data);
    // }

    while(true)  
    // while(!thread_should_exit)
   {
        //write(uart_read,&outdata1,1);
        //printf("ccc%x\n",outdata1);
        //write(uart_read,&outdata2,1);
        //printf("ccc%x\n",outdata2);
        //write(uart_read,&outdata3,1);
        //printf("ccc%x\n",outdata3);

    	for(int i = 0;i <2;++i){
    	read(uart_read,&data,1);
    	buffer[i]=data;
    	}

    	// result=buffer[0]*256+buffer[1];

        // printf("data \n");
    	printf("data: %s\n", buffer);

    	// warnx("发布的是 == %d\n",result);


   }



    warnx("[YCM]exiting");
    thread_running = false;
    close(uart_read);

    fflush(stdout);
    return 0;
}

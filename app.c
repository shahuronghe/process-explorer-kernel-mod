/*
 ========================================================================================
 Author      : SHAHU RONGHE & SURYANSH KAUSHIK
 YEAR		 : FEBRUARY 2018
 License	 : GPL
 Description : This program communicates with the process explorer driver program and 
			   displays ongoing processes.
 ========================================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
struct process_data {
    unsigned long long prev_cal;
    unsigned long long cur_cal;
    unsigned long resident_size;
    char *process_name;
    int process_id;
    int parent_process_id;
    struct process_data *next;
};
int main() {
    struct process_data head[1024];
    
	int fd;
	char Ubuff[]="SAMPLE CHARDRIVER USING LOWLEVEL FRAMEWORK";
	char Kbuff[50];

	fd = open("/dev/sample_cdev0", O_RDWR);
	if(fd < 0) {
		perror("Unable to open the device file\n");
		return -1;
	}
        
	/* Write the data into the device */
	//write(fd , Ubuff , strlen(Ubuff) + 1);

	/* Read the data back from the device */
	//memset(Kbuff , 0 ,sizeof(Kbuff));
	read(fd , head , sizeof(struct process_data)*1024);
	printf("Data from kernel : %d\n",head[2].process_id);

	close(fd);	
	return 0;
}


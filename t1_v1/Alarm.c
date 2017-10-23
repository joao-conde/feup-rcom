#include "Alarm.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "DataLink.h"

#define TIMEOUT 3

int flag = 0;

void atende(int signal) {
	if(signal != SIGALRM)
		return;
	flag = 1;
	printf("Failed connecting, time out!\n\nRetrying:\n");
	alarm(TIMEOUT);
}

void setAlarm() {
	struct sigaction sa;
	sa.sa_handler = &atende;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;


	sigaction(SIGALRM, &sa, NULL);

	flag = 0;

	alarm(TIMEOUT);
}

void stopAlarm() {
	struct sigaction action;
	action.sa_handler = NULL;

	sigaction(SIGALRM, &action, NULL);

	alarm(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

typedef struct {
	long type;
	unsigned int accountnum;
	unsigned int accountpin;
	float funds;
	char operation[20];
} Message;

int main() {
	int accountnum = 1;
	unsigned int accountpin = 1;
	float accountbalance = 1;

	key_t key = ftok("DBserver.c", 'A');

	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	int msgid = msgget(key, IPC_CREAT | 0666);
	if (msgid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	Message msg;
	msg.type = 1;

	while(1) {

		Message msg;
		printf("account number(negative to exit): ");
		scanf("%5d", &accountnum);		

		while(getchar() != '\n');		

		if (accountnum < 0) {
			break;
		}
		

		printf("accountpin: ");
		scanf("%3d", &accountpin);

		while(getchar() != '\n');		

		printf("account balance: ");
		scanf("%f", &accountbalance);

		while(getchar() != '\n');

		printf("%05i %03i %011.02f\n", accountnum, accountpin, accountbalance);

		msg.accountnum = (unsigned int)accountnum;
		msg.accountpin = accountpin;
		msg.funds = accountbalance;	
		strcpy(msg.operation, "UPDATE_DB");
		msg.type = 1; 
		if (msgsnd(msgid, &msg, sizeof(msg),0 ) == -1) {
			perror("msgsnd");
			exit(EXIT_FAILURE);
		}


	}


}

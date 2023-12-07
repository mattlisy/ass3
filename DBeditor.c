#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

typedef struct {
	int accountnum;
	int accountpin;
	float funds;
	char operations[20];
} Data;

typedef struct {
	Data data;
	long type;
} Message;

int main() {
	int accountnum = 1;
	int accountpin = 1;
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
		printf("account number: ");
		scanf("%5d", &accountnum);		

		while(getchar() != '\n');		

		if (accountnum < 0) {
			break;
		}

		printf("accountpin: ");
		scanf("%3d", &accountpin);

		while(getchar() != '\n');		

		printf("account balance: ");
		scanf("%11f", &accountbalance);

		while(getchar() != '\n');

		printf("%05i %03i %011.2f\n", accountnum, accountpin, accountbalance);

		msg.data.accountnum = accountnum;
		msg.data.accountpin = accountpin;
		msg.data.funds = accountbalance;	
		strcpy(msg.data.operations, "UPDATE_DB");
		msg.type = 1; 
		if (msgsnd(msgid, &msg, sizeof(msg),0 ) == -1) {
			perror("msgsnd");
			exit(EXIT_FAILURE);
		}


	}

	msgctl(msgid, IPC_RMID, NULL);

}

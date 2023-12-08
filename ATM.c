#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_key_t.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

typedef struct {
	long type;
	unsigned int accountnum;
	unsigned int accountpin;
	float funds;
	char operation[20];

} Message;

int main() {
	int  accountnum;
	unsigned int accountpin;	
	int options;
	Message msg;
	Message pin_check;

	key_t key = ftok("DBserver.c", 'A');
	if (key == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	int msgid = msgget(key, IPC_CREAT | 0666);
	if (msgid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	int count = 0;

	do {
	
		printf("account number(negative for quit): ");
		scanf("%5d", &accountnum);
		while(getchar() != '\n');

		if(accountnum < 0) {
			exit(EXIT_SUCCESS);
		}


		printf("account pin: ");
		scanf("%3d", &accountpin);

		while(getchar() != '\n');

		msg.type = 1;
		msg.accountnum = (unsigned int) accountnum;
		msg.accountpin = accountpin;
		msg.funds = 0;
		strcpy(msg.operation, "PIN");

		// check pin 
		if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
			perror("msgsnd");
			exit(EXIT_FAILURE);
		}

		if (msgrcv(msgid, &pin_check, sizeof(msg), 2, 0) == -1) {
			perror("msgrcv");
			exit(EXIT_FAILURE);
		}
	
		count++;

	} while(strcmp(pin_check.operation, "PIN_WRONG") == 0 && count < 3);

	if (strcmp(pin_check.operation, "PIN_WRONG") == 0) {
		printf("account is blocked\n");
		return 0;

	}

	if (strcmp(pin_check.operation, "OK") == 0) {
		printf("Balance(1), Withdraw(2): ");
		scanf("%d", &options);

		switch (options) {
			case 1:
				// Balance 
				pin_check.type = 1;
				strcpy(pin_check.operation, "BALANCE");

				msgsnd(msgid, &pin_check, sizeof(pin_check), 0);
				msgrcv(msgid, &pin_check, sizeof(pin_check), 2, 0);

				printf("balance of account: %011.02f\n", pin_check.funds);
				break;
			case 2:	
				// Withdraw
				pin_check.type = 1;
				strcpy(pin_check.operation, "WITHDRAW");

				printf("Amount you would like to take out: ");
				scanf("%f", &pin_check.funds);
				while(getchar() != '\n');

				msgsnd(msgid, &pin_check, sizeof(msg), 0);
				msgrcv(msgid, &pin_check, sizeof(pin_check), 2, 0);

				if (strcmp(pin_check.operation, "FUNDS_OK") == 0) {
					printf("balance of account: %011.02f\n", pin_check.funds);
				} else if (strcmp(pin_check.operation, "NSF") == 0){
					printf("not enough funds are available");
				} else {
					perror("withdraw option");
					exit(EXIT_FAILURE);
				}
				break;	
			default: 
				printf("not an option\n");
				break;
		}	

	}	

}

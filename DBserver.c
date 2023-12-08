/*

   1. Waits for messages forever

   2. When a PIN message is received, it gets the account number and thePIN. It searches the account number in the DB. It then subtracts 1 from the PIN number and compares the result with the number stored in the DB (this is how the PIN number is “encrypted” in the DB). If the account number is found and the PIN is correct, return an “OK’ message, and saves the information on the account in a local variable. If there is a failure on the account or PIN numbers, it returns a “PIN_WRONG” message. After the 3rd attempt, the account is blocked. To do so, the first digit in the account number is converted to an “X” character.

   3. If the message is “BALANCE”, search the current account number, get the Funds field from the DB, and return it.

   4. If the message is “WITHDRAW”, get the amount requested, and check the funds available. If there are enough funds in the account, return “FUNDS_OK”, decrement the funds available, and update the file. If there is not enough money, return “NSF”

   5. If an “UPDATE_DB” message is received, the updated information is obtained and saved to the file (subtracting 1 to the PIN number      before saving it, to “encrypt” the PIN)

*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> 
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdbool.h>


#define FAILED_CHECK (Accountinfo){0, 0, 0, -1}

char Filename[20] = "test.txt";

typedef struct {
	unsigned int accountnum;
	unsigned int accountpin;
	float funds;	
	bool check;
	
} Accountinfo; 

typedef struct  {
	long type;
	unsigned int accountnum;
	unsigned int accountpin; 
	float funds;
	char operation[20];
} Message;

Accountinfo check_pin(unsigned int accountnum, unsigned int pinnum) {
	
	static int check_count = 0;
	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	Accountinfo saved_account;

	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {
		char* DBaccountnum = strtok(line, " "); 
		char* DBaccountpin = strtok(NULL, " ");
		// use space to seperate values 
		if (accountnum == atoi(DBaccountnum) && (pinnum -1) == atoi(DBaccountpin)) {

			saved_account.accountnum = atoi(DBaccountnum);

			saved_account.accountpin = atoi(DBaccountpin);

			saved_account.funds = atof(strtok(NULL, " "));
			
			saved_account.check = 0;
			fclose(db);
			printf("account found\n");
			return saved_account;

		}
			
		

	}

	if (check_count == 2) {
		fseek(db, 0, SEEK_SET);
		int cursorpos = 0;
		unsigned int pinnum = 0;
		float balance = 0;
		while (fgets(line, sizeof(line), db) != NULL) {
			printf("looping");	
			// does account exist?
			if (accountnum == atoi(strtok(line, " "))) {

				pinnum = atoi(strtok(NULL, " "));
				balance = atof(strtok(NULL, " "));

				printf("cursorpos: %i\n", cursorpos);
				fseek(db, cursorpos, SEEK_SET);
				int test = fprintf(db,"    X %03i %011.02f", pinnum, balance); 
				assert(test != -1);
				fclose(db);
				return FAILED_CHECK;
			}

			cursorpos = ftell(db);
		}
		// account does not exist 
		int test = fprintf(db,"    X %3i %011.02f\n", pinnum, balance); 			
		check_count = 0;
		assert(test != -1);
		fclose(db);
		return FAILED_CHECK;

	}

	check_count++;
	fclose(db);
	return FAILED_CHECK;
}

float get_balance(unsigned int accountnum) {

	FILE* db = fopen(Filename, "r");	
	assert(db != NULL);
	char line[64];	
	float balance;

	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {

		// use space to seperate values 
		if (accountnum == atoi(strtok(line, " "))) {

			strtok(NULL, " "); // skip pin	
			balance = atof(strtok(NULL, " ")); 
			printf("%011.02f\n", balance);
			fclose(db);
			return balance;
		}
	}
	// db changed account? 
	printf("account has been changed\n");
	fclose(db);
	return -1;
}




float withdraw(unsigned int accountnum, float amount) {

	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	float balance;
	int cursorpos = ftell(db);	
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {

		// use space to seperate values
		if (accountnum == atoi(strtok(line, " "))) {

			unsigned int accountpin = atoi(strtok(NULL, " "));
			balance = atof(strtok(NULL, " "));
			printf("balance before: %f", balance);
			if (amount > balance) {
				printf("insufficient funds, cannot withdraw\n");
				fclose(db);
				return -1;
			} else {
				balance -= amount;

				printf("new balance, %f\n", balance);
				// replace balance here one word ahead 	
				printf("cursorpos: %i\n", cursorpos);
				fseek(db, cursorpos, SEEK_SET);
				int test = fprintf(db,"%05i %03i %011.02f", accountnum, accountpin, balance); 
				assert(test != -1);
				fclose(db);
				return balance;
			}
		}
		cursorpos = ftell(db);
	}
	// db changed account? 
	printf("account has been changed\n");
	fclose(db);
	return -1;
}

void update_DB(unsigned int accountnum, unsigned int pinnum, float balance) {

	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	int cursorpos = ftell(db);	
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {

		// does account exist?
		if (accountnum == atoi(strtok(line, " "))) {

			// replace acc, using this cursor method does not account 
			// for bigger values meaning that the current balance must
			// be smaller or it no worky	
			printf("cursorpos: %i\n", cursorpos);
			fseek(db, cursorpos, SEEK_SET);
			int test = fprintf(db,"%05i %03i %011.02f", accountnum, --pinnum, balance); 
			assert(test != -1);
			fclose(db);
			return;
		}

		cursorpos = ftell(db);
	}
	// account does not exist 
	int test = fprintf(db,"%5i %3i %011.02f\n", accountnum, --pinnum, balance); 			
	assert(test != -1);
	fclose(db);
}








int main() {

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


	while(1){

		Message msg;
		printf("waiting\n");
		if (msgrcv(msgid, &msg, sizeof(msg), 1, 0) == -1) {
			perror("msgrcv");
			exit(EXIT_FAILURE);
		}

		printf("%s %i %i %011.2f\n", msg.operation, msg.accountnum, msg.accountpin, msg.funds);

		if (strcmp("UPDATE_DB", msg.operation) == 0) {
			update_DB(msg.accountnum, msg.accountpin, msg.funds);

		} else if (strcmp("BALANCE", msg.operation) == 0) {
			msg.funds = get_balance(msg.accountnum);
			msg.type = 2;
			if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
				perror("msgsnd withdraw");
				exit(EXIT_FAILURE);
			}


		} else if (strcmp("WITHDRAW", msg.operation) == 0){

			msg.funds = withdraw(msg.accountnum, msg.funds);
			msg.type = 2;
			if (msg.funds == -1) {
				strcpy(msg.operation, "NSF");
			} else {
				strcpy(msg.operation, "FUNDS_OK");
			}	
			if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
				perror("msgsnd withdraw");
				exit(EXIT_FAILURE);
			}

		} else if (strcmp("PIN", msg.operation) == 0) {
			Accountinfo usersacc = check_pin(msg.accountnum, msg.accountpin);

			Message return_pin;

			return_pin.type = 2;
			return_pin.accountnum = usersacc.accountnum;
			return_pin.accountpin = usersacc.accountpin;
			return_pin.funds = usersacc.funds;
			if (usersacc.check == 0) {
				strcpy(return_pin.operation, "OK");
			} else {
				strcpy(return_pin.operation, "PIN_WRONG");
			}

			if (msgsnd(msgid, &return_pin, sizeof(msg), 0) == -1) {
				perror("msgsnd return_pin");
				exit(EXIT_FAILURE);

			}
		} else {
			// not operation
			perror("not an operation");
			exit(EXIT_FAILURE); 
		}

	}
}

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


#define FAILED_CHECK (Accountinfo){"", "", -1};

char Filename[20] = "test.txt";

typedef struct {
	char accountnum[6];
	char accountpin[4];
	float funds;	
} Accountinfo; 

typedef struct {
	int accountnum;
	int accountpin; 
	float funds;
	char operation[20];
} Data;
typedef struct  {
	Data data;
	long type;
} Message;

Accountinfo check_pin(int accountnum, int pinnum) {
		
	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	Accountinfo saved_account;
	
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {
		char* DBaccountnum = strtok(line, " "); 
		char* DBaccountpin = strtok(NULL, " ");
		// use space to seperate values 
		printf("accountnum: %s\n", DBaccountnum);
		if (accountnum == atoi(DBaccountnum) && pinnum == atoi(DBaccountpin)) {
				
			strcpy(saved_account.accountnum, DBaccountnum);	

			strcpy(saved_account.accountpin, DBaccountpin);
			saved_account.funds = atof(strtok(NULL, " "));
			// msg "OK"
			fclose(db);
			printf("account found\n");
			return saved_account;

		}

	}
	// implement failure x3 
	printf("account not found\n");
	fclose(db);
	return FAILED_CHECK;
}

float get_balance(Accountinfo account) {
		
	FILE* db = fopen(Filename, "r");	
	assert(db != NULL);
	char line[64];	
	float balance;
	
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {
		
		// use space to seperate values 
		if (atoi(account.accountnum) == atoi(strtok(line, " "))) {
		
			strtok(line, " "); // skip pin	
			balance = atof(strtok(line, " ")); 
			fclose(db);
			return balance;
		}
	}
	// db changed account? 
	printf("account has been changed\n");
	fclose(db);
	return -1;
}




float withdraw(Accountinfo account, float amount) {

	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	float balance;
	int cursorpos = ftell(db);	
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {
		
		// use space to seperate values
		if (atoi(account.accountnum) == atoi(strtok(line, " "))) {
			strtok(NULL, " "); // skip pin
			balance = atof(strtok(NULL, " "));
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
				int test = fprintf(db,"%.5s %.3s %010.01f", account.accountnum, account.accountpin, balance); 
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

void update_DB(int accountnum, int pinnum, float balance) {
	
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
			int test = fprintf(db,"%5i %3i %010.01f", accountnum, pinnum, balance); 
			assert(test != -1);
			fclose(db);
			return;
		}

		cursorpos = ftell(db);
	}
	// account does not exist 
	int test = fprintf(db,"%5i %3i %011.02f", accountnum, pinnum, balance); 			
	assert(test != -1);
	fclose(db);
}








int main() {
	key_t key = ftok("DBserver.c", 'A');
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	int msgid = msgget(key, IPC_CREAT | 0666 | IPC_CREAT);
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

		printf("%s\n", msg.data.operation);
		/*
		   if (strcmp("PIN", msg.operation) == 0) {

		   } else if (strcmp("BALANCE", msg.operation) == 0) {

		//get_balance((Accountinfo){msg.accountnum, msg.accountpin, msg.funds});		
		} else if (strcmp("WITHDRAW", msg.operation) == 0){

		} else if (strcmp("UPDATE_DB", msg.operation) == 0) {

		} else {
		// not operation
		perror("not an operation");
		exit(EXIT_FAILURE);
		}
		*/
	}		
	msgctl(msgid, IPC_RMID, NULL);
}

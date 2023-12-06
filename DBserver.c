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

#define FAILED_CHECK (Accountinfo){"", "", -1};

char Filename[20] = "test.txt";

typedef struct {
	char accountnum[6];
	char accountpin[4];
	float funds;	
} Accountinfo; 

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
		if (accountnum == atoi(DBaccountnum) && pinnum == atoi(DBaccountpin)) {
				
			strncpy(saved_account.accountnum, DBaccountnum, sizeof(saved_account.accountnum)-1);	
			strncpy(saved_account.accountpin, DBaccountpin, sizeof(saved_account.accountpin)-1);
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
	printf("account has been changed");
	fclose(db);
	return -1;
}




float withdraw(Accountinfo account, int amount) {

	FILE* db = fopen(Filename, "r+");	
	assert(db != NULL);
	char line[64];	
	float balance;
	int cursorpos = ftell(db);	
	printf("Current file position: %ld\n", ftell(db));
	// file has 1 account per line
	while (fgets(line, sizeof(line), db) != NULL) {
		
		// use space to seperate values
		if (atoi(account.accountnum) == atoi(strtok(line, " "))) {
			strtok(NULL, " "); // skip pin
			balance = atof(strtok(NULL, " "));
			if (amount > balance) {
				printf("insufficient funds, cannot withdraw");
				fclose(db);
				return -1;
			} else {
				balance -= amount;
				printf("new balance, %f\n", balance);
				// replace balance here one word ahead 	
				fseek(db, cursorpos, SEEK_SET);
				int test = fprintf(db,"%s %s %0.2f", account.accountnum, account.accountpin, balance); 
				assert(test != -1);

				fclose(db);
				return balance;
			}

		}
		cursorpos = ftell(db);
	}
	// db changed account? 
	printf("account has been changed");
	fclose(db);
	return -1;
}




int main() {

	Accountinfo a1 = check_pin(11111, 111);		
	printf("%0.2f\n", get_balance(a1));
	withdraw(a1, 500);

}

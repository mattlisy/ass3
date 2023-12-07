#include <stdio.h>
#include <stdlib.h>


int main() {
	int accountnum;
	int accountpin;	
	int options;

	while (accountnum > -1) {
		
		printf("account number(negative for quit: ");
		scanf("%5d", &accountnum);

		while(getchar() != '\n');
			
		printf("account pin: ");
		scanf("%3d", &accountpin);

		while(getchar() != '\n');
			
		// look for account
		
		// if account is good -> choose service 
		printf("Balance(1), Withdraw(2): ");
		scanf("%d", &options);

		switch (options) {
			case 1:
				break;
			case 2:	
				break;	
			default: 
				printf("not an option\n");
				break;
		}

	}	


}

#include "BullsAndCows.h"

void menu() {
	int option;

	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Quit\n");
	scanf_s("%d", &option);
	while (option != 1 && option != 2) {
		printf("Invalid option. Try again\n");
		scanf_s("%d", &option);
	}
	if (1 == option)
		printf("1 was chosen.");
	else if (2 == option)
		printf("2 was chosen.");
}
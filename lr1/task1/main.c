#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include <utils.h>

status_t number_multiples(const int num) {
	int result[100];
	int i = 0;
	for (int x = 1; x <= 100; x++) {
		if (x % num == 0) {
			result[i++] = x;
		}
	}

	if (i == 0) {
		printf("В [1, 100] кратных числа '%d' нет", num);
	}
	for (int j = 0; j < i; j++) {
		printf("%d ", result[j]);
	}
	printf("\n");

	return STATUS_OK;
}

status_t number_is_prime(const int num) {
	bool is_prime = true;
	for (int x = 2; x < sqrt(num) + 1; x++) {
		if (num % x == 0) {
			is_prime = false;
			break;
		}
	}

	if (is_prime) { printf("'%d' является простым числом\n", num); }
	else { printf("'%d' является составным числом\n", num); }
	
	return STATUS_OK;
}

status_t number_in_hexadecimal(const int num) {
	if (num > pow(16, 100) || num < 1) { return STATUS_ERR_RANGE; }

	char digits[100];
	int i = 0;
	int cnum = num;
	while (cnum > 0) {
		int rem = cnum % 16;
		char c;
		if (rem > 9) { c = 'A' + (rem - 10); }
		else { c = '0' + rem; }
		digits[i++] = c;
		cnum /= 16; 
	}

	for (int j = i - 1; j >= 0; j--) {
		printf("%c ", digits[j]);
	}
	printf("\n");
	return STATUS_OK;
}

status_t degree_table(const int num) {
	if (num > 10 || num < 1) { return STATUS_ERR_RANGE; }

	long table[10][num];
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < num; j++) {
			table[i][j] = (long)pow(i+1, j+1);
		}
	}

	for (int i = 0; i < 10; i++) {
        for (int j = 0; j < num; j++) {
            printf("%ld ", table[i][j]);
        }
		printf("\n");
    }
	return STATUS_OK;
}

status_t sum_of_natural(const int num) {
	long result = 0;
	for (int i = 1; i <= num; i++) {
		result += i;
	}

	printf("%ld\n", result);
	return STATUS_OK;
}

status_t factorial(const int num) {
	if (num > 50 || num < 1) { return STATUS_ERR_RANGE; }	

	unsigned long long result = 1;
    for (int i = 2; i <= num; i++) {
        result *= i;
    }

    printf("%llu\n", result);
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Формат запуска программы: [программа] [число] -[флаг]\n");
		return 0;
	}

	int num;
	if (sscanf(argv[1], "%d", &num) != 1) {
		fprintf(stderr, "'%s' не число\n", argv[1]);
		return 0;
	}

	char flag;
	if (sscanf(argv[2], "-%c", &flag) != 1 || argv[2][2] != '\0') {
		fprintf(stderr, "'%s' не флаг формата -X\n", argv[2]);
        return 0;
	}
	
	status_t status;
	switch (flag) {
		case 'h': number_multiples(num); break;
		case 'p': number_is_prime(num); break;
		case 's': 
			status = number_in_hexadecimal(num); 
			if (status == STATUS_ERR_RANGE) {
				printf("Число слишком маленькое или "
				"слишком большое для того чтобы представить"
				"его в системе счисление с основанием 16");
			}
			break;
		case 'e':
			status = degree_table(num);
            if (status == STATUS_ERR_RANGE) {
                printf("Число не должно превышать 10\n");
            }
            break;
		case 'a': sum_of_natural(num); break;
		case 'f': 
			status = factorial(num);
            if (status == STATUS_ERR_RANGE) {
                printf("Факториалы выше 50 нельзя вместить в unsigned long long\n");
            }
            break;
	}
}

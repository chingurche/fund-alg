#include <utils.h>

#include <stdio.h>
#include <math.h>
#include <stdbool.h>


status_t number_multiples(const int num, int *out, size_t *out_size) {
	if (num == 0) { return STATUS_ERR_RANGE; }

	size_t i = 0;
	for (int x = 1; x <= 100; x++) {
		if (x % num == 0) {
			out[i++] = x;
		}
	}

	*out_size = i;
	return STATUS_OK;
}

status_t number_is_prime(const int num, bool *out) {
	if (num <= 1) {
		*out = false;
		return STATUS_OK;
	}
	if (num == 2) {
		*out = true;
		return STATUS_OK;
	}
	
	bool is_prime = true;
	for (int x = 3; x < sqrt(num) + 1; x++) {
		if (num % x == 0) {
			is_prime = false;
			break;
		}
	}

	*out = is_prime;
	return STATUS_OK;
}

status_t number_in_hexadecimal(const int num, char *out, size_t *out_size) {
	if (num < 1) { return STATUS_ERR_RANGE; }

	size_t i = 0;
	int cnum = num;
	while (cnum > 0) {
		int rem = cnum % 16;
		char c;
		if (rem > 9) { c = 'A' + (rem - 10); }
		else { c = '0' + rem; }
		out[i++] = c;
		cnum /= 16; 
	}

	*out_size = i;
	return STATUS_OK;
}

status_t degree_table(const int num, size_t out[10][num]) {
	if (num > 10 || num < 1) { return STATUS_ERR_RANGE; }

	for (size_t i = 0; i < 10; i++) {
		for (size_t j = 0; j < num; j++) {
			out[i][j] = (size_t)pow(i+1, j+1);
		}
	}
	return STATUS_OK;
}

status_t sum_of_natural(const int num, size_t *out) {
	size_t result = 0;
	for (size_t i = 1; i <= num; i++) {
		result += i;
	}

	*out = result;
	return STATUS_OK;
}

status_t factorial(const int num, size_t *out) {
	if (num > 20 || num < 1) { return STATUS_ERR_RANGE; }	

	size_t result = 1;
    for (size_t i = 2; i <= num; i++) {
        result *= i;
    }

	*out = result;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Формат запуска программы: [программа] [число] -[флаг]\n");
		return 0;
	}

	int num;
	status_t rc = parse_int(argv[1], &num);
	switch (rc) {
		case STATUS_ERR_FORMAT:
			fprintf(stderr, "'%s' не целое число\n", argv[1]);
			return 0;
		case STATUS_ERR_RANGE:
			fprintf(stderr, "'%s' вне диапазона int\n", argv[1]);
			return 0;
		case STATUS_ERR_NULL_ARG:
			fprintf(stderr, "NULL\n", argv[1]);
			return 0;
	}

	char flag;
	if (sscanf(argv[2], "-%c", &flag) != 1 || argv[2][2] != '\0') {
		fprintf(stderr, "'%s' не флаг формата -X\n", argv[2]);
        return 0;
	}

	switch (flag) {
		case 'h': {
			int result[100];
			size_t count;
			status_t rc = number_multiples(num, result, &count);

			if (rc == STATUS_ERR_RANGE) {
				fprintf(stderr, "Делитель равен нулю\n");
				return 0;
			}

			if (count == 0) {
				printf("В [1, 100] кратных числа '%d' нет", num);
			}
			for (int j = 0; j < count; j++) {
				printf("%d ", result[j]);
			}
			printf("\n");
			break;
		}
		case 'p': {
			bool is_prime;
			number_is_prime(num, &is_prime);
			
			if (is_prime) { printf("'%d' является простым числом\n", num); }
			else { printf("'%d' является составным числом\n", num); }
			break;
		}
		case 's': {
			char digits[100];
			size_t count;
			status_t rc = number_in_hexadecimal(num, digits, &count); 
			if (rc == STATUS_ERR_RANGE) {
				fprintf(stderr, "Число слишком маленькое или "
				"слишком большое для того чтобы представить"
				"его в системе счисление с основанием 16");
				return 0;
			}

			for (int j = count - 1; j >= 0; j--) {
				printf("%c ", digits[j]);
			}
			printf("\n");
			break;
		}
		case 'e': {
			size_t table[10][num];
			status_t rc = degree_table(num, table);
            if (rc == STATUS_ERR_RANGE) {
                fprintf(stderr, "Число не должно превышать 10\n");
				return 0;
            }
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < num; j++) {
					printf("%zu ", table[i][j]);
				}
				printf("\n");
			}
            break;
		}
		case 'a': {
			size_t sum;
			sum_of_natural(num, &sum);
			
			printf("%zu\n", sum);
			break;
		}
		case 'f': {
			size_t result;
			status_t rc = factorial(num, &result);
            if (rc == STATUS_ERR_RANGE) {
                fprintf(stderr, "Факториалы выше 20 нельзя вместить даже в size_t\n");
				return 0;
            }
    		printf("%zu\n", result);
            break;
		}
		default: {
			fprintf(stderr, "Существующие флаги: h, p, s, e, a, f\n");
			return 0;
		}
	}
}
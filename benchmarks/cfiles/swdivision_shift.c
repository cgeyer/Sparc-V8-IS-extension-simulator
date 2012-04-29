/* Copyright Clemens Geyer 2011 */
#include <stdint.h>

static uint16_t divide_shift(uint16_t a, uint16_t b) {

	uint16_t result = 0;
	uint16_t divisor = b;

	if (b == 0) {
		return UINT16_MAX;
	}

	while (b < a && !(b & (uint16_t) (1<<15))) {
		b = b << 1;
	}

	while (b >= divisor) {
		result = result << 1;
		if (a >= b) {
			result = result + 1;
			a = a - b;
		}
		b = b >> 1;
	}

	return result;

}

int main(int argc, char** argv) {

	uint16_t result;
	int i;

	for (i = 0; i <= UINT16_MAX ; i++) {
		asm("sim-clearcycles");
		result = divide_shift(UINT16_MAX, i);
		asm("sim-printcycles");
		if (i == 0 && result != UINT16_MAX) {
			return -1;
		}
		if (i != 0 && result != UINT16_MAX / i) {
			return -1;
		}
	}

	return 0;

}

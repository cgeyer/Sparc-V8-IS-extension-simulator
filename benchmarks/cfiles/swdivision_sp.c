/* Copyright Clemens Geyer 2011 */
#include <stdint.h>

static uint16_t divide_wcet(uint16_t a, uint16_t b) {

	int i = 0;
	int boolean1;
	int boolean2;

	uint16_t result = 0;
	uint16_t divisor = b;

	for (i = 0; i < 16; i++) {
		boolean1 = (b & (uint16_t) (1 << 15));
		b = boolean1 ? b : b << 1;
	}

	for (i = 0; i < 16; i++) {
		boolean1 = (b >= divisor);
		result = boolean1 ? result << 1 : result;
		boolean2 = (a >= b);
		result = (boolean1 && boolean2) ? result + 1 : result;
		a = (boolean1 && boolean2) ? a - b : a;
		b = boolean1 ? b >> 1 : b;
	}

	boolean1 = (b != 0);
	result = boolean1 ? result : UINT16_MAX;
	return result;

}


int main(int argc, char** argv) {

	uint16_t result;
	int i;

	for (i = 0; i <= UINT16_MAX ; i++) {
		asm("sim-clearcycles");
		result = divide_wcet(UINT16_MAX, i);
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

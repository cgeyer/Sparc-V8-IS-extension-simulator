/* Copyright Clemens Geyer 2011 */
#include <stdint.h>

static uint16_t divide_simple(uint16_t a, uint16_t b) {
	uint16_t result = 0;
	if (b == 0) {
		return UINT16_MAX;
	}
	while ((int32_t) (a - b) >= 0) {
		a = a - b;
		result++;
	}
	return result;
}

int main(int argc, char** argv) {

	uint16_t result;
	uint32_t i;

	for (i = 0; i <= UINT16_MAX ; i++) {
		asm("sim-clearcycles");
		result = divide_simple(UINT16_MAX, i);
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

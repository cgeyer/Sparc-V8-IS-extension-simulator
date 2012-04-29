/* Copyright Clemens Geyer 2011 */
#define MATRIX_SIZE 8

static int matrix[MATRIX_SIZE][MATRIX_SIZE] = {
	{1, 2, 3, 4, 5, 6, 7, 8},
	{2, 3, 4, 5, 6, 7, 8, 9},
	{3, 4, 5, 6 ,7, 8, 9, 10},
	{4, 5, 6, 7, 8, 9, 10, 11},
	{5, 6, 7, 8, 9, 10, 11, 12},
	{6, 7, 8, 9, 10, 11, 12, 13},
	{7, 8, 9, 10, 11, 12, 13, 14},
	{8, 9, 10, 11, 12, 13, 14, 15}
};

static void matrix_sum(	int** inputmatrix, 
						int* rowsums, 
						int* colsums, 
						int size) {
	int i, j;
	// init loop
	for (i = 0; i < size; i++) {
		// init rowsums[i] with 0
		rowsums[i] = 0;
		// init colsums[i] with 0
		colsums[i] = 0;
	}
	// sum up rows and columns
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			rowsums[i] += matrix[i][j];
			colsums[j] += matrix[i][j];
		}
	}
}



int main(void) {

	int rowsums[MATRIX_SIZE];
	int colsums[MATRIX_SIZE];

	asm("sim-clearcycles");
	matrix_sum((int**) matrix, rowsums, colsums, MATRIX_SIZE);
	asm("sim-printcycles");

	return 0;
}

#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

using namespace std;

void generateMatrix(vector<vector<int>>& matrix, int begin, int end, int size)
{
	for (int i = begin; i < end; i++)
	{
		int n = i / size;
		int m = i % size;

		matrix[n][m] = rand() % 200 - 100;
	}
}

void diagonalMin(vector<vector<int>>& matrix, int begin, int end, int size)
{
	for (int i = begin; i < end; i++)
	{
		int n = i / size;
		int min = 200;

		for (int m = 0; m < size; m++)
		{
			if (matrix[n][m] < min)
			{
				min = matrix[n][m];
			}
		}
		matrix[n][size - n - 1] = min;
	}
}

void checkDiagonal(const vector<vector<int>>& matrix, int size)
{
	for (int i = 0; i < size; i++) {
		int minVal = 200;
		for (int j = 0; j < size; j++) {
			if (matrix[i][j] < minVal) {
				minVal = matrix[i][j];
			}
		}
		if (matrix[i][size - i - 1] != minVal) {
			cout << "Error in diagonal element at row " << i << endl;
		}
	}
}


void threadsCalc(int size, int threadsNum)
{
	vector<vector<int>> matrix(size, vector<int>(size, 0));
	vector<thread> threads;

	int matrixElementsNum = size * size;
	int chunkSize = (matrixElementsNum + threadsNum - 1) / threadsNum;

	auto startTime = chrono::high_resolution_clock::now();

	for (int i = 0; i < threadsNum; i++)
	{
		int begin = i * chunkSize;
		int end = begin + chunkSize;
		threads.emplace_back(generateMatrix, ref(matrix), begin, end, size);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
	threads.clear();

	for (int i = 0; i < threadsNum; i++)
	{
		int begin = i * chunkSize;
		int end = begin + chunkSize;
		threads.emplace_back(diagonalMin, ref(matrix), begin, end, size);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	auto endTime = chrono::high_resolution_clock::now();


	chrono::duration<double> elapsed = endTime - startTime;
	cout << "Threads: " << threadsNum << " Time: " << elapsed.count() << " seconds" << endl;

	//checkDiagonal(matrix, size);
}

int main() {

	int choice = 0;
	int matrixSize;
	int threadsVariants[] = { 1, 3, 6, 12, 24, 48, 96, 192 };

	srand(time(NULL));

	cout << "Select matrix size" << endl;

	cin >> choice;

	switch (choice)
	{
	case 1:
		matrixSize = 120;
		break;

	case 2:
		matrixSize = 1200;
		break;

	case 3:
		matrixSize = 12000;
		break;

	default:
		cout << "invalid" << endl;
		return 0;
	}

	for (int threadsNum : threadsVariants)
	{
		threadsCalc(matrixSize, threadsNum);
	}

	return 0;
}


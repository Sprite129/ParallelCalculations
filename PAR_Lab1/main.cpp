#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

using namespace std;

void threadsCalc(int size, int threadsNum)
{
	vector<vector<int>> matrix(size, vector<int>(size, 0));
	vector<thread> threads;

	int matrixElementsNum = size * size;
	int chunkSize = (matrixElementsNum + threadsNum - 1) / threadsNum;

	for (int i = 0; i < threadsNum; i++)
	{
		int begin = i * chunkSize;
		int end = begin + chunkSize;
		//threads.emplace_back();
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
		//threads.emplace_back();
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	if (size == 120) {
		cout << "Matrix after processing:" << endl;
		//printMatrix(matrix);
	}
}

int main() {

	int choice = 0;
	int marixSize;
	int threadsVariants[] = { 1, 3, 6, 12, 24, 48, 96, 192 };

	cout << "Select matrix size" << endl;

	cin >> choice;

	switch (choice)
	{
	case 1:
		cout << "Size 120" << endl;
		break;

	case 2:
		cout << "Size 1200" << endl;
		break;

	case 3:
		cout << "Size 12000" << endl;
		break;

	default:
		cout << "invalid" << endl;
		break;
	}

	return 0;
}


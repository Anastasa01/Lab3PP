#include <iostream>
#include <time.h>
#include "mpi.h"

using namespace std;

int main(int argc, char** argv){
	srand(time(0));
	int id;
	int num;
	int N;
	int count;
	int remains;
	int size;
	int* a = NULL;
	int* sendcounts = NULL;
	int* displs = NULL;
	bool fail = false;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	//double time = MPI_Wtime();

	if (id == 0)
	{
		do {
			fail = false;
			cout << "Number of elements in the array = ";
			cin >> N;
			if (cin.fail() || (N <= 0)) {
				cout << "Error. Number of elements in the array = " << endl;
				fail = true;
			}
			cin.clear();
			cin.ignore();
		} while (fail);

		a = new int[N];

		double sum1 = 0;
		for (int i = 0; i < N; ++i)
		{
			a[i] = 1 + rand() % 100;
			cout << a[i] << ' ';
		}

		for (int i = 0; i < N; ++i)
		{
			sum1 += sqrt(a[i]);
		}

		cout << endl;
		cout << "Sum = " << sum1 << endl;

		sendcounts = new int[num];
		displs = new int[num];

		count = N / num;
		remains = N - count * num;
		int prefixSum = 0;
		for (int i = 0; i < num; ++i)
		{
			sendcounts[i] = (i < remains) ? count + 1 : count;
			displs[i] = prefixSum;
			prefixSum += sendcounts[i];
		}
	}


	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (0 != id)
	{
		count = N / num;
		remains = N - count * num;
	}
	size = id < remains ? count + 1 : count;

	int* res = new int[size];

	MPI_Scatterv(a, sendcounts, displs, MPI_INT, res, size, MPI_INT, 0, MPI_COMM_WORLD);

	if (0 == id)
	{
		delete[] sendcounts;
		delete[] displs;
		delete[] a;
	}

	float sum2 = 0;

#pragma omp parallel for reduction(+:sum2)
	for (int i = 0; i < size; ++i) {
		sum2 += sqrt(res[i]);
	}
	delete[] res;

	float sum3;
	for (int i = 1; i < num; i *= 2)
	{
		if (id % (i * 2) != 0)
		{
			MPI_Send(&sum2, 1, MPI_INT, id - i, 0, MPI_COMM_WORLD);
			break;
		}
		else if (id + i < num)
		{
			MPI_Recv(&sum3, 1, MPI_INT, id + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			sum2 += sum3;
		}
	}

	//time = MPI_Wtime() - time;
	//cout << "Computation time = " << time << "\n";

	MPI_Finalize();

	if (id == 0)
	{
		cout << "Computed sum = " << sum2 << endl;
#if _DEBUG
		system("pause");
#endif
	}
	return EXIT_SUCCESS;

}
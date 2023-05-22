#include <iostream>

#include <future>

#include <random>
#include <ctime>

#include <chrono>

using namespace std;

long ThreadedSum(int* begin, int* end)
{
	long sum = 0;
	while (begin != end)
	{
		sum += *begin;
		begin++;
	}
	return sum;
}

class MultiThreading
{
private:
	int* Array;
	size_t Size;
	size_t Parts;

	vector<future<long>> futures;

public:
	MultiThreading(size_t size, size_t parts)
	{
		size += parts - size % parts;

		Size = size;
		Parts = parts;
		Array = new int[size];
	}
	~MultiThreading()
	{
		delete[] Array;
	}

	double Run(unsigned int times = 1, launch politics = launch::async)
	{
		chrono::duration<double> TotalDuration(0);
		for (unsigned int run = 0; run < times; run++)
		{
			long EstimatedSum = 0;
			for (size_t i = 0; i < Size; i++)
			{
				Array[i] = rand() % 256;
				EstimatedSum += Array[i];
			}
			cout << "Estimated sum:\t" << EstimatedSum << endl;

			chrono::steady_clock::time_point StartTime = chrono::steady_clock::now();
			for (size_t i = 0; i < Parts; i++)
			{
				future<long> future = async(politics, 
					bind(ThreadedSum, Array + (i * Size / Parts), Array + ((i + 1) * Size / Parts)));
				futures.push_back(move(future));
			}

			long ThreadsSum = 0;
			for (size_t i = 0; i < Parts; i++)
			{
				ThreadsSum += futures.back().get();
				futures.pop_back();
			}
			chrono::steady_clock::time_point EndTime = chrono::steady_clock::now();
			chrono::duration<double> Duration = EndTime - StartTime;

			cout << "Calculated sum:\t" << ThreadsSum << endl 
				 << "Time spend:\t" << Duration.count() << endl;
			TotalDuration += Duration;
		}

		cout << "Average time:\t" << TotalDuration.count() / times << endl << endl;
		return TotalDuration.count() / times;
	}
};

int main()
{
	srand(time(0));

	int ArraySize = 1048576;
	int RunsCount = 32;
	launch LaunchPolitics = launch::async;

	MultiThreading MT1(ArraySize, 1);
	double MT1Result = MT1.Run(RunsCount, LaunchPolitics);

	MultiThreading MT2(ArraySize, 2);
	double MT2Result = MT2.Run(RunsCount, LaunchPolitics);

	MultiThreading MT4(ArraySize, 4);
	double MT4Result = MT4.Run(RunsCount, LaunchPolitics);

	MultiThreading MT8(ArraySize, 8);
	double MT8Result = MT8.Run(RunsCount, LaunchPolitics);

	MultiThreading MT16(ArraySize, 16);
	double MT16Result = MT16.Run(RunsCount, LaunchPolitics);

	cout << endl;
	cout << "   ArraySize:\t" << ArraySize << endl;
	cout << "1  thread time:\t" << MT1Result << endl;
	cout << "2  thread time:\t" << MT2Result << endl;
	cout << "4  thread time:\t" << MT4Result << endl;
	cout << "8  thread time:\t" << MT8Result << endl;
	cout << "16 thread time:\t" << MT16Result << endl;


	return 0;
}
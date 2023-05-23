#include <iostream>
#include <queue>

#include <future>
#include <chrono>
#include <mutex>

#include <random>
#include <ctime>

#include <conio.h>

using namespace std;

/* Part 1 */
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

/* Part 2 */
struct Generator
{
	string Name;
	short Delay = 0;
	int MinValue = 0;
	int MaxValue = 0;
};
atomic<bool> StopFlag(false);
mutex mut;
queue<pair<string, int>> ProcessorQueue;
void GeneratorThread(Generator Gen)
{
	int Data;
	while (!StopFlag)
	{
		mut.lock();
		Data = Gen.MinValue + (rand() % (Gen.MaxValue - Gen.MinValue));
		ProcessorQueue.emplace(Gen.Name, Data);
		//cout << "Generated data: " << Data << "\tfrom generator: " << Gen.Name << endl;
		mut.unlock();
		this_thread::sleep_for(chrono::seconds(Gen.Delay));
	}
}
void ProcessorThread()
{
	while (!StopFlag)
	{
		mut.lock();
		if (ProcessorQueue.size() > 0)
		{
			pair<string, int> Data = ProcessorQueue.front();
			cout << "Processing data: " << Data.second << "\tfrom generator: " << Data.first << endl;
			ProcessorQueue.pop();
		}
		mut.unlock();
	}
}
class GenProcSystem
{
private:
	vector<Generator> generators;
	vector<future<void>> futures;
public:
	GenProcSystem() {}
	~GenProcSystem() {}
	void AddGenerator(string name, short delay, int minValue = 0, int maxValue = 256)
	{
		Generator newGen;
		newGen.Name = name;
		newGen.Delay = delay;
		newGen.MinValue = minValue;
		newGen.MaxValue = maxValue;
		generators.push_back(newGen);
	}
	void Run()
	{
		cout << endl;
		cout << "Press any key to stop." << endl;
		cout << "Generator-Processor Started!" << endl;

		future<void> processor = async(launch::async, ProcessorThread);
		futures.push_back(move(processor));

		for (Generator Gen : generators)
		{
			future<void> generator = async(launch::async, bind(GeneratorThread, Gen));
			futures.push_back(move(generator));
		}

		_getch();
		StopFlag = true;
		cout << "Stopping Generator-Processor!" << endl;

		for (size_t i = 0; i < futures.size(); i++)
		{
			futures.back().get();
			futures.pop_back();
		}
		cout << "Generator-Processor Stoped!" << endl;
	}
};

int main()
{
	srand(time(0));

	/* Part 1: Parallel processing of array */
	int ArraySize = 1048576;
	int RunsCount = 16; // 0 to skip this part
	launch LaunchPolitics = launch::async;

	if (RunsCount > 0)
	{
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

		cout << "   ArraySize:\t" << ArraySize << endl;
		cout << "1  thread time:\t" << MT1Result << endl;
		cout << "2  thread time:\t" << MT2Result << endl;
		cout << "4  thread time:\t" << MT4Result << endl;
		cout << "8  thread time:\t" << MT8Result << endl;
		cout << "16 thread time:\t" << MT16Result << endl;
	}

	/* Part 2: Generator - Processor */

	GenProcSystem gps;
	gps.AddGenerator("Gen1s", 1, 0, 16); // name, delay(seconds), [minimal generated value], [maximal generated value]
	gps.AddGenerator("Gen3s", 3, 16, 32);
	gps.AddGenerator("Gen7s", 7, 32, 64);
	gps.Run();

	return 0;
}
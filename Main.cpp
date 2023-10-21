#include <iostream>
#include <queue>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <semaphore>
#include "mySemaphore.h"

#define CONSUMERS_COUNTS 8
#define PRODUCER_SLEEP_TIME 400ms
#define CONSUMERS_SLEEP_TIME 1000ms


//##### Will wokr only one version #########
//#define CUSTOM_SEMAPHOR    // Elapsed time: 40.9553
//#define MUTEX                // Elapsed time: 41.3669
#define CXX20              //  Elapsed time: 40.4093



//OUTPUT HELPER 
// # The symbol means that this output from the producer
// * The symbol means that this output from the consumer


//##### NAMESPACE ############
using namespace std::literals;
namespace SharedMembers
{
	bool file_EOF_status(false);
	bool work_finished(false);
	std::queue<int> buffer; 
	std::counting_semaphore<CONSUMERS_COUNTS> sem(CONSUMERS_COUNTS);
	std::mutex mx;

	#ifdef CUSTOM_SEMAPHOR
	semaphore MySem;
	#endif // CUSTOM_SEMAPHOR
}


void producer(std::string file_path)
{
	std::cout << "Producer Thread ID = " << std::this_thread::get_id() << std::endl;
	std::string num;
	std::fstream File(file_path);

	using namespace std::literals;

	if (File.is_open())
	{
		while (std::getline(File,num))
		{
			std::this_thread::sleep_for(PRODUCER_SLEEP_TIME); // producer's run time for pushing one number to the buffer 
			std::cout << "######## -> " << "Producer pushed number -> " << std::stoi(num) << " to buffer." << std::endl;
			SharedMembers::buffer.push(std::stoi(num));
#			ifdef CUSTOM_SEMAPHOR
			SharedMembers::MySem.release();
			#endif // CUSTOM_SEMAPHOR
		}
		if (File.eof())
		{
			std::cout << "######## -> " << "The producer wrote all numbers from the file to the buffer." << std::endl;
			SharedMembers::file_EOF_status = true;
		}
		File.close();
	}
	else
	{
		std::cout << "Unable to open file";
		exit(1);
	}
}

// #define CXX20
#ifdef CXX20
void consumer() {
	while (true) {
		SharedMembers::sem.acquire();
		if (!SharedMembers::buffer.empty()) {
			if (CONSUMERS_COUNTS - SharedMembers::buffer.size() > 0 && CONSUMERS_COUNTS - SharedMembers::buffer.size() <= CONSUMERS_COUNTS) {
				std::cout << "******** -> " << CONSUMERS_COUNTS - SharedMembers::buffer.size() << "  Consumers waiting for work" << std::endl;
			}

			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);

			SharedMembers::mx.lock();
			if (!SharedMembers::buffer.empty()) {
				std::cout << "******** -> " << "Consumer by THREAD ID -> " << std::this_thread::get_id() << "  works and prints number -> " << SharedMembers::buffer.front() * 2 << std::endl;
				SharedMembers::buffer.pop();
			}
			else {
				SharedMembers::mx.unlock(); // Release the mutex before exiting the loop
				break;
			}
			SharedMembers::mx.unlock();
		}
		else {
			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);
			std::cout << "Consumer by THREAD ID -> " << std::this_thread::get_id() << " is waiting because the buffer is empty" << std::endl;
			if (SharedMembers::buffer.empty() && SharedMembers::file_EOF_status)
			{
				SharedMembers::work_finished = true;
				break;
			}
		}
		SharedMembers::sem.release();
	}
}
#endif 

//#define MUTEX
#ifdef MUTEX
void consumer()
{
	//std::cout << "Consumer Thread ID = " << std::this_thread::get_id() << std::endl;
	while (true) {
		
		
		if (!SharedMembers::buffer.empty()) {
			if (CONSUMERS_COUNTS - SharedMembers::buffer.size() > 0 && CONSUMERS_COUNTS - SharedMembers::buffer.size() <= CONSUMERS_COUNTS) {
				std::cout << "******** -> " << CONSUMERS_COUNTS - SharedMembers::buffer.size() << "  Consumers waiting for work" << std::endl;
			}

			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);

			SharedMembers::mx.lock();
			if (!SharedMembers::buffer.empty()) {
				std::cout << "******** -> " << "Consumer by THREAD ID -> " << std::this_thread::get_id() << "  works and prints number -> " << SharedMembers::buffer.front() * 2 << std::endl;
				SharedMembers::buffer.pop();
			}
			SharedMembers::mx.unlock();
		}
		else {
			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);
			std::cout << "Consumer by THREAD ID -> " << std::this_thread::get_id() << " is waiting because the buffer is empty" << std::endl;
			if (SharedMembers::buffer.empty() && SharedMembers::file_EOF_status)
			{
				SharedMembers::work_finished = true;
			}
		}
		
	}
}
#endif 

//#define CUSTOM_SEMAPHOR
#ifdef CUSTOM_SEMAPHOR
void consumer()
{
	//std::cout << "Consumer Thread ID = " << std::this_thread::get_id() << std::endl;
	while (!(SharedMembers::file_EOF_status && SharedMembers::buffer.empty()))
	{
		if (!SharedMembers::buffer.empty())
		{
			SharedMembers::MySem.acquire();
			if (CONSUMERS_COUNTS - SharedMembers::buffer.size() >= 0 && CONSUMERS_COUNTS - SharedMembers::buffer.size() <= CONSUMERS_COUNTS)
			{
				std::cout << "******** -> " << CONSUMERS_COUNTS - SharedMembers::buffer.size() << "  Consumers waiting for work" << std::endl;
			}
			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);
			std::cout << "******** -> " << "Consumer by THREAD ID -> " << std::this_thread::get_id() << "  works and prints number -> " << SharedMembers::buffer.front() * 2 << std::endl;
			SharedMembers::buffer.pop();
		}
		else
		{
			std::this_thread::sleep_for(CONSUMERS_SLEEP_TIME);
			//std::cout << "Consumer by THREAD ID -> " << std::this_thread::get_id() << " is waiting because the buffer is empty" << std::endl;
		}
	}
	
	SharedMembers::work_finished = true;	
	

}
#endif 

void duration()
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	while (!SharedMembers::work_finished);
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = end - start;

	if (SharedMembers::buffer.empty() && SharedMembers::file_EOF_status)
	{
		std::cout << "EOF & buffer is empty | works done" << std::endl;
	}
	std::cout << "Elapsed time: " << elapsed_time.count() << std::endl;
	
	exit(0);
}



int main()
{
	std::thread duration_thread(&duration);

	std::cout << "Consumers: " << CONSUMERS_COUNTS << std::endl;
	std::cout << "Producer: 1" << std::endl;

	std::thread producer_thread(&producer, "numbers.txt");
	std::cout << "MAIN THREAD ID -> " << std::this_thread::get_id() << std::endl;


	std::thread consumer_threads[CONSUMERS_COUNTS];
	for (int i = 0; i < CONSUMERS_COUNTS; ++i)
	{
		consumer_threads[i] = std::thread(&consumer);
	}

	producer_thread.join();
	for (int i = 0; i < CONSUMERS_COUNTS; ++i)
	{
		consumer_threads[i].join();
	}	
}
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <thread>
#include <semaphore.h>
#include <stdatomic.h>
#include "util.h"

#define BUFF_SIZE 10
size_t str_size = 30;
std::string buffer[BUFF_SIZE];

std::atomic<bool> producer_complete(false);

bool activeParser = false;


static int item_count = 0;

sem_t *mutex;
sem_t *spaces_avl;
sem_t *spaces_fld;


void parse() {
	int in = 0;
	for (int n = 1; n < 6; n++) {
		std::ifstream file("input/names" + std::to_string(n) + ".txt");
		std::string line;

		
		if (!file) {
			std::cerr << "File Error" << std::endl; 
			exit(1);
		}
		while (std::getline(file, line)) {

			if (line != "\n") {
				// cout << line << endl;
				while (item_count == BUFF_SIZE);
				
				sem_wait(mutex);
				/* ------ CS ------- */
				buffer[in] = line;
				item_count++;
				/* ----- END ------- */
				
				sem_post(mutex);
				in = (in + 1) % BUFF_SIZE;
			}
		}
		producer_complete = true;

		file.close();
	}
}

void convert() {
	int out = 0;
	std::string to_convert;
	char converted[30];
	while (!producer_complete || item_count > 0)
	{	
		while(item_count == 0);
			
		sem_wait(mutex);
		/* ------ CS ------- */
		to_convert = buffer[out];
		item_count--;
		/* ----- END ------- */
		sem_post(mutex);

		//cout << to_convert << endl;
			
		out = (out + 1) % BUFF_SIZE;

		if (dnslookup(to_convert.c_str(), converted, 30) == -1) {
			std::cerr << "Unable to resolve domain name " << to_convert << std::endl;
			std::cout << to_convert << ", " << std::endl;
		} else 
			std::cout << to_convert << ", " << std::string(converted) << std::endl;

	}

}

void Sem_Open(sem_t *sem, std::string name, int count){
	if ((sem = sem_open(name.c_str(), O_CREAT, 0644, count)) == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
}

int main(){
	Sem_Open(mutex, "/mutex", 1);
	std::thread producer(parse);
	std::thread consumer(convert);
	producer.join();
	consumer.join();
	std::cout << "Threads Joined" << std::endl;
	std::cout << "item_count: " << item_count << std::endl;
	return 0;
}


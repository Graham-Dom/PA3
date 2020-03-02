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
				//std::cout << "Read " << line << std::endl;

				sem_wait(spaces_avl);
				
				sem_wait(mutex);
				/* ------ CS ------- */
				buffer[in] = line;
				item_count++;
				in = (in + 1) % BUFF_SIZE;
				/* ----- END ------- */
				
				sem_post(mutex);
				

				sem_post(spaces_fld);
			}
		}

		file.close();
	}

	producer_complete = true;
}

void convert() {
	int out = 0;
	std::string to_convert;
	char converted[30];
	while (!producer_complete || item_count)
	{	
		sem_wait(spaces_fld);
			
		sem_wait(mutex);
		/* ------ CS ------- */
		to_convert = buffer[out];
		item_count--;
		out = (out + 1) % BUFF_SIZE;
		/* ----- END ------- */
		sem_post(mutex);

		sem_post(spaces_avl);

		//std::cout << "write " << to_convert << std::endl;
			
		

		if (dnslookup(to_convert.c_str(), converted, 30) == -1) {
			std::cerr << "Unable to resolve domain name " << to_convert << std::endl;
			std::cout << to_convert << ", " << std::endl;
		} else 
			std::cout << to_convert << ", " << std::string(converted) << std::endl;

	}
	//std::cout << "consumer complete" << std::endl;

}

void Sem_Open(sem_t* &sem, std::string name, int num){
	if ((sem = sem_open(name.c_str(), O_CREAT | O_EXCL, 0644, num)) == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
}

int main(){

	sem_unlink("/mutex");
	sem_unlink("/avail");
	sem_unlink("/filled");

	Sem_Open(mutex, "/mutex", 1);
	Sem_Open(spaces_avl, "/avail", BUFF_SIZE);
	Sem_Open(spaces_fld, "/filled", 0);

	std::thread producer(parse);
	std::thread consumer(convert);
	producer.join();
	consumer.join();
	std::cout << "Threads Joined" << std::endl;
	std::cout << "item_count: " << item_count << std::endl;

	sem_unlink("/mutex");
	sem_unlink("/avail");
	sem_unlink("/filled");
	return 0;
}


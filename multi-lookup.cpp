#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <thread>
#include <semaphore.h>
#include <stdatomic.h>
#include <vector>
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
sem_t *outfile_mutex;


void parse(int fileno) {
	static int in = 0;
	static int complete = 0;
	std::ifstream file("input/names" + std::to_string(fileno) + ".txt");
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
	complete++;
	if (complete == 5)
		producer_complete = true;
	file.close();
}


void convert() {
	static int out = 0;
	std::string to_convert;
	char converted[30];
	while (!producer_complete || item_count)
	{	
		sem_wait(spaces_fld);
			
		sem_wait(mutex);
		/* ------ CS ------- */
		to_convert = buffer[out];
		item_count--;
		//std::cout << "out: " << out << std::endl;
		out = (out + 1) % BUFF_SIZE;
		/* ----- END ------- */
		sem_post(mutex);

		sem_post(spaces_avl);
			
		
		sem_wait(outfile_mutex);
		if (dnslookup(to_convert.c_str(), converted, 30) == -1) {
			std::cerr << "Unable to resolve domain name " << to_convert << std::endl;
			std::cout << to_convert << ", " << std::endl;
		} else 
			std::cout << to_convert << ", " << std::string(converted) << std::endl;

		sem_post(outfile_mutex);	
	}
	//std::cout << "consumer complete" << std::endl;

}

void Sem_Open(sem_t* &sem, std::string name, int num){
	if ((sem = sem_open(name.c_str(), O_CREAT, 0644, num)) == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
}

int main(){
	Sem_Open(mutex, "/mutex", 1);
	Sem_Open(outfile_mutex, "/outfile", 1);
	Sem_Open(spaces_avl, "/avail", BUFF_SIZE);
	Sem_Open(spaces_fld, "/filled", 0);

	std::vector<std::thread> producers;
	for (int i = 1; i <= 5; ++i) {
		producers.push_back(std::thread(parse, i));
	}

	std::vector<std::thread> consumers;
	for (int i = 1; i <= 5; ++i) {
		consumers.push_back(std::thread(convert));
	}
	
	for (int i = 0; i < 5; ++i){
		producers[i].join();
	}

	for (int i = 0; i < 5; ++i){
		consumers[i].join();
	}

	std::cout << "Threads Joined" << std::endl;
	std::cout << "item_count: " << item_count << std::endl;

	sem_unlink("/mutex");
	sem_unlink("/avail");
	sem_unlink("/filled");
	sem_unlink("/outfile");
	return 0;
}


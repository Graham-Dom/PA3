#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <mutex>
#include <thread>
#include <stdatomic.h>
#include "util.h"
using namespace std;

#define BUFF_SIZE 10
size_t str_size = 30;
string buffer[BUFF_SIZE];

std::atomic<bool> producer_complete(false);
std::mutex mtx;

bool activeParser = false;


static int item_count = 0;


void parse() {
	int i = 0;
	int in = 0;
	for (int n = 1; n < 6; n++) {
		ifstream file("input/names" + to_string(n) + ".txt");
		string line;

		
		if (!file) {
			cerr << "File Error" << endl; 
			exit(1);
		}
		while (getline(file, line)) {

			if (line != "\n") {
				// cout << line << endl;
				while (item_count == BUFF_SIZE);
				
				mtx.lock();
				/* ------ CS ------- */
				buffer[in] = line;
				item_count++;
				/* ----- END ------- */
				
				mtx.unlock();
				in = (in + 1) % BUFF_SIZE;
			}
		}
		producer_complete = true;

		file.close();
	}
}

void convert() {
	int out = 0;
	ofstream file("outfile.txt");
	if (file){
		string to_convert;
		char converted[30];
		while (!producer_complete || item_count > 0)
		{	
			while(item_count == 0);
			
			mtx.lock();
			/* ------ CS ------- */
			to_convert = buffer[out];
			item_count--;
			/* ----- END ------- */
			mtx.unlock();

			//cout << to_convert << endl;
			
			out = (out + 1) % BUFF_SIZE;

			if (dnslookup(to_convert.c_str(), converted, 30) == -1) {
				cerr << "Unable to resolve domain name " << to_convert << endl;
				file << to_convert << ", " << endl;
			} else 
				file << to_convert << ", " << string(converted) << endl;

		}
		file.close();
	}

}

int main(){
	thread producer(parse);
	thread consumer(convert);
	producer.join();
	consumer.join();
	cout << "Threads Joined" << endl;
	cout << "item_count: " << item_count << endl;
	return 0;
}


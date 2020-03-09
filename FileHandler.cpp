#include "FileHandler.h"

using namespace std;

FileHandler::FileHandler(vector<ifstream*> &infiles): files(infiles), current_file(0){
	if ((mtx = sem_open("/FileHandler", O_CREAT, 0644, 1)) == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
}

FileHandler::~FileHandler(){
	sem_unlink("/FileHandler");
}

string FileHandler::getLine(void) {
	sem_wait(mtx);
	string line;
	while ((size_t) current_file < files.size()) {
		
		if (getline(*(files[current_file]), line)){
			if (line != "\n") {
				sem_post(mtx);
				return line;
			}
		}

		else
			current_file++;
	}

	sem_post(mtx);
	return "";
	current_file++;
}

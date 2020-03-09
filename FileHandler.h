#include <vector>
#include <fstream>
#include <string>
#include <semaphore.h>
#include <fcntl.h>

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

class FileHandler {
	std::vector<std::ifstream *> files;
	sem_t *mtx;
	int current_file;

public:
	FileHandler(std::vector<std::ifstream*> &infiles);
	~FileHandler();

	// Used to get the next IP address from a handler's set of files
	std::string getLine(void);
};

#endif
#include <iostream>
#include <sstream>
#include <vector>
#include "filesystem.h"

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 15;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
	"quit", "format", "ls", "create", "cat", "createImage", "restoreImage",
	"rm", "cp", "append", "mv", "mkdir", "cd", "pwd", "help"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
std::string help();

int main(void) {

	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "user@DV1492";    // Change this if you want another user to be displayed
	std::string currentDir = "/";    // current directory, used for output
	FileSystem FS = FileSystem();
	string newName = "";
	string oldFileName = "";
	vector<string> lsFS;
	string textString = "";
	int returnResult = -1;

	bool bRun = true;

	do {
		std::cout << user << ":" << currentDir << "$ ";
		getline(std::cin, userCommand);

		int nrOfCommands = parseCommandString(userCommand, commandArr);
		if (nrOfCommands > 0) {

			int cIndex = findCommand(commandArr[0]);
			switch (cIndex) {

			case 0: // quit
				bRun = false;
				std::cout << "Exiting\n";
				break;
			case 1: // format
				FS.format();

				currentDir = FS.pwd();
				break;
			case 2:
				/* ls - List directory and files */
				returnResult = -1;
				newName = commandArr[1];

				returnResult = FS.listAll(newName);

				if (returnResult == -1){
					cout << "Error listing directory." << endl;
				}

				break;
			case 3:
				/* create - creates a file */
				returnResult = -1;
				newName = commandArr[1];

				returnResult = FS.createFile(newName);

				if (returnResult == -1){
					cout << "Error creating file." << endl;
				}

				break;
			case 4: // cat

				newName = commandArr[1];

				textString = FS.readFromFile(newName);

				if (textString == ""){
					cout << "Error: No data returned" << endl;
				}
				else{
					cout << textString << endl;
				}

				break;
			case 5: // createImage
				// commandArr[1] should be a valid filepath.

				FS.createImage(commandArr[1]);

				break;
			case 6: // restoreImage
				//commandArr[1] should be a valid filepath.

				FS.restoreImage(commandArr[1]);

				currentDir = FS.pwd();
				break;
			case 7:
				/* rm - remove file */
				returnResult = -1;
				newName = commandArr[1];

				returnResult = FS.removeFile(newName);

				if (returnResult == -1){
					cout << "Error removing file." << endl;
				}

				break;
			case 8:
				/* cp - copy file */
				returnResult = -1;

				returnResult = FS.copyFile(commandArr[1], commandArr[2]);

				if (returnResult == -1){
					cout << "Error copying file" << endl;
				}

				break;
			case 9: // append   -- Do not include
				break;
			case 10:
				/* mv - renames oldfilename to newFilename */
				returnResult = -1;
				oldFileName = commandArr[1];
				newName = commandArr[2];

				returnResult = FS.renameAFile(oldFileName, newName);

				if (returnResult == -1){
					cout << "Error renaming file" << endl;
				}
				else if (returnResult == 0){
					cout << "File not found" << endl;
				}

				break;
			case 11:
				/* mkdir - Makes a directory */
				returnResult = -1;

				returnResult = FS.createFolder(commandArr[1]);

				if (returnResult == -1){
					cout << "Error creating directory." << endl;
				}

				break;
			case 12:
				/* cd - change directory */
				returnResult = -1;

				returnResult = FS.goToFolder(commandArr[1]);

				if (returnResult == -1){
					cout << "Error changing directory." << endl;
				}
				else{
					currentDir = FS.pwd();
				}

				break;
			case 13:
				/* pwd - shows current path to working directory */
				cout << FS.pwd() << endl;
				break;
			case 14:
				/* help - prints out help text */
				std::cout << help() << std::endl;
				break;
			default:
				std::cout << "Unknown command: " << commandArr[0] << std::endl;
			}
		}

		// Clears commandArr, newName, oldfileName
		for (int i = 0; i < 8; i++)
		{
			commandArr[i] = "";
		}
		newName = "";
		oldFileName = "";
		lsFS.clear();

	} while (bRun == true);

	return 0;
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
	std::stringstream ssin(userCommand);
	int counter = 0;
	while (ssin.good() && counter < MAXCOMMANDS) {
		ssin >> strArr[counter];
		counter++;
	}
	if (strArr[0] == "") {
		counter = 0;
	}
	return counter;
}
int findCommand(std::string &command) {
	int index = -1;
	for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
		if (command == availableCommands[i]) {
			index = i;
		}
	}
	return index;
}

std::string help() {
	std::string helpStr;
	helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
	helpStr += "-----------------------------------------------------------------------------------\n";
	helpStr += "* quit:                             Quit OSD Disk Tool\n";
	helpStr += "* format;                           Formats disk\n";
	helpStr += "* ls     <path>:                    Lists contents of <path>.\n";
	helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
	helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
	helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
	helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
	helpStr += "* rm     <file>:                    Removes <file>\n";
	helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
	helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
	helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
	helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";
	helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";
	helpStr += "* pwd:                              Get current working directory\n";
	helpStr += "* help:                             Prints this help screen\n";
	return helpStr;
}

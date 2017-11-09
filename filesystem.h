#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memblockdevice.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib>

using namespace std;

class FileSystem
{
private:

  class FSFile {
  public:
    string fileName;
    string fileData;
    int blockIdNr;

    FSFile(){
      this->fileName = "X";
      this->fileData = "X";
      this->blockIdNr = -1;
    }

    FSFile(string fileName, int blockIdNr){
        this->fileName = fileName;
        this->blockIdNr = blockIdNr;
    }

    FSFile(const FSFile &other){
      this->fileName = other.fileName;
      this->fileData = other.fileData;
      this->blockIdNr = -1;
    }

    FSFile& operator=(const FSFile &other){
      this->fileName = other.fileName;
      this->fileData = other.fileData;
      this->blockIdNr = -1;

      return *this;
    }
    ~FSFile() {
    }
  };

  class FSNode {
  public:
    vector<FSNode *> FSDirectory;
    vector<FSFile *> FSFiles;
    string directoryName;

    int makeDirectory(FSNode *rootDir, FSNode *parentDir,  string directoryName){
      FSNode *newDir = new FSNode(rootDir, parentDir, directoryName);

      this->FSDirectory.push_back(newDir);

      return 1;
    }

    FSNode(){
      this->directoryName = "root";
      this->FSDirectory.push_back(this);
      this->FSDirectory.push_back(this);
    }

    FSNode(FSNode *rootDir, FSNode *parentDir, string directoryName){
      this->FSDirectory.push_back(rootDir);
      this->FSDirectory.push_back(parentDir);
      this->directoryName = directoryName;
    }

    ~FSNode() {

		// Start at 2. 0 = root, 1 = parent.
		for (int i = 2; i < FSDirectory.size(); i++){
			FSDirectory[i]->~FSNode();
			delete FSDirectory[i];

		}

		for (int i = 0; i < FSFiles.size(); i++){
			delete FSFiles[i];
		}

		FSDirectory.clear();	
    }

  };

  MemBlockDevice mMemblockDevice;
  FSNode* rootDir;
  FSNode* currentDir;
  bool freeBlocksArray[250];

  void removeCharFromString(string &str, char* charsToRemove);
  vector<string> traversePath(string pathName);
  int setWorkDirectory(string fileName);
  vector<string> pathToRoot(FSNode* cwd, vector<string> path);
  string createImageTraverse(FSNode* dir);

public:
    FileSystem();
    ~FileSystem();


    /* These commands needs to implemented
     *
     * However, you are free to change the parameterlist and specify your own returntype for each function below.
     */

    /* This function formats the filesystem */
    void format();

    /* This function reads from a file in the filesystem and returns a string */
    string readFromFile(string fileName);

    /* This functions creates an image of the current filesystem */

    void createImage(string imageFilePath);

    /* This function restores the filesystem from an image file */

    void restoreImage(string imageFilePath);

    /* This function creates a file in the filesystem */
    int createFile(string fileName);

    /* Creates a folder in the filesystem */
    int createFolder(string dirName);

    /* Removes a file in the filesystem */
    int removeFile(string fileName);

    /* Function will move the current location to a specified location in the filesystem */
    int goToFolder(string folderName);

    /* This function will get all the files and folders in the specified folder */
    int listAll(string pathName);

    /* Renames a file */
    int renameAFile(string oldName, string newName);

	  /* copy - creates a copy of a file */
	  int copyFile(string fileOne, string fileTwo);

	  /* Retrive current working directory */
    string pwd();

    /* Add your own member-functions if needed */
};

#endif // FILESYSTEM_H

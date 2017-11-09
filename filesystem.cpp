#include "filesystem.h"
using namespace std;

FileSystem::FileSystem() {
  rootDir = new FSNode();
  currentDir = rootDir;
  for(int i = 0; i < 250; i++){
    freeBlocksArray[i] = false; // false free, true used
  }
}

FileSystem::~FileSystem() {
	
	rootDir->~FSNode();

	delete rootDir;
}

void FileSystem::removeCharFromString(string &str, char* charsToRemove){
  for(int i = 0; i < 1; i++){
    str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
  }
}

/* format . format the filesystem */

void FileSystem::format(){

  currentDir = nullptr;

  rootDir->~FSNode();

  delete rootDir;

  rootDir = new FSNode();
  currentDir = rootDir;

  for(int i = 0; i < 250; i++){
    freeBlocksArray[i] = false;
  }

}

// Splits recieved string on '/'. Returns vector with each piece of the split.
vector<string> FileSystem::traversePath(string pathName){

  vector<string> path;
  size_t pos = 0, found;
  while((found = pathName.find_first_of("/", pos)) != string::npos) {
    path.push_back(pathName.substr(pos, found - pos));
    pos = found+1;
  }
  path.push_back(pathName.substr(pos));

  return path;
}

// Tries to set currentDir to the directory recieved via pathName. Returns 0 on success, -1 on error.
int FileSystem::setWorkDirectory(string pathName){

  vector<string> tempVect;
  tempVect = traversePath(pathName);

  FSNode* traversingDir = currentDir;
  int i = 0;

  int valueToReturn = 0;
  int pathFound = 0;

  if(pathName.at(0) == '/'){
    traversingDir = rootDir;
    i = 1;
    if(pathName.size() == 1){
      pathFound = 1;
    }
  }
  else if(tempVect[0] == "."){
    i = 1;
  }
  else if(tempVect[0] == ".."){
    if(traversingDir != rootDir){
      for(int j = 0; j < tempVect.size() && tempVect[j] == ".."; j++){
        if(tempVect[j] == ".."){
          if(traversingDir == rootDir){
            valueToReturn = -1;
            j = tempVect.size();
          }
          else{
            traversingDir = traversingDir->FSDirectory[1];
            i = j+1;
          }
        }
      }
    }
  }
  if(pathFound != 1){
    for(i; i < tempVect.size() && valueToReturn == 0; i++){
      pathFound = 0;

      if(i == tempVect.size()-1 && tempVect[i] == ""){
        pathFound = 1;
      }
      else{
        for(int k = 0; k < traversingDir->FSDirectory.size() && pathFound != 1; k++){
            if(tempVect[i] == traversingDir->FSDirectory[k]->directoryName){
              traversingDir = traversingDir->FSDirectory[k];
              pathFound = 1;
            }
        }
        if(pathFound != 1){
          valueToReturn = -1;
        }
      }
    }
  }

  if(valueToReturn != -1){
    currentDir = traversingDir;
  }

  return valueToReturn;
}

/* cat . read from file */
string FileSystem::readFromFile(string fileName){
  string dataToReturn = "";
  int valueToReturn = 0;
  FSNode* tempDir = currentDir;
  vector<string> tempVect;

  if(fileName != "")
  {
    tempVect = traversePath(fileName);

    fileName = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);
    }
  }

  if(fileName != "" && valueToReturn != -1){
    for(int i = 0; i < currentDir->FSFiles.size() && valueToReturn != 1; i++){
      if(fileName == currentDir->FSFiles[i]->fileName){

        dataToReturn = mMemblockDevice.readBlock(currentDir->FSFiles[i]->blockIdNr).toString();

        removeCharFromString(dataToReturn, "*");
        valueToReturn = 1;
      }
    }
  }

  currentDir = tempDir;
  return dataToReturn;
}

string FileSystem::createImageTraverse(FSNode* dir){

  string dataToReturn = "";

  dataToReturn += ("\nD\n" + dir->directoryName);

  for(int i = 0; i < dir->FSFiles.size(); i++){

    dataToReturn += "\nF\n";
    dataToReturn += to_string(dir->FSFiles[i]->blockIdNr) + "\n";
    dataToReturn += dir->FSFiles[i]->fileName;
  }

  for(int i = 2; i < dir->FSDirectory.size(); i++){
    dataToReturn += createImageTraverse(dir->FSDirectory[i]);
  }
  dataToReturn += "\nENDOFDIR";

  return dataToReturn;

}

/* createImage - creates an image of the current filesystem */
void FileSystem::createImage(string imageFilePath){
  if(imageFilePath != ""){

    ofstream out;
    string dataToStore = "";

    for(int i = 0; i < 250; i ++){
      if (freeBlocksArray[i] == true){
        dataToStore += to_string(i);
        dataToStore += "\n";
        dataToStore += mMemblockDevice.readBlock(i).toString() += "\n";
      }
    }
    dataToStore += "!Endofdata!\n";


    // Traverse through tree
    string tempData = createImageTraverse(this->rootDir);

    if(tempData != ""){
      dataToStore += tempData;
    }

    // Write to the file
    out.open(imageFilePath);

    if(out.is_open()){

      out << dataToStore;

      out.close();
    }
    else{
      // Error opening file
    }

  }
}

/* restoreImage - loads a filesystem from an image */

void FileSystem::restoreImage(string imageFilePath){
  if(imageFilePath != ""){

    FSNode* walker = nullptr;
    string dataStream = "";

    ifstream in;
    in.open(imageFilePath);

    if(in.is_open()){
      this->format();

      walker = rootDir;

      while(dataStream != "!Endofdata!" && !in.eof()){
        getline(in, dataStream);

        if(dataStream != "!Endofdata!"){
          int blockPosition = 0;
          blockPosition = atoi(dataStream.c_str());

          string dataToWrite = "";
          getline(in, dataStream);
          dataToWrite = dataStream;

          mMemblockDevice.writeBlock(blockPosition, dataToWrite);

          this->freeBlocksArray[blockPosition] = true;
        }
      }

      while(!in.eof()){
        getline(in, dataStream);

        if(dataStream == "D"){
          getline(in, dataStream);

          if(dataStream != "root"){
            walker->makeDirectory(*&rootDir, *&walker, dataStream);

            walker = walker->FSDirectory[walker->FSDirectory.size()-1];

          }
        }
        else if(dataStream == "F"){
          getline(in, dataStream);

          int blockToWrite = 0;
          blockToWrite = atoi(dataStream.c_str());

          getline(in, dataStream);

          FSFile *newFile = new FSFile(dataStream, blockToWrite);

          walker->FSFiles.push_back(newFile);
        }
        else if(dataStream == "ENDOFDIR"){
          if(walker->directoryName != "root"){
            walker = walker->FSDirectory[1];
          }
        }
      }
      in.close();
    }
  }
}

/* create - creates a file */
int FileSystem::createFile(string fileName){

  FSNode* tempDir = currentDir;
  vector<string> tempVect;
  string dataToWrite = "";
	int blockId = -1;
	int writeOk = 1;

  if(fileName != "")
  {
    tempVect = traversePath(fileName);

    fileName = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      writeOk = setWorkDirectory(tempStr);
    }
  }

  if(fileName != "" && writeOk != -1){
    writeOk = 1;
  	for (int i = 0; i < currentDir->FSFiles.size() && writeOk == 1; i++){
  		if (currentDir->FSFiles[i]->fileName == fileName){
  			writeOk = -1;
  		}
  	}
    for (int i = 0; i < currentDir->FSDirectory.size() && writeOk == 1; i++){
  		if (currentDir->FSDirectory[i]->directoryName == fileName){
  			writeOk = -1;
  		}
  	}
  }
  else{
    writeOk = -1;
  }

	for (int i = 0; i < 250 && blockId == -1; i++){
		if (freeBlocksArray[i] == false){
			blockId = i;
		}
	}

	if (writeOk != -1 && blockId != -1){


		getline(cin, dataToWrite);
		cin.clear();

		if (dataToWrite.size() <= 512 && !(dataToWrite.size() > 512)){
			string junkString((512 - dataToWrite.size()), '*');

			dataToWrite += junkString;

			mMemblockDevice.writeBlock(blockId, dataToWrite);

			freeBlocksArray[blockId] = true;

			FSFile *newFile = new FSFile(fileName, blockId);

			currentDir->FSFiles.push_back(newFile);

      currentDir = tempDir;
			return 1;
		}
		else{
      currentDir = tempDir;
			return -1;
		}
	}
	else{
    currentDir = tempDir;
		return -1;
	}
}

/* mkdir - Makes a directory */
int FileSystem::createFolder(string dirName){
  int valueToReturn = 1;
  FSNode* tempDir = currentDir;
  vector<string> tempVect;

  if(dirName != "")
  {
    tempVect = traversePath(dirName);

    dirName = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);
    }
  }

  if(dirName != "" && valueToReturn != -1){
    valueToReturn = 1;
    for(int i = 0; i < currentDir->FSDirectory.size() && valueToReturn == 1; i++){
      if(currentDir->FSDirectory[i]->directoryName == dirName){
        valueToReturn = -1;
      }
    }
    for(int i = 0; i < currentDir->FSFiles.size() && valueToReturn == 1; i++){
      if(currentDir->FSFiles[i]->fileName == dirName){
        valueToReturn = -1;
      }
    }
  }
  else{
    valueToReturn = -1;
  }

  if(valueToReturn == 1){
    currentDir->makeDirectory(*&rootDir, *&currentDir, dirName);
  }

  currentDir = tempDir;

  return valueToReturn;
}

/* rm - remove file */
int FileSystem::removeFile(string fileName){

  int valueToReturn = 0;

  FSNode* tempDir = currentDir;
  vector<string> tempVect;

  if(fileName != "")
  {
    tempVect = traversePath(fileName);

    fileName = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);
    }
  }

  if(fileName != "" && valueToReturn != -1){
    valueToReturn = -1;
    for(int i = 0; i < currentDir->FSFiles.size() && valueToReturn == -1; i++){
      if(fileName == currentDir->FSFiles[i]->fileName)
      {
        freeBlocksArray[currentDir->FSFiles[i]->blockIdNr] = false;
		delete currentDir->FSFiles[i];
		currentDir->FSFiles.erase(currentDir->FSFiles.begin()+i);

        valueToReturn = 1;
      }
    }
  }
  if(fileName == ""){
    valueToReturn = -1;
  }

  currentDir = tempDir;
  return valueToReturn;
}

/* mv - renames oldfilename to newFilename */
int FileSystem::renameAFile(string oldName, string newName){

  int valueToReturn = 0;
  FSNode* tempDir = currentDir;
  vector<string> tempVect;

  if(oldName != "" && newName != ""){

    tempVect = traversePath(oldName);

    oldName = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);
    }
  }
  else{
    valueToReturn = -1;
  }


  for(int k = 0; k < currentDir->FSFiles.size() && valueToReturn != -1; k++){
    if(newName == currentDir->FSFiles[k]->fileName){
      valueToReturn = -1;
    }
  }
  for (int k = 0; k < currentDir->FSDirectory.size() && valueToReturn != -1; k++){
	  if (newName == currentDir->FSDirectory[k]->directoryName){
		  valueToReturn = -1;
	  }
  }

  for(int i = 0; i < currentDir->FSFiles.size() && valueToReturn == 0; i++)
  {
    if(oldName == currentDir->FSFiles[i]->fileName)
    {
      valueToReturn = 1;

      currentDir->FSFiles[i]->fileName = newName;
    }
  }

  currentDir = tempDir;

  return valueToReturn;
}

/* cd - change directory */
int FileSystem::goToFolder(string pathName){

  return setWorkDirectory(pathName);
}

/* ls - List directory and files */
int FileSystem::listAll(string pathName) {

    FSNode* tempDir = currentDir;
    int valueToReturn = 0;

    if(pathName != ""){
      valueToReturn = setWorkDirectory(pathName);
    }

    if(valueToReturn != -1){
      vector<string> itemList;

      if(currentDir->FSDirectory.size() > 0){
        for(int i = 2; i < currentDir->FSDirectory.size(); i++)
        {
          itemList.push_back(currentDir->FSDirectory[i]->directoryName);
        }
      }

      if(currentDir->FSFiles.size() > 0){
        for(int i = 0; i < currentDir->FSFiles.size(); i++)
        {
          itemList.push_back(currentDir->FSFiles[i]->fileName);
        }
      }

  	for (int i = 0; i < itemList.size(); i++)
  	{
  		cout << itemList[i] << endl;
  	}

    currentDir = tempDir;
  }

  return valueToReturn;
}

/* copy - creates a copy of a file */
int FileSystem::copyFile(string fileOne, string fileTwo) {
	FSFile* file1 = nullptr;
	FSFile* file2 = nullptr;
	bool okToCopy = false;
	int blockId = -1;
	string dataToCopy = "";
	int valueToReturn = -1;
	bool exists = false;
  FSNode* tempDir = currentDir;
  vector<string> tempVect;

  if(fileOne != "" && fileTwo != ""){

    tempVect = traversePath(fileOne);

    fileOne = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);

      if(valueToReturn == -1){
        exists = true;
      }

    }
  }
  else{
    exists = true;
  }

	if (exists == false){
		for (int i = 0; i < currentDir->FSFiles.size() && okToCopy == false; i++) {
			if (currentDir->FSFiles.at(i)->fileName == fileOne)
			{
				file1 = currentDir->FSFiles.at(i);

				dataToCopy = mMemblockDevice.readBlock(currentDir->FSFiles[i]->blockIdNr).toString();

				okToCopy = true;
			}
		}
	}

	for (int i = 0; i < 250 && blockId == -1; i++){
		if (freeBlocksArray[i] == false){

			blockId = i;
		}
	}

	if (okToCopy == true && blockId != -1){

    currentDir = tempDir;

    tempVect = traversePath(fileTwo);

    fileTwo = tempVect[tempVect.size()-1];

    if(tempVect.size() > 1 && tempVect[tempVect.size()-1] != ""){

      string tempStr = "";

      for(int i = 0; i < tempVect.size()-1; i++)
      {
        tempStr += tempVect[i] + '/';
      }

      valueToReturn = setWorkDirectory(tempStr);

      if(valueToReturn == -1){
        exists = true;
      }
    }

    for (int i = 0; i < currentDir->FSFiles.size() && exists != true; i++){
  		if (currentDir->FSFiles.at(i)->fileName == fileTwo){
  			exists = true;
  		}
  	}
	for (int i = 0; i < currentDir->FSDirectory.size() && exists != true; i++){
		if (currentDir->FSDirectory.at(i)->directoryName == fileTwo){
			exists = true;
		}
	}


    if(exists != true){
  		mMemblockDevice.writeBlock(blockId, dataToCopy);
  		freeBlocksArray[blockId] = true;

  		file2 = new FSFile(fileTwo, blockId);
  		currentDir->FSFiles.push_back(file2);

  		valueToReturn = 1;
    }
    else{
      valueToReturn = -1;
    }
	}
  else{
    valueToReturn = -1;
  }

  currentDir = tempDir;
  return valueToReturn;
}

/* pwd - shows current path to working directory */
string FileSystem::pwd() {
	vector<string> ToRoot;
	vector<string> path;
  string toReturn = "";

	ToRoot = pathToRoot(currentDir, path);
	reverse(ToRoot.begin(), ToRoot.end());
	for (int i = 0; i < ToRoot.size(); i++)
	{
		toReturn += ToRoot[i];
	}

  return toReturn;
}

/* Recursive function to move to root */
vector<string> FileSystem::pathToRoot(FSNode* cwd, vector<string> path) {
	if (cwd->directoryName == "root")
	{
		path.push_back("/");
		return path;
	}
	else {
		path.push_back((cwd->directoryName + "/"));
		return pathToRoot(cwd->FSDirectory.at(1), path);
	}

}

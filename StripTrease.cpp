/*
Compile with:
g++ -Wall StripTrease.cpp -m64 -O3 -o StripTrease

or with Makefile:
make
*/

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
//#include <regex>	// requires -std=gnu++11
#include <stdlib.h> // needed on linux

using namespace std;

void printProgramInfo();
void printProgramUsage ();
bool checkValidInputFile (string fileName);
bool checkValidOutputFile (string outputFileName);
void processTrees (string & fileName, string & outName, bool & newick, bool & stripAll);
void processCommandLineArguments(int argc, char *argv[], string & fileName, string & outName, 
	bool & newick, bool & stripAll);
string parseString (string & stringToParse, int stringPosition);
vector <string> tokenizeString (string & stringToParse);
string stripTree (string & tree, bool & newick, bool & stripAll);
string removeAnnotations (string & tree, bool & stripAll);
void keepSupportValue (string & annotation, int & start, vector <int> & charsToRemove);

// version information
double version = 0.1;
string month = "November";
int year = 2013;

int main (int argc, char *argv[]) {
	printProgramInfo();
	string fileName;
	string outName = "";
	bool newick = true;
	bool stripAll = false;
	
	processCommandLineArguments(argc, argv, fileName, outName, newick, stripAll);
	processTrees(fileName, outName, newick, stripAll);
	
	cout << endl << "Fin." << endl;
	return 0;
}

void printProgramInfo() {
	cout << endl << 
	"************************************************" << endl <<
	"             StripTrease version "   << version    << endl <<
	"                Joseph W. Brown"                  << endl <<
	"             University of Michigan"              << endl <<
	"         Complaints: josephwb@umich.edu"          << endl <<
	"                 " << month <<", " << year <<        endl << 
	"************************************************" << endl << endl;
}

void processCommandLineArguments(int argc, char *argv[], string & fileName, string & outName, 
	bool & newick, bool & stripAll)
{
	if (argc == 1) {
		cout << "Enter the name of the tree file to be processed: ";
		cin >> fileName;
	} else {
		for (int i = 1; i < argc; i++) {
			string temp = argv[i];
			
			if (temp == "-h" || temp == "-help") {
				cout
				<< "Program description: Strips annotations from tree strings." << endl
				<< endl
				<< "To compile, type the following in a unix prompt:" << endl << endl
				<< "g++ -Wall StripTrease.cpp -m64 -O3 -o StripTrease" << endl
				<< endl
				<< "To run, type:" << endl;
				printProgramUsage ();
				exit(0);  
			} else if (temp == "-in") {
				i++;
				checkValidInputFile(argv[i]);
				fileName = argv[i];
				continue;
			} else if (temp == "-out") {
				i++;
				checkValidOutputFile(argv[i]);
				outName = argv[i];
				continue;
			} else if (temp == "-nexus") {
				newick = false;
				continue;
			} else if (temp == "-all") {
				stripAll = true;
				continue;
			} else {
				cout
				<< "Unknown command-line argument '" << argv[i] << "' encountered." << endl
				<< endl
				<< "Usage:" << endl;
				printProgramUsage ();
				exit(0);
			}
			cout << endl;
		}
	}
	if (outName == "") {
		outName = "Stripped-" + fileName;
		checkValidOutputFile(outName);
	}
}

void printProgramUsage () {
	cout << "./StripTrease [-in treefile] [-out outname] [-nexus] [-all] [-h]" << endl
	<< endl
	<< "where:" << endl
	<< endl
	<< "  'treefile' contains tree(s) to be processed." << endl
	<< "  'outname' the file to write stripped trees to." << endl
	<< "  '-nexus' specifies that output trees will be in Nexus format (default = newick)." << endl
	<< "  '-all' remove all annotations (default = preserve node support values)." << endl
	<< "  '-h' prints this help" << endl << endl;
}

bool checkValidInputFile (string fileName) {
	bool validInput = false;
	ifstream tempStream;
	
	tempStream.open(fileName.c_str());
	if (tempStream.fail()) {
		ofstream errorReport("Error.StripTrease.txt");
		errorReport << "StripTrease analysis failed." << endl << "Error: unable to open file '";
		errorReport << fileName << "'" << endl;
		errorReport.close();
		
		cerr << endl << "StripTrease analysis failed. " << endl << "Error: unable to open file '";
		cerr << fileName << "'" <<  endl;
		exit(1);
	} else {
		cout << "Successfully opened file '" << fileName << "'." <<  endl << endl;
		validInput = true;
		tempStream.close();
		tempStream.clear();
	}
	return validInput;
}

bool checkValidOutputFile (string outputFileName) {
	bool testOutBool = true;
	bool fileNameAcceptable = false;
	bool keepFileName = false;
	
// First, check if file already exists, so overwriting can be prevented
	fstream testIn;
	while (!fileNameAcceptable) {
		testIn.open(outputFileName.c_str());
		if (!testIn) {
			testIn.close();
			fileNameAcceptable = true;
		} else {
			testIn.close();
			cout << endl << "Default output file '" << outputFileName << "' exists!  Change name (0) or overwrite (1)? ";
			cin >> keepFileName;
			if (!keepFileName) {
				cout << "Enter new output file name: ";
				cin >> outputFileName;
			} else {
				cout << "Overwriting existing file '" << outputFileName << "'." << endl;
				fileNameAcceptable = true;
			}
		}
	}
	
	ofstream outFile;
	outFile.open(outputFileName.c_str());
	
	if (outFile.fail()) {
		ofstream errorReport("Error.StripTrease.txt");
		errorReport << "StripTrease analysis failed." << endl << "Error: unable to open file '";
		errorReport << outputFileName << "'" << endl;
		errorReport.close();

		cerr << endl << "StripTrease analysis failed." << endl << "Error: unable to open file '";
		cerr << outputFileName << "'" <<  endl;
		testOutBool = false;
		exit(1);
	} else {
		outFile.close();
		outFile.clear();
	}
	return testOutBool;
}

void processTrees (string & fileName, string & outName, bool & newick, bool & stripAll) {
	ifstream treeInput;
	ofstream strippedTrees;
	int numTrees = 0;
	string line;
	
	treeInput.open(fileName.c_str());
	strippedTrees.open(outName.c_str());
	
	while (getline(treeInput, line)) {
		if (line.empty()) {
			continue;
		}
		// process character-by-character
		if (parseString(line, 0) == "tree") {
		//	cout << "Found a tree!" << endl;
			strippedTrees << stripTree(line, newick, stripAll) << endl;
			numTrees++;
		} else {
			if (!newick) { // print out non-tree lines to nexus file.
				strippedTrees << line << endl;
			}
		}
	}
	treeInput.close();
	strippedTrees.close();
	
	if (numTrees == 0) {
		cout << endl << "No trees found." << endl;
	} else if (numTrees == 1) {
		cout << endl << "Processed " << numTrees << " tree." << endl;
	} else {
		cout << endl << "Processed " << numTrees << " trees." << endl;
	}
}


/* expecting format (5 elements):
	tree treename = [&rooting] treestring;
could be a simple newick, though...
Assume if > 1 element, Nexus. Otherwise, newick.
*/
string stripTree (string & tree, bool & newick, bool & stripAll) {
	string res;
	int numElements = 0;
	
	vector <string> tokenizedString = tokenizeString(tree);
	numElements = tokenizedString.size();
	//cout << "Treestring contains "  << numElements << " elements." << endl;
	res = tree;
	
	if (newick) {
		if (numElements == 5) { // tree treename = [&rooting] treestring;
			res = removeAnnotations(tokenizedString[4], stripAll);
		} else if (numElements == 4) { // tree treename = treestring;
			res = removeAnnotations(tokenizedString[3], stripAll);
		} else if (numElements == 1) {
			res = removeAnnotations(tokenizedString[0], stripAll);
		} else {
			cout << "Ack! Don't know how to deal with " << numElements << " elements." << endl;
		}
	} else {
		if (numElements == 5) {
			res = tokenizedString[0] + " " + tokenizedString[1] + " " + tokenizedString[2] + 
				" " + tokenizedString[3] + " " + removeAnnotations(tokenizedString[4], stripAll);
		} else if (numElements == 4) {
			res = tokenizedString[0] + " " + tokenizedString[1] + " " + tokenizedString[2] + 
				" " + removeAnnotations(tokenizedString[3], stripAll);
		} else {
			cout << "Ack! Don't know how to deal with " << numElements << " tree elements!" << endl;
		}
	}
	//cout << "Cleaned tree: " << endl << res << endl;
	return res;
}


string removeAnnotations (string & tree, bool & stripAll) {
	string cleaned = tree;
	int numChars = cleaned.size();
	bool going = false;
	vector <int> charsToRemove;
	
	int start = 0;
	string temp;
	
	//cout << "tree: " << tree << endl;
	
// not very elegant...
	if (stripAll) {
		for (int i = 0; i < (numChars - 1); i++) {
			if (going) {
				if (cleaned[i] == ']') {
					going = false;
					charsToRemove.push_back(i);
					temp = cleaned.substr(start, (i - start + 1));
			//		cout << "Annotation: " << temp << endl;
					continue;
				} else {
					charsToRemove.push_back(i);
				}
			} else {
				if (cleaned[i] == '[') {
					going = true;
					charsToRemove.push_back(i);
					start = i;
				}
			}
		}
		reverse(charsToRemove.begin(), charsToRemove.end());
		for (int i = 0; i < (int)charsToRemove.size(); i++) {
			cleaned.erase(cleaned.begin()+charsToRemove[i]);
		}
	} else {
		for (int i = 0; i < (numChars - 1); i++) {
			if (going) {
				if (cleaned[i] == ']') {
					going = false;
					temp = cleaned.substr(start, (i - start + 1));
			//		cout << "Annotation: " << temp << "; start currently = " << start << endl;
					keepSupportValue(temp, start, charsToRemove);
					continue;
				}
			} else {
				if (cleaned[i] == '[') {
					going = true;
					start = i;
				}
			}
		}
		reverse(charsToRemove.begin(), charsToRemove.end());
		for (int i = 0; i < (int)charsToRemove.size(); i++) {
			cleaned.erase(cleaned.begin()+charsToRemove[i]);
		}
	}
	return cleaned;
}

// take in a bracketed annotation e.g. [&height_95%_HPD={0.19508017007579698,0.35535530647436653},...,posterior=1.0,...]
// add to charsToRemove vector all characters that are not the nodal support value.
// this may be specified as 'posterior' (BEAST), 'label' (tree processed in FigTree), probably other junk.
// support value could be an integer or a float
void keepSupportValue (string & annotation, int & start, vector <int> & charsToRemove) {
	int supStop = 0;
	int supStart = (int)annotation.size() - 1;
	string temp;
	size_t found = annotation.find("posterior");
	if (found != string::npos) {
		supStart = (int)found + 10; // 10 is 'posterior='
		found = annotation.find_first_not_of("0123456789.", supStart);
		supStop = (int)found - 1;
	} else {
		found = annotation.find("label");
		if (found != string::npos) {
			supStart = (int)found + 6; // 6 is 'label='
			found = annotation.find_first_not_of("0123456789.", supStart);
			supStop = (int)found - 1;
		}
	}
	// add characters to be removed to vector
	for (int i = 0; i < (int)annotation.size(); i++) {
		if (i < supStart || i > supStop) {
			charsToRemove.push_back(i + start);
		}
	}
}

string parseString (string & stringToParse, int stringPosition) {
	vector <string> tempVector = tokenizeString(stringToParse);
	string res = tempVector[stringPosition];
	return res;
}

vector <string> tokenizeString (string & stringToParse) {
	vector <string> tempVector;
	istringstream tempStream(stringToParse);
	string tempString;
	while (tempStream >> tempString) {
		tempVector.push_back(tempString);
	}
	return tempVector;
}

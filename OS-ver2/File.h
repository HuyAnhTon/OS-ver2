#pragma once
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;



class File {

private:
	bool _isFolder;
	string _name;
	float _size;
	int _first_cluster;
	vector<File*> _files;
public:

	bool isFolder() { return _isFolder; }
	string name() { return _name; }
	int first_cluster() {return _first_cluster; }
	float size() { return _size; }

	File(bool isFolder, string name, float size, int first_cluster) {

		_isFolder = isFolder;
		_name = name;
		_size = size;
		_first_cluster = first_cluster;
	}

	void add(File* file) {
		_files.push_back(file);
	}

	string toString() {

		stringstream builder;

		string type;
		if (_isFolder) {
			type = "Folder";
		}
		else
			type = "File";

		builder << "[" << type << "] " << _name << "(size: " << _size <<", first cluster: " << _first_cluster << ")";
		return builder.str();

	}

	void setName(string val) {
		_name = val;
	}

	void setFirstCluster(int val) {
		_first_cluster = val;
	}

	void printTree() {
		
		for (int i = 0; i < _files.size(); i++) {

			if (_files[i]->isFolder()) {
				cout << _files[i]->name() << endl;
				_files[i]->printTree();
			}
			else {
				cout << _files[i]->name() << endl;
			}
		}
	}
};
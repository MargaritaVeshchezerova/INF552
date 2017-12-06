#include "stdafx.h"
#include "DatabaseUpload.h"

vector<string> getNames(DIR* dir)
{
	vector<string> names;
	struct dirent* ent = NULL;

	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
		{
			names.push_back(ent->d_name);
			cout << endl << ent->d_name;
		}
	}
	return names;
}

void loadDatabase(char* directory, vector<Mat>& images)
{
	DIR* rep = NULL;
	rep = opendir(directory);

	if (rep == NULL)
		exit(1);


	cout << "The folder was successfully opened" << endl;

	vector<string> names = getNames(rep);

	for (int i = 0; i < names.size(); i++)
	{
		cout << string(directory) + '\\' + names[i] << endl;
		images.push_back(imread(string(directory) + '\\' + names[i]));
	}

	if (closedir(rep) == -1)
		exit(-1);

	cout << "The folder was successfully closed" << endl;
}
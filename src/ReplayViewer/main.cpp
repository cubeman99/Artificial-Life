#include "ReplayViewer.h"
#include <iostream>

using namespace std;


int main(int argc, char** argv)
{
	const char* title = "Artificial Life Replay Viewer";
	int width  = 1100;
	int height = 800;

	ReplayViewer application;
	if (!application.Initialize(title, width, height))
		return 1;

	application.Run();

	return 0;
}

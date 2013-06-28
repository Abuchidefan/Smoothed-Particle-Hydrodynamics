#include "owWorldSimulation.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if(argc == 1)
		run( argc, argv);
	else{
		for(int i = 1; i<argc; i++)
			if(strncmp(argv[i], "-no_g", 5) == 0)//run without graphics
				run( argc, argv, false);
	}
	return 0;
}

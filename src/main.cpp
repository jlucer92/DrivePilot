#include <stdio.h>
#include "DrivePilot.h"

int main( int argc, const char* argv[] )
{
	cout<<"DrivePilot Starting..."<<endl;
	DrivePilot drivePilot;
	drivePilot.init();
	while(1){
		drivePilot.parseData();
	}
}
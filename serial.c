#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

HANDLE hComm;

int COMConnect(const char * port_name, uint32_t speed);
int COMDisconnect();
int COMConfig(uint32_t speed);

int main(int argc, char **argv)
{
	// Make everything print instantly
	setbuf(stdout, NULL);

	// Check for enough parameters
	if(argc < 3)
	{
		printf("Need to pass more parameters!\n");
		return 0;
	}

	// Convert the speed to a number
	uint32_t speed = (uint32_t)atoi(argv[2]);

	// Print the parameter
	printf("Connecting to port %s with speed %u\n", argv[1], speed);

	if(COMConnect(argv[1], speed) != 0)
	{
		printf("Connection failed\n");
		return -1;
	}

	printf("Connected\n");

	char TempChar;
	DWORD NoBytesRead;

	while(1)
	{
		// Get a character
		while(NoBytesRead == 0)
		{
			ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
		}

		NoBytesRead = 0;

		// Print it
		printf("%c", TempChar);
	}

	// Be well behaved, take no prisoners
	COMDisconnect();

	return 0;
}

int COMConnect(const char * port_name, uint32_t speed)
{

	char str[100];// = "\\\\.\\COM8";

	strcat(str, "\\\\.\\");
	strcat(str, port_name);

	// Attempt to connect
	hComm = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	// Check for connection
	if(hComm == INVALID_HANDLE_VALUE) 
	{
		printf("Connection failed\n");
		return -1;
	}

	if(COMConfig(speed) != 0)
	{
		printf("Error configuration\n");
		return -1;
	}

	return 0;
}

int COMDisconnect()
{
	CloseHandle(hComm);

	return 0;
}

int COMConfig(uint32_t speed)
{
	int state = 0;

	DCB dcbSerialParams = {0}; // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	// Get the connection parameters
	if(GetCommState(hComm, &dcbSerialParams) == FALSE)
	{
		printf("Error getting CommState\n");
		return -1;
	}

	// Set the 
	switch(speed)
	{
		case 115200:
			dcbSerialParams.BaudRate = CBR_115200;
		break;

		default:
			printf("Error non valid speed\n");
			return -1;
		break;
	}

	dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
	dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None

	// Set the connection parameters
	if(SetCommState(hComm, &dcbSerialParams) == FALSE)
	{
		printf("Error setting CommState\n");
		return -1;
	}

		// Set the timeouts
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50; // in milliseconds
	timeouts.ReadTotalTimeoutConstant    = 50; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier  = 10; // in milliseconds
	timeouts.WriteTotalTimeoutConstant   = 50; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

	// Set the timeouts
	if(SetCommTimeouts(hComm, &timeouts) == FALSE)
	{
		printf("Error setting timeouts\n");
		return -1;
	}

	// Set the timeouts
	if(SetCommMask(hComm, EV_RXCHAR) == FALSE)
	{
		printf("Error setting CommMask\n");
		return -1;
	}

	// Set the timeouts
	DWORD dwEventMask; 
	if(WaitCommEvent(hComm, &dwEventMask, NULL) == FALSE)
	{
		printf("Error setting CommEvent\n");
		return -1;
	}

	return state;
}
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include "packet.h"
#include "serial.h"
#include "serialize.h"
#include "constants.h"

#define PORT_NAME			"/dev/ttyACM0"
#define BAUD_RATE			B57600

int exitFlag=0;
sem_t _xmitSema;
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace cv;
//using namespace std;

void colour_detection()
{
    //Mat image=imread("output.png",1); //1 because it's coloured
     
    //Capture the image from the webcam
    Mat image; //cap >> image;
    VideoCapture cap(0); cap >> image;

    if(image.empty())
        printf("Something is wrong with the picam.\n");
        
    //namedWindow("image", CV_WINDOW_FREERATIO);
    //imshow("image",image); //display the original image
    //waitKey(1);

    //Converting image from BGR to HSV color space.
    Mat OutputImageRed;
    cvtColor(image, OutputImageRed, COLOR_BGR2HSV);

    Mat maskred1, maskred2;
    // Creating masks to detect the upper and lower red
    inRange(OutputImageRed, Scalar(140, 100, 70), Scalar(180, 255, 255), maskred1);
    inRange(OutputImageRed, Scalar(0, 100, 70), Scalar(20, 255, 255), maskred2);

    //Generating the final mask
    maskred1 = maskred1+maskred2;

    //Now for green 
    Mat OutputImageGreen;
    cvtColor(image, OutputImageGreen, COLOR_BGR2HSV);

    Mat maskgreen1, maskgreen2;
    // Creating masks to detect the upper and lower red
    //inRange(OutputImageGreen, Scalar(40, 100, 70), Scalar(80, 255, 255), maskgreen1);
    inRange(OutputImageGreen, Scalar(40, 100, 70), Scalar(90, 255, 255), maskgreen1);

   //inRange(OutputImageBlue, Scalar(170, 120, 70), Scalar(100, 255, 255), maskred2);
    
    //Generating the final mask
   // maskgreen1 = maskgreen1+maskgreen2;

    int red_count,green_count;
    red_count=countNonZero(maskred1);
    green_count=countNonZero(maskgreen1);

    if (red_count > green_count)
        printf("The color is green\n");
    else if (red_count < green_count)
        printf("The color is green\n");
    else
        printf("No green or red detected\n");

    printf("%d %d \n", red_count, green_count);
}
    
void handleError(TResult error)
{
	switch(error)
	{
		case PACKET_BAD:
			printf("ERROR: Bad Magic Number\n");
			break;

		case PACKET_CHECKSUM_BAD:
			printf("ERROR: Bad checksum\n");
			break;

		default:
			printf("ERROR: UNKNOWN ERROR\n");
	}
}
void handleStop(TPacket *packet){
	printf("------OBSTACLE------\n");
	switch (packet->data[0])
	{
		case 'l':
			printf("Too near to left\n");
			break;
		case 'r':
			printf("Too near to right\n");
			break;
		case 'f':
			printf("Too near to front\n");
			break;
	
		default:
			break;
	}
}
void handleStatus(TPacket *packet)
{
	printf("\n ------- ALEX STATUS REPORT ------- \n\n");
	printf("Left Forward Ticks:\t\t%d\n", packet->params[0]);
	printf("Right Forward Ticks:\t\t%d\n", packet->params[1]);
	printf("Left Reverse Ticks:\t\t%d\n", packet->params[2]);
	printf("Right Reverse Ticks:\t\t%d\n", packet->params[3]);
	printf("Left Forward Ticks Turns:\t%d\n", packet->params[4]);
	printf("Right Forward Ticks Turns:\t%d\n", packet->params[5]);
	printf("Left Reverse Ticks Turns:\t%d\n", packet->params[6]);
	printf("Right Reverse Ticks Turns:\t%d\n", packet->params[7]);
	printf("Forward Distance:\t\t%d\n", packet->params[8]);
	printf("Reverse Distance:\t\t%d\n", packet->params[9]);
	printf("\n---------------------------------------\n\n");
}

void handleResponse(TPacket *packet)
{
	// The response code is stored in command
	switch(packet->command)
	{
		case RESP_OK:
			printf("Command OK\n");
		break;

		case RESP_STATUS:
			handleStatus(packet);
		break;

		case RESP_STOP:
			handleStop(packet);
		break;

		default:
			printf("Alex is confused.\n");
	}
}

void handleErrorResponse(TPacket *packet)
{
	// The error code is returned in command
	switch(packet->command)
	{
		case RESP_BAD_PACKET:
			printf("Arduino received bad magic number\n");
		break;

		case RESP_BAD_CHECKSUM:
			printf("Arduino received bad checksum\n");
		break;

		case RESP_BAD_COMMAND:
			printf("Arduino received bad command\n");
		break;

		case RESP_BAD_RESPONSE:
			printf("Arduino received unexpected response\n");
		break;

		default:
			printf("Arduino reports a weird error\n");
	}
}

void handleMessage(TPacket *packet)
{
	printf("Message from Alex: %s\n", packet->data);
}

void handlePacket(TPacket *packet)
{
	switch(packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
				// Only we send command packets, so ignore
			break;

		case PACKET_TYPE_RESPONSE:
				handleResponse(packet);
			break;

		case PACKET_TYPE_ERROR:
				handleErrorResponse(packet);
			break;

		case PACKET_TYPE_MESSAGE:
				handleMessage(packet);
			break;
	}
}

void sendPacket(TPacket *packet)
{
	char buffer[PACKET_SIZE];
	int len = serialize(buffer, packet, sizeof(TPacket));

	serialWrite(buffer, len);
}

void *receiveThread(void *p)
{
	char buffer[PACKET_SIZE];
	int len;
	TPacket packet;
	TResult result;
	int counter=0;

	while(1)
	{
		len = serialRead(buffer);
		counter+=len;
		if(len > 0)
		{
			result = deserialize(buffer, len, &packet);

			if(result == PACKET_OK)
			{
				counter=0;
				handlePacket(&packet);
			}
			else 
				if(result != PACKET_INCOMPLETE)
				{
					printf("PACKET ERROR\n");
					handleError(result);
				}
		}
	}
}

void flushInput()
{
	char c;

	while((c = getchar()) != '\n' && c != EOF);
}
int dis = 0;
int sp = 0;
int ang = 0;
void setParams()
{
	printf("Enter distance, angle in cm/degrees (e.g. 50) and power in %% (e.g. 75) separated by space.\n");
	printf("E.g. 50 60 75 means go at 50 cm or 60 degrees left or right turn at 75%%\n");
	scanf("%d %d %d", &dis , &ang , &sp);
	flushInput();
}
void getParams(TPacket *commandPacket)
{
	//printf("Enter distance/angle in cm/degrees (e.g. 50) and power in %% (e.g. 75) separated by space.\n");
	//printf("E.g. 50 75 means go at 50 cm at 75%% power for forward/backward, or 50 degrees left or right turn at 75%%  power\n");
	//scanf("%d %d", &commandPacket->params[0], &commandPacket->params[1]);
	if (commandPacket->command ==  COMMAND_FORWARD || commandPacket->command ==  COMMAND_REVERSE) {
		commandPacket->params[0] = dis;
	}else{
		commandPacket->params[0] = ang;
	}

	commandPacket->params[1] = sp;
	
//	flushInput();
}

void sendCommand(char command)
{
	TPacket commandPacket;

	commandPacket.packetType = PACKET_TYPE_COMMAND;

	switch(command)
	{
		case 'w':
		case 'W':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_FORWARD;
			sendPacket(&commandPacket);
			break;

		case 's':
		case 'S':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_REVERSE;
			sendPacket(&commandPacket);
			break;

		case 'a':
		case 'A':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_TURN_LEFT;
			sendPacket(&commandPacket);
			break;

		case 'd':
		case 'D':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_TURN_RIGHT;
			sendPacket(&commandPacket);
			break;

		case 'o':
		case 'O':
			commandPacket.command = COMMAND_STOP;
			sendPacket(&commandPacket);
			break;

		case 'c':
		case 'C':
			commandPacket.command = COMMAND_CLEAR_STATS;
			commandPacket.params[0] = 0;
			sendPacket(&commandPacket);
			break;

		case 'g':
		case 'G':
			commandPacket.command = COMMAND_GET_STATS;
			sendPacket(&commandPacket);
			break;

		case 'q':
		case 'Q':
			exitFlag=1;
			break;
		
		case 'p':
		case 'P':
			setParams();
            break;

        case 'v':
		case 'V':
			colour_detection();
            break;
		default:
			printf("Bad command\n");

	}
}

int main()
{
	// Connect to the Arduino
	startSerial(PORT_NAME, BAUD_RATE, 8, 'N', 1, 5);

	// Sleep for two seconds
	printf("WAITING TWO SECONDS FOR ARDUINO TO REBOOT\n");
	sleep(2);
	printf("DONE\n");

	// Spawn receiver thread
	pthread_t recv;

	pthread_create(&recv, NULL, receiveThread, NULL);

	// Send a hello packet
	TPacket helloPacket;

	helloPacket.packetType = PACKET_TYPE_HELLO;
	sendPacket(&helloPacket);

	while(!exitFlag)
	{
		char ch;
		printf("Command (p=set param,w=forward, s=reverse, a=turn left, d=turn right, o=stop, c=clear stats, g=get stats, v=view colour ,q=exit)\n");
		scanf("%c", &ch);

		// Purge extraneous characters from input stream
		flushInput();

		sendCommand(ch);
	}

	printf("Closing connection to Arduino.\n");
	endSerial();
}

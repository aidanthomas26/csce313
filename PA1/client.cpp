/*
	Author of the starter code
    Yifan Ren
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 9/15/2024
	
	Please include your Name, UIN, and the date below
	Name: Aidan Thomas
	UIN: 332003177
	Date: 9/17/2024
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>

using namespace std;
using namespace std::chrono;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	string filename = "";
	int buffer = MAX_MESSAGE;
	bool new_chan = false;
	vector <FIFORequestChannel*> channels;

	//Add other arguments here
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				buffer = atoi (optarg);
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}

	//Task 1:
	//Run the server process as a child of the client process
	//give arguments for the server
	//server needs './server', '-m', '<val for -m arg>', 'NULL'
	//fork
	//in the child, run execvp using the server arguments.
	pid_t pid = fork();
	if (pid == 0)
	{
		const char* args[] = {"./server", "-m", to_string(buffer).c_str(), NULL};
		execvp(args[0], (char* const*)args);
	}

    FIFORequestChannel* control_chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(control_chan);

	if(new_chan)
	{
		//send hcannel request to server
		MESSAGE_TYPE nc = NEWCHANNEL_MSG;
   		control_chan->cwrite(&nc, sizeof(MESSAGE_TYPE));
		//create variable to hold name
		//cread the response from the server
		//call the fifo request channel constructor with the name from the server
		//push the new channel into the vector
		char new_channel_name[100];
		control_chan->cread(new_channel_name, sizeof(new_channel_name));
		std::cout << "New channel created: " << new_channel_name << endl;

		FIFORequestChannel* new_channel = new FIFORequestChannel(new_channel_name, FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(new_channel);
	}


	//Task 4:
	//Request a new channel
	FIFORequestChannel chan = *(channels.back());
	
	//Task 2:
	//Request data points
	//single data point when p,t,e are all != -1
	if (p != -1 && t != -1 && e != -1)
	{
    	char buf[MAX_MESSAGE];
    	datamsg x(p, t, e); //change from hardcoding to user values
	
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); //question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		std::cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}

	else if (p != -1)
	{
		ofstream outfile("received/x1.csv");

		for (int i=0; i<1000; ++i)
		{
			double time = i * 0.004;

			datamsg x1(p, time, 1);
			chan.cwrite(&x1, sizeof(datamsg));
			double ecg1;
			chan.cread(&ecg1, sizeof(double));

			datamsg x2(p, time, 2);
			chan.cwrite(&x2, sizeof(datamsg));
			double ecg2;
			chan.cread(&ecg2, sizeof(double));

			outfile << time << "," << ecg1 << "," << ecg2 << endl;
		}

		outfile.close();
		std::cout << "First 1000 ECG data points for person " << p << " have been saved to 'receieved/x1.csv'." << endl;
	}


	//else if p != -1 then request next 1000 data points
	//loop over first 1000 lines
	//send request for ecg 1
	//send request for ecg 2
	//write line to recieved/x1.csv
	auto start = high_resolution_clock::now();
	//Task 3:
	//Request files
	filemsg fm(0, 0);
	string fname = filename; //hardcoded
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // Request file length

	__int64_t file_length;
	chan.cread(&file_length, sizeof(__int64_t));  // Read file length from server
	std::cout << "The length of " << fname << " is " << file_length << " bytes." << endl;

	char* buf3 = new char[buffer];  // Buffer for receiving file data
	string output_file = "received/" + fname;  // Output file path
	ofstream ofs(output_file, ios::binary);

	__int64_t offset = 0;
	while (offset < file_length)
	{
		filemsg fm(offset, min((__int64_t)buffer, file_length - offset));  // Set offset and length
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());

		chan.cwrite(buf2, len);  // Send file request for the current chunk
		chan.cread(buf3, fm.length);  // Read the file data

		ofs.write(buf3, fm.length);  // Write to output file

		offset += fm.length;  // Update the offset
	}

	// End timing
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop - start);

	// Output the time taken
	std::cout << "Time taken for file transfer: " << duration.count() << " ms" << std::endl;

	//send the request (buf2)
	//get file length
	//receive the response
	//cread into buf3 length file_req -> len
	//write buf3 into file: recieved/filename


	delete[] buf2;
	delete[] buf3;
	//if necesary, close and delete the new channel

	ofs.close();
	std::cout << "File" << filename << " has been received and saved to " << output_file << endl;
	//Task 5:
	// Closing all the channels
    if(new_chan)
	{
		for (FIFORequestChannel* ch : channels)
		{
			MESSAGE_TYPE m = QUIT_MSG;
			ch->cwrite(&m, sizeof(MESSAGE_TYPE));  // Send QUIT_MSG to close each channel
			delete ch;  // Properly delete each channel
		}
	}
	else
	{
		// Even if new channels weren't created, close and delete control channel
		MESSAGE_TYPE m = QUIT_MSG;
		control_chan->cwrite(&m, sizeof(MESSAGE_TYPE));  // Send QUIT_MSG to control channel
		delete control_chan;  // Properly delete control channel
	}


	wait(NULL);

	return 0;
}

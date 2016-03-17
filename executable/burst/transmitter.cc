//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file   main.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Implementation of simple transmitter 
 *
 */
#include "utilities.h"
/**
 * Main: the transmitter
**/
int main(int argc, char* argv[]) {
	// CHECK parameters
  cout << "Ciao" << endl;
	if (argc < 7) {
		cerr << "Usage:" <<endl;
    cerr << "./mycc ID ID_RECEIVER ip port period[ms] packet_in_a_burst" << endl;
		return -1;
	}
  // INITIALIZATION
  int ID = atoi(argv[1]);
	int RECEIVER = atoi(argv[2]);
	std::string ip = argv[3];
	std::string port = argv[4];
	double period = atof(argv[5]) * pow10(3);
	int packet_in_a_burst = atoi(argv[6]);
	if ( !packet_in_a_burst ) {
		cerr << "Error: packet_in_a_burst must be > 0" <<endl;
		return -1;
	}
	uint const message_length = 18;
  int backoff_period = 10 * pow10(6);
	int packet_counter = 0;
	std::string base_message = "CIAO CIAO";
  
  // SET LOG FILES
	std::stringstream transm_file;
	std::stringstream delay_file;
	std::ofstream transm_file_stats(0);
	std::ofstream delay_file_stats(0);
        transm_file << "./sender_log.out";
	delay_file << "./delay_log.out";
        transm_file_stats.open(transm_file.str().c_str(),std::ios_base::app);
	delay_file_stats.open(delay_file.str().c_str(),std::ios_base::app);
        transm_file_stats << "[" << getEpoch() << "]:: Beginning of )Experiment - Tranmsmissions log" << endl;
	delay_file_stats << "[" << getEpoch() << "]:: Beginning of Experiment - Delay log" << endl;

  // CONNECT to the modem
  MdriverS2C_EvoLogics* pmDriver1 = connectModem(ip, port, ID);
  MdriverS2C_EvoLogics* pmDriver = pmDriver1;
	cout << "ID " << ID << " socket: " << ip <<":" << port << endl;
  // START THE APPLICATION
	while (true) {
    // SEND a packet of the "burst" (in IM mode)
		std::string complete_message = create_message(base_message,packet_counter,message_length);

		transmit(pmDriver,RECEIVER, complete_message,NOACK);
		packet_counter ++;
		cout << "Inviato " << ID << " to " << RECEIVER << " " << complete_message << endl;
		transm_file_stats << "[" << getEpoch() << "]:: Send from " << ID << " to " << RECEIVER 
			       << " " << complete_message << "packet_counter = " << packet_counter << endl;
		usleep(period);
    // The last packet of the burst is sent in ack mode
		if ((packet_counter % packet_in_a_burst ) == (packet_in_a_burst - 1) ) {
			complete_message = create_message(base_message,packet_counter,message_length); 
			transmit(pmDriver,RECEIVER, complete_message,ACK);
			packet_counter ++;
			cout << "Inviato con ack " << ID << " to " << RECEIVER << " " << complete_message 
           << endl;
			transm_file_stats << "[" << getEpoch() << "]:: Send with ACK request from " << ID 
			          		   << " to " << RECEIVER << " packet_counter = "  << packet_counter 
                       << " message: " <<  complete_message << "packet_counter = ";
			usleep(period);
		}
    // Retrieve the propagation delay thanks to the ACK
		if ( !(packet_counter % packet_in_a_burst )) {
		  cout << "Backoff, pacchetto " << packet_counter << " period = " << backoff_period 
           << "[usec]" << endl;
			usleep(backoff_period);
			int delay = pmDriver->getDelay();
			cout << "Delay = " << delay << endl;
			delay_file_stats << "[" << getEpoch() << "]:: Delay = " << delay << endl;
		}
	}
	return 0;
}


//
// Copyright (c) 2016 Regents of the SIGNET lab, University of Padova.
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
 * @file   burst_transmitter.cc
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
	if (argc < 10) {
		cerr << "Usage:" <<endl;
    cerr << "./mycc ID ID_RECEIVER ip1 port1 ip2 port2 period[ms] packet_in_a_burst keep-online" << endl;
		return -1;
	}
  // INITIALIZATION
  int ID = atoi(argv[1]);
	int RECEIVER = atoi(argv[2]);
	std::string ip1 = argv[3];
	std::string port1 = argv[4];
	std::string ip2 = argv[5];
	std::string port2 = argv[6];
	double period = atof(argv[7]) * pow10(3);
	int packet_in_a_burst = atoi(argv[8]);
	bool k_o = bool(atoi(argv[9]));
	if ( !packet_in_a_burst ) {
		cerr << "Error: packet_in_a_burst must be > 0" <<endl;
		return -1;
	}
	uint const message_length = 18;
  int backoff_period = 10 * pow10(6);
	int packet_counter = 0;
	std::string base_message = "CIAO CIAO";
  bool set_id = true;

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
  MdriverS2C_EvoLogics* pmDriver1 = connectModem(ip1, port1, ID, set_id, "tx_socket1.log");

	usleep(400000);
	cout << "ID " << ID << " socket: " << ip1 <<":" << port1 << endl;
  MdriverS2C_EvoLogics* pmDriver2 = connectModem(ip2, port2, ID, set_id, "tx_socket2.log");
	usleep(400000);
	cout << "ID " << ID << " socket: " << ip2 <<":" << port2 << endl;
	sleep(1);
  // START THE APPLICATION
	while (true) {
    // SEND a packet of the "burst" (in IM mode)
		std::string complete_message = create_message(base_message,packet_counter,message_length);
		cout << "Inviato " << ID << " to " << RECEIVER << " " << complete_message << endl;
		if (packet_counter % 2) {
			k_o ? transmitBurst(pmDriver2,RECEIVER, complete_message) :
			      transmit(pmDriver2,RECEIVER, complete_message,ACK);
			usleep(50000);
			while(pmDriver2->getAckStatus() == ACK_PENDING) {
				usleep(500000);
				//cout <<"waiting ack";
				pmDriver2->updateStatus();
			}
			pmDriver2->getAckStatus() == ACK_CONFIRMED ? cout << "ACK confirmed!" << endl :  cout << "FAILED!" << endl;
		}
		else {
			k_o ? transmitBurst(pmDriver1,RECEIVER, complete_message) :
						transmit(pmDriver1,RECEIVER, complete_message,ACK);
			usleep(50000);
			while(pmDriver1->getAckStatus() == ACK_PENDING) {
				usleep(50000);
				//cout <<"waiting ack";
				pmDriver1->updateStatus();
			}
			pmDriver1->getAckStatus() == ACK_CONFIRMED ? cout << "ACK confirmed!" << endl :  cout << "FAILED!" << endl;
		}
		packet_counter ++;
		//cout << "Inviato " << ID << " to " << RECEIVER << " " << complete_message << endl;
		transm_file_stats << "[" << getEpoch() << "]:: Send from " << ID << " to " << RECEIVER 
			       << " " << complete_message << "packet_counter = " << packet_counter << endl;
		usleep(period);
    // The last packet of the burst is sent in ack mode
		if ((packet_counter % packet_in_a_burst ) == (packet_in_a_burst - 1) ) {
			complete_message = create_message(base_message,packet_counter,message_length); 
			k_o ? transmitBurst(pmDriver2,RECEIVER, complete_message) :
			      transmit(pmDriver2,RECEIVER, complete_message,ACK);
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
			int delay = pmDriver1->getDelay();
			cout << "Delay = " << delay << endl;
			delay_file_stats << "[" << getEpoch() << "]:: Delay = " << delay << endl;
		}
	}
	return 0;
}


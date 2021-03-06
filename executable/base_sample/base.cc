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
 * @file  base.cc
 * @author Filippo Campagnaro
 * @version 1.0.0
 *
 * \brief Basic example where a sender sends packets periodically and 
 * a received node receives them. 
 */

#include "utilities.h"

int main(int argc, char* argv[]) {
	cout << "Ciao" << endl;
	if (argc < 8) {
		cerr << "Usage:" <<endl;
                cerr << "./mycc ID ID_SENDER ID_RECEIVER ip port period[ms] packet_length[Bytes]" << endl;
		return -1;
	}
  int ID = atoi(argv[1]);
	int SENDER = atoi(argv[2]);
	int RECEIVER = atoi(argv[3]);
	std::string ip = argv[4];
	std::string port = argv[5];
	double period = atof(argv[6]) * pow10(3);
	int message_length = atoi(argv[7]);
	
  int backoff_period = 3 * pow10(6);

	cout << "ID " << ID << endl;
	MdriverS2C_EvoLogics* pmDriver = connectModem(ip, port, ID);
	int modemStatus_old = pmDriver->getStatus();
	int modemStatus = pmDriver->updateStatus();

	if (ID == SENDER) {
		int packet_counter = 1;
		std::string message = "CIAO CIAO";
		while (true) {
			std::string ss = create_message(message,packet_counter,message_length);
			pmDriver->updateTx(RECEIVER, ss);
			pmDriver->modemTx();
			cout << "Inviato " << ID << " to " << RECEIVER << " " << ss << endl;
			usleep(period);
			packet_counter ++;
		}
	}
	else {
		while (true) {
			if (modemStatus == RX_STATE_IDLE)
				cout << "Ricevuto " << ID << " " << pmDriver->getRxPayload() << endl;
			sleep(3);			
			modemStatus_old = pmDriver->getStatus();
			modemStatus = pmDriver->updateStatus();
		}
	}
	return 0;
}



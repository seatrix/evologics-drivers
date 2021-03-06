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
#include "uwenea_opt_driver.h"
/**
 * Main: the transmitter
**/
int main(int argc, char* argv[]) {
	// CHECK parameters
  cout << "Ciao" << endl;
	if (argc < 6) {
		cerr << "Usage:" <<endl;
   	cerr << "./mycc ID serial, baud period[ms] tot_pkts" << endl;
		return -1;
	}
  // INITIALIZATION
 	int ID = atoi(argv[1]);
	std::string serial = argv[2];
	int baud = atoi(argv[3]);
	double period = atof(argv[4]) * pow10(3);
	int tot_pkts = atoi(argv[5]);
	if ( !tot_pkts ) {
		cerr << "Error: tot_pkts must be > 0" <<endl;
		return -1;
	}
	Enea_opt_driver pmDriver(serial, baud);
	pmDriver.setID(ID);
  pmDriver.setPeriod(0.01);
	pmDriver.start();
 	int backoff_period = 10 * pow10(6);
	int packet_counter = 0;
	std::string complete_message = "CIAO CIAO";
  
  // SET LOG FILES
	std::stringstream transm_file;
	std::stringstream delay_file;
	std::ofstream transm_file_stats(0);
	std::ofstream delay_file_stats(0);
  transm_file << "./sender_log.out";
	delay_file << "./delay_log.out";
  transm_file_stats.open(transm_file.str().c_str(),std::ios_base::app);
	delay_file_stats.open(delay_file.str().c_str(),std::ios_base::app);
  transm_file_stats << "[" << pmDriver.getEpoch() << "]:: Beginning of )Experiment - Tranmsmissions log" << endl;
	delay_file_stats << "[" << pmDriver.getEpoch() << "]:: Beginning of Experiment - Delay log" << endl;

  // START THE APPLICATION
	while (packet_counter < tot_pkts) {
    // SEND a packet of the "burst" (in IM mode)
		pmDriver.updateTx(complete_message);
 		pmDriver.modemTx();
		packet_counter ++;
		cout << "Inviato from " << ID << " pachet num " << packet_counter 
         << " payload "  << complete_message << endl;
		usleep(period);
	}
	return 0;
}


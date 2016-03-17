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
 * @file mdriverS2C_EvoLogics.cc
 * @author Riccardo Masiero and Federico Favaro
 * \version 1.0.0
 * \brief Implementation of the MdriverS2C_EvoLogics class.
 */


#include "mdriverS2C_EvoLogics.h"

static void hexdump(std::string name, std::string str) {
    int len = str.size();
    const char *data = str.c_str();

    std::cout << name << "[" << len << "]: " << std::hex;
    for (int i = 0; i < len; i++) {
        std::cout.fill('0');
        std::cout.width(2);
        std::cout << std::right << (unsigned int) (unsigned char) data[i];

        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            std::cout << "(" << data[i] << ")";
        std::cout << " ";
    }

    std::cout.width(0);
    std::cout << std::dec << std::endl;
}

static std::string hexdumplog(std::string str) {
    int len = str.size();
    const char *data = str.c_str();
    std::string str_out = "";

    for (int i = 0; i < len; i++) {
        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            str_out += data[i];
        else {
            std::string str;
            std::stringstream sstr("");
            sstr << "[" << std::hex << (unsigned int) (unsigned char) data[i] << std::dec << "]";
            sstr >> str;
            str_out += str;
        }
    }
    return str_out;
}

MdriverS2C_EvoLogics::MdriverS2C_EvoLogics(std::string path) : UWMdriver(), mInterpreter(this), mConnector(this, path) {
    m_status_tx = TX_STATE_IDLE;
    m_status_rx = RX_STATE_IDLE;
    setConnections(&mInterpreter, &mConnector);
}

MdriverS2C_EvoLogics::~MdriverS2C_EvoLogics() {
}

void MdriverS2C_EvoLogics::start() {
    printOnLog(LOG_LEVEL_DEBUG,"MS2C_EVOLOGICSDRIVER","MODEM_START");
    mConnector.openConnection();
    if(getResetModemQueue())
    {
	printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER","RESET_MODEM_QUEUE");
	status = MODEM_RESET;
	m_status_tx = TX_STATE_DROPBUFFER;
	modemTxManager();
    } else if (getKeepOnlineMode()) {
        printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER","SETTING_KEEP_ONLINE_MODALITY");
        status = MODEM_CFG;
        m_status_tx = TX_STATE_SET_KO;
        modemTxManager();
    } else if (SetModemID) {
	printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER","SETTING_MODEM_ID");
        status = MODEM_CFG;
        m_status_tx = TX_STATE_SET_ID;
        modemTxManager();
    } else {
        status = MODEM_IDLE;
    }
}

void MdriverS2C_EvoLogics::emptyModemQueue() {
    status = MODEM_RESET;
    m_status_tx = TX_STATE_DROPBUFFER;
    modemTxManager();
}

void MdriverS2C_EvoLogics::stop() {
    if (getKeepOnlineMode())
    {
        m_status_tx = TX_STATE_SEND_CLOSE;
        status = MODEM_QUIT;
        modemTxManager();
    } else {
        mConnector.closeConnection();
    }
}

void MdriverS2C_EvoLogics::modemTx() {
    status = MODEM_TX;
    m_status_tx = TX_STATE_SEND_IM;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemTxACK() {
    status = MODEM_TX;
    m_status_tx = TX_STATE_SEND_IM_ACK;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemTxBurst() {
    status = MODEM_TX;
    m_status_tx = TX_STATE_SEND_BURST;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemTxPBM() {
    status = MODEM_TX;
    m_status_tx = TX_STATE_SEND_PBM;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemSetID() {
    modemTxManager();
}

modem_state_t MdriverS2C_EvoLogics::updateStatus() {
  
    std::stringstream sstr("");
    string strlog;
    sstr << "UPDATE_STATUS::STATUS_" << status << "_M_STATUS_TX_" << m_status_tx << "_M_STATUS_RX_" << m_status_rx;
    sstr >> strlog;
    printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog);
    
    std::string rx_msg;
    bool cread = true;

    while (cread) {
        rx_msg = mConnector.readFromModem();

        if (rx_msg != "") {

            std::string pr_msg;
            std::string parser("\r\n");
            size_t p_offset = 0;
            size_t p_parser = rx_msg.find(parser);

            while (p_parser != std::string::npos) {
                pr_msg = rx_msg.substr(p_offset, p_parser + parser.size() - p_offset);
		
		if ( (pr_msg.find("RECVIM") != string::npos) || (pr_msg.find("RECV") != string::npos) )
		{
		  if (!getResetModemQueue())
		  {
		    queue_rx.push(pr_msg);
		  }
		}
		if ((pr_msg.find("OK") != string::npos) || (pr_msg.find("BUSY CLOSING CONNECTION") != string::npos) ||
		  (pr_msg.find("BUSY BACKOFF STATE") != string::npos) || (pr_msg.find("BUSY DELIVERING") != string::npos) ||
		  (pr_msg.find("FAILEDIM") != string::npos) || (pr_msg.find("FAILED") != string::npos) || (pr_msg.find("ERROR") != string::npos))
		{
		  queue_tx.push(pr_msg);
		}
                p_offset += pr_msg.size();
                p_parser = rx_msg.find(parser, p_offset + 1);
            } // End while (p_parser!=std::string::npos)

        } else {
            cread = false; // exit the loop
        } // End if (rx_msg!="")
    } // End of while (cread)
    cread = true;
    if (status == MODEM_TX || status == MODEM_CFG || status == MODEM_RESET || status == MODEM_QUIT) {// read from queue_tx only

        if (m_status_rx == RX_STATE_RX_IM && status == MODEM_TX) {
            // Update S2C RX status
            m_status_rx = RX_STATE_IDLE;
        }


        while (cread) {

            if (!queue_tx.empty()) {

                // Read the possible received message
                rx_msg = queue_tx.front();
                queue_tx.pop();

                if (rx_msg.find("OK") != string::npos) {
                    std::stringstream sstr("");
		    string strlog;
		    sstr << "UPDATE_STATUS::PROCESSING_OK::" << status << "_M_STATUS_TX_" << m_status_tx << "_M_STATUS_RX_" << m_status_rx;
		    sstr >> strlog;
		    printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog);
                    if (status == MODEM_RESET || status == MODEM_CFG || status == MODEM_QUIT)
		    {
		      
		      if (getKeepOnlineMode())
		      {
			status = MODEM_CFG;
			m_status_tx  = TX_STATE_SET_KO;
			cread = false;
		      }
		      else if(getModemID())
		      {
			status = MODEM_CFG;
			m_status_tx = TX_STATE_SET_ID;
			cread = false;
		      }
		      else
		      {
			status = MODEM_IDLE;
			m_status_tx = TX_STATE_IDLE;
			cread = false;
		      }
		      
		    }
		    else if (status == MODEM_TX)
		    {
		      status = MODEM_IDLE;
		      m_status_tx = TX_STATE_IDLE;
		      cread = false;
		    }
		    else
		    {

                        std::stringstream sstr("");
			string strlog;
			sstr << "UPDATE_STATUS::OK_WRONG_STATUS_" << m_status_tx << "_" << status;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog);
		      
		    }

                } else if (rx_msg.find("ERROR") != string::npos) {
		     if(status == MODEM_CFG || status == MODEM_TX || status == MODEM_RESET)
		     {
		       
			std::stringstream sstr("");
			string strlog;
			sstr << "UPDATE_STATUS::PROCESSING_ERROR_" << m_status_tx << "_" << status;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_ERROR,"MS2C_EVOLOGICSDRIVER",strlog);
                        status = MODEM_IDLE;
                    } else {
			std::stringstream sstr("");
			string strlog;
			sstr << "UPDATE_STATUS::ERROR_WRONG_STATUS_TX_" << m_status_tx << "_STATUS_" << status;
			sstr >> strlog;
			printOnLog(LOG_LEVEL_ERROR,"MS2C_EVOLOGICSDRIVER",strlog);
                    }
                    m_status_tx = TX_STATE_IDLE;
                    cread = false;

                } else if  (rx_msg.find("BUSY DELIVERING") != string::npos) {
		    printOnLog(LOG_LEVEL_ERROR,"MS2C_EVOLOGICSDRIVER","UPDATE_STATUS::BUSY_DELIVERING_MESSAGE");
                    if (m_status_tx == TX_STATE_SEND_PBM_DONE && status == MODEM_TX)
                    {
                        status = MODEM_IDLE;
                        m_status_tx = TX_STATE_IDLE;
                        cread = false;
                    }
                } else {

		  status = MODEM_IDLE;
		}
            } else {
                cread = false;
            }
        }
    } else if(status == MODEM_RX) {
      
      
        m_status_rx = RX_STATE_IDLE;
        status = MODEM_IDLE_RX;
        cread = false;

    }
    else {// read from queue_rx only

        while (cread) {

            if (!queue_rx.empty()) {

                // Read the possible received message
                rx_msg = queue_rx.front();
                queue_rx.pop();

                if (rx_msg.find("RECVIM") != string::npos) {
                    m_status_rx = RX_STATE_RX_IM;
                    status = MODEM_RX;
                    mInterpreter.parse_recvim(rx_msg);
                    cread = false;
                } else if (rx_msg.find("RECV") != string::npos) {
                    m_status_rx = RX_STATE_RX_BURST;
                    status = MODEM_RX;
                    mInterpreter.parse_recv(rx_msg);
                    cread = false;
                } else if (rx_msg.find("RECVPBM") != string::npos) {
                    m_status_rx = RX_STATE_RX_BURST;
                    status = MODEM_RX;
                    mInterpreter.parse_recvpbm(rx_msg);
                    cread = false;
                } else {// Any other AT messages (ignored for the moment)    
		    std::stringstream sstr("");
		    string strlog;
		    sstr << "UPDATE_STATUS::IGNORED_AT_MESSAGE_" <<rx_msg;
		    sstr >> strlog;
		    printOnLog(LOG_LEVEL_ERROR,"MS2C_EVOLOGICSDRIVER",strlog); 
                }
            } else {
                cread = false;
            } // End if (!queue_rx.empty())
        }// End while (cread)

    }// End if (status == _TX || status == _CFG)        

    return status;
}

int MdriverS2C_EvoLogics::getDelay() {
    std::string req_msg = mInterpreter.build_get_delay();
    mConnector.writeToModem(req_msg);
    std::string rx_msg;
    bool cread = true;
    string base = "0123456789";
    while (cread) {
        rx_msg = mConnector.readFromModem();
        cout << rx_msg ;
        int ret = atoi(rx_msg.c_str());
        if (!(ret == 0 && rx_msg.c_str()[0] != '0'))
            return ret;

    }
}

void MdriverS2C_EvoLogics::modemTxManager() {
    std::string tx_msg;
    std::stringstream sstr("");
    string strlog;
    switch (m_status_tx) {
      case TX_STATE_SET_ID:
	// Build the configuration message 
	  tx_msg = mInterpreter.build_setAL(ID);
	  sstr << "MODEM_TX_MANAGER::SETAL!"<< ID <<"_MESSAGE";
	  sstr >> strlog;
	  printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog);
        break;
      case TX_STATE_SEND_IM:
	// Build the instant message
	  tx_msg = mInterpreter.build_sendim(payload_tx.length(), dest, "noack", payload_tx);
	  sstr << "MODEM_TX_MANAGER::SENDIM = " << hexdumplog(tx_msg);
	  sstr >> strlog;
	  printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog); 
        break;
      case TX_STATE_SEND_IM_ACK:
    // Build the instant message
      tx_msg = mInterpreter.build_sendim(payload_tx.length(), dest, "ack", payload_tx);
      sstr << "MODEM_TX_MANAGER::SENDIM = " << hexdumplog(tx_msg);
      sstr >> strlog;
      printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog); 
        break;
      case TX_STATE_SEND_BURST:
	//build a burst data
	  tx_msg = mInterpreter.build_atsend(payload_tx.length(), dest, payload_tx);
	sstr << "MODEM_TX_MANAGER::SEND = " << hexdumplog(tx_msg);
	sstr >> strlog;
	printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog); 
        break;
      case TX_STATE_SEND_PBM:
	tx_msg = mInterpreter.build_atsendpbm(payload_tx.length(), dest, payload_tx);
	sstr << "MODEM_TX_MANAGER::SEND_PBM = " << hexdumplog(tx_msg);
	sstr >> strlog;
	printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog); 
	break;
      case TX_STATE_DROPBUFFER:
	tx_msg = mInterpreter.build_atzn(DROPBUFFER_TYPE);
	break;
      case TX_STATE_SET_KO:
	sstr << "MODEM_TX_MANAGER::SEND_AT!KO = " << hexdumplog(tx_msg);
	sstr >> strlog;
	printOnLog(LOG_LEVEL_INFO,"MS2C_EVOLOGICSDRIVER",strlog); 
	tx_msg = mInterpreter.build_atko(0);
	break;
      case TX_STATE_SEND_CLOSE:
        tx_msg = mInterpreter.build_ath(0);
	break;
      default:
	
	sstr << "MODEM_TX_MANAGER::UNEXPECTED_CALL_m_status_tx = " << m_status_tx;
	sstr >> strlog;
	printOnLog(LOG_LEVEL_ERROR,"MS2C_EVOLOGICSDRIVER",strlog); 
	return;
  }//end switch
  mConnector.writeToModem(tx_msg);
  m_status_tx = updateTxStatus(m_status_tx);
}

double MdriverS2C_EvoLogics::getIntegrity() {
    return mInterpreter.getIntegrity();
}

tx_states_t MdriverS2C_EvoLogics::updateTxStatus(tx_states_t state)
{
  tx_states_t new_state;
  switch(state)
  {
    case TX_STATE_SET_ID:
      new_state = TX_STATE_SET_ID_DONE;
      setModemID(false);
      break;
    case TX_STATE_DROPBUFFER:
      new_state = TX_STATE_DROPBUFFER_DONE;
      break;
    case TX_STATE_SEND_IM:
      new_state = TX_STATE_SEND_IM_DONE;
      break;
    case TX_STATE_SEND_IM_ACK:
      new_state = TX_STATE_SEND_IM_ACK_DONE;
      break;
    case TX_STATE_SET_KO:
      new_state = TX_STATE_SET_KO_DONE;
      setKeepOnlineMode(false);
      break;
    case TX_STATE_SEND_BURST:
      new_state = TX_STATE_SEND_BURST_DONE;
      break;
    case TX_STATE_SEND_PBM:
      new_state = TX_STATE_SEND_PBM_DONE;
      break;
    case TX_STATE_SEND_CLOSE:
      new_state = TX_STATE_SEND_CLOSE_DONE;
    default:
      new_state = state;
  }
  return new_state;
}



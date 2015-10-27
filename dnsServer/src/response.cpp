/* 
 * File:   response.cpp
 * Author: tomas
 * 
 * Created on 29 de junio de 2009, 4:45
 */
#include <iostream>
#include <sstream>

#include "logger.h"
#include "message.h"
#include "response.h"

using namespace std;
using namespace dns;

string Response::asString() const throw() {

    ostringstream text;
    text << endl << "RESPONSE { ";
    text << Message::asString();

    text << "\tname: " << m_name << endl;
    text << "\ttype: " << m_type << endl;
    text << "\tclass: " << m_class << endl;
    text << "\tttl: " << m_ttl << endl;
    text << "\trdLength: " << m_rdLength << endl;
    text << "\trdata: " << m_rdata << " }" << dec;

    return text.str();
}

void Response::decode(const char* buffer, int size) throw() {

    // Only needed for the DNS client
}

int Response::code(char* buffer) throw() {

    Logger& logger = Logger::instance();
    logger.trace("Response::code()");

    char* bufferBegin = buffer;

    code_hdr(buffer);
    buffer += HDR_OFFSET;

    // Code Question section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);

    // Code Answer section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);
    put32bits(buffer, m_ttl);
    if(m_type==DNS_PTR){
        put16bits(buffer, m_rdLength);
        code_domain(buffer, m_rdata);
    }
    if(m_type==DNS_A){
        put16bits(buffer, 4);
        uint32_t ipAddr = 0;
        ipAddr = code_ipAddress(m_rdata);
        cout << "code ip=" << hex << ipAddr << endl;
        put32bits(buffer, ipAddr);
    }
    
    int size = buffer - bufferBegin;
    log_buffer(bufferBegin, size);

    return size;
}

void Response::code_domain(char*& buffer, const std::string& domain) throw() {

    int start(0), end; // indexes

    while ((end = domain.find('.', start)) != string::npos) {

        *buffer++ = end - start; // label length octet
        for (int i=start; i<end; i++) {

            *buffer++ = domain[i]; // label octets
        }
        start = end + 1; // Skip '.'
    }

    *buffer++ = domain.size() - start; // last label length octet
    for (int i=start; i<domain.size(); i++) {

        *buffer++ = domain[i]; // last label octets
    }

    *buffer++ = 0;
}

uint32_t Response::code_ipAddress(const std::string& ipAddr) throw() {

    int start(0), end; // indexes

    uint32_t ip = 0;
    uint8_t byte = 0;
    string::const_iterator it = ipAddr.begin();
    while( it != ipAddr.end() )
    {
        if( *it == '.' )
        {
            ip = (ip << 8) + byte;
            ++it;
            byte = 0;
            continue;
        }

        byte = 10*byte + (*it - '0');

        ++it;
    }
    ip = (ip << 8) + byte;

    cout << "code_ipAddress " << ipAddr << " - " << hex << ip << endl;

    return ip;
}


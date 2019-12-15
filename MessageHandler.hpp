#include "RTSP.hpp"
#define CRLF "\r\n"

using namespace std;
#pragma once

class MessageHandler {
    protected:
        string data;        // 원본 요청
        // string message;     // 수정한 요청
    public:
        MessageHandler() : data("") {

        }
        ~MessageHandler() {
            
        }
        void inputData(const char *buffer) {
            data.append(buffer);
        }
        void inputData(string buffer) {
            data.append(buffer);
        }
        void setData(string input) {
            data = input;
        }
        string getData() {
            return data;
        }
        void printData() {
            cout << data << "\n";
        }
        virtual void parseData() = 0;
        // virtual void addHeader(string key, string value) = 0;
        // virtual const char* getRequestMessage() = 0;
        virtual string makeMessage() = 0;
};
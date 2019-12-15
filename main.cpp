#include <string>
#include <iostream>
#include <map>
#include "RequestHandler.hpp"
#include "ResponseHandler.hpp"
// #include <boost/asio.hpp>
#define CRLF "\r\n"
using namespace std;

string temp = "RTSP/1.0 200 OK\r\nCSeq: 2\r\nContent-Base: rtsp://example.com/media.mp4\r\nContent-Type: application/sdp\r\nContent-Length: 460\r\n\r\nm=video 0 RTP/AVP 96\r\na=control:streamid=0\r\na=range:npt=0-7.741000\r\na=length:npt=7.741000\r\na=rtpmap:96 MP4V-ES/5544\r\na=mimetype:string; video/MP4V-ES\r\na=AvgBitRate:integer;304018\r\n";

void split();

int main() {

    ResponseHandler *reqHandler = new ResponseHandler();
    reqHandler->setData(temp);
    cout << "----------------------\n";
    reqHandler->printData();
    reqHandler->parseData();
    cout << "----------------------\n";
    
    // reqHandler->setURL("test");
    // reqHandler->setStatus_coode("400");
    string message = reqHandler->makeMessage();
    cout << message << endl;
    // reqHandler->printMessage();
    cout << "----------------------\n";
    return 0;
}
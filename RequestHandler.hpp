#include <vector>
#include <algorithm>
#include <iostream>
#include "MessageHandler.hpp"
using namespace std;

enum METHOD {
    OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
};
/*
1. FromClientRequestHandler
    session ?
    port number ?

2. ToServerRequestHandler
    session
    rtp, rtcp port number
*/
class RequestHandler : public MessageHandler
{
private:
    Request req;
    vector<string> message_set;    // OPTIONS ~ TEARDOWN
    string server_url;
    string session;

public:
    RequestHandler() : server_url("rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov")
    {
        initRequest();
        
        intMessageSet(OPTIONS);
        intMessageSet(DESCRIBE);
        intMessageSet(SETUP);
    }
    ~RequestHandler()
    {
    }
    string getMessageSet(int index) { return message_set[index]; }
    virtual void parseData()
    {
        int find_header_last_index = data.find("\r\n\r\n") - 1;
        // body = temp[body_first_index ~ size-2]
        int body_first_index = find_header_last_index + 5;
        req.body = data.substr(body_first_index, data.size() - body_first_index - 2);

        int find_index, new_index;
        find_index = data.find_first_of("\r\n");

        // Method
        find_index = data.find_first_of(" ");
        req.method = data.substr(0, find_index);
        // URL
        new_index = find_index + 1;
        find_index = data.find_first_of(" ", new_index);
        req.url = data.substr(new_index, find_index - new_index);
        // version
        new_index = find_index + 1;
        find_index = data.find_first_of(CRLF, new_index);
        req.version = data.substr(new_index, find_index - new_index);
        // cSeq
        new_index = find_index + 2;
        find_index = data.find_first_of(CRLF, new_index);
        string Cseq_line = data.substr(new_index, find_index - new_index);
        Cseq_line.erase(remove(Cseq_line.begin(), Cseq_line.end(), ' '), Cseq_line.end());
        int separator_index = Cseq_line.find_first_of(":");
        req.cSeq = Cseq_line.substr(separator_index + 1, Cseq_line.size() - 1);

        

        // headers
        int i = 0;
        while (new_index <= find_header_last_index)
        {
            new_index = find_index + 2;
            find_index = data.find_first_of(CRLF, new_index);
            string line = data.substr(new_index, find_index - new_index);
            
            if (line.size() > 2)
            {
                separator_index = line.find_first_of(":");
                string key = line.substr(0, separator_index);
                string value = line.substr(separator_index + 1, line.size() - 1);
                req.headers.insert(make_pair(key, value));
            }
        }
    }

    virtual string makeMessage() {
        string message;
        message.append(req.method + " ");
        message.append(req.url + " ");
        message.append("RTSP/1.0");
        message.append(CRLF);

        message.append("CSeq:"+req.cSeq);
        message.append(CRLF);

        for (auto it = req.headers.begin(); it != req.headers.end(); it++)
        {
            message.append(it->first);
            message.append(":");
            message.append(it->second);
            message.append(CRLF);
        }
        message.append(CRLF);
        if (req.body.size() > 0) {
            // message.append("\r\n");
            message.append(req.body);
            message.append(CRLF);
        }
        message.append(CRLF);
        return message;
    }

    void initRequest() {
        req.method = "";
        req.url = "";
        req.version = "";
        if(req.headers.size() > 0) {
            req.headers.clear();
        }
        req.body = "";
    }

    string getURL()
    {
        return req.url;
    }

    void setURL(string input_url) {
        req.url = input_url;
    } 

    void setMethod(string method)
    {
        req.method = method;
    }

    void setCseq(string input_cSeg) {
        req.cSeq = input_cSeg;
    }

    void setSession(string input_session) { session = input_session; }

    // For Connect - Server
    // init message_set in RequestHandler()
    void intMessageSet(METHOD m) {
        string temp;
        // Request 
        switch(m) {
            case OPTIONS: {   // OPTIONS
                // Request-Line
                temp.append("OPTIONS ");
                temp.append(server_url + " ");
                temp.append("RTSP/1.0");
                temp.append(CRLF);
                // CSeq
                temp.append("CSeq:2");
                temp.append(CRLF);
                // Header
                temp.append("User-Agent: LibVLC/3.0.8 (LIVE555 Streaming Media v2016.11.28)");
                temp.append(CRLF);
                // End
                temp.append(CRLF);
                message_set.push_back(temp);
                break;
            }
            case DESCRIBE: {   // DESCRIBE
                // Request-Line
                temp.append("DESCRIBE ");
                temp.append(server_url + " ");
                temp.append("RTSP/1.0");
                temp.append(CRLF);
                // CSeq
                temp.append("CSeq:3");
                temp.append(CRLF);
                // Header
                temp.append("User-Agent: LibVLC/3.0.8 (LIVE555 Streaming Media v2016.11.28)");
                temp.append(CRLF);
                temp.append("Accept: application/sdp");
                temp.append(CRLF);
                // End
                temp.append(CRLF);
                message_set.push_back(temp);
                break;
            }
            case SETUP: {   // SETUP
                // Request-Line
                temp.append("SETUP ");
                temp.append(server_url + "/trackID=1 ");
                temp.append("RTSP/1.0");
                temp.append(CRLF);
                // CSeq
                temp.append("CSeq:4");
                temp.append(CRLF);
                // Header
                temp.append("User-Agent: LibVLC/3.0.8 (LIVE555 Streaming Media v2016.11.28)");
                temp.append(CRLF);
                temp.append("Transport: RTP/AVP;unicast;client_port=60126-60127");
                temp.append(CRLF);
                // End
                temp.append(CRLF);
                message_set.push_back(temp);
                break;
            }
            default:
                break;
            // case PLAY: {   // PLAY
            //     // Request-Line
            //     temp.append("PLAY ");
            //     temp.append(server_url + " ");
            //     temp.append("RTSP/1.0");
            //     temp.append(CRLF);
            //     // CSeq
            //     temp.append("CSeq:5");
            //     temp.append(CRLF);
            //     // Header
            //     temp.append("User-Agent: LibVLC/3.0.8 (LIVE555 Streaming Media v2016.11.28)");
            //     temp.append(CRLF);
            //     temp.append("Session: 1576488959");
            //     temp.append(CRLF);
            //     temp.append("Range: npt=0.000-");
            //     temp.append(CRLF);
            //     // End
            //     temp.append(CRLF);
            //     message_set.push_back(temp);
            //     break;
            // }
            // case TEARDOWN: {   // TEARDOWN
            //     break;
            // }
        }
    }
};
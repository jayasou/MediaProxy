#include "MessageHandler.hpp"

#include <iostream>
#include <fstream>

class ResponseHandler : public MessageHandler {
    private:
        Response res;
        string session;
        string server_rtp_port;
        string server_rtcp_port;
        ofstream outfile;
        
    public:
        ResponseHandler() {
            cout << "open response.txt\n";
            outfile.open("response.txt");
        }
        ~ResponseHandler() {
            cout << "close response.txt\n";
            outfile.close();
        }
        bool checkContentLengthHeader(string input_header) {
            int separator_index = input_header.find_first_of(":");
            string key = input_header.substr(0, separator_index);
            string value = input_header.substr(separator_index + 1, input_header.size() - 1);
            if(key == "Content-Length" && stoi(value) > 0) {
                return true;
            }
            return false;
        }

        void saveResponseToFile(string input_message) {
            outfile << "--------------------------\n";
            outfile << input_message;
        }
        
        virtual void parseData() {
            int find_header_last_index = data.find("\r\n\r\n") - 1;
            // body = temp[body_first_index ~ size-2]
            int body_first_index = find_header_last_index + 5;
            res.body = data.substr(body_first_index, data.size() - body_first_index - 2);
            
            int find_index, new_index;

            // version
            find_index = data.find_first_of(" ");
            res.version = data.substr(0, find_index);
            // status_code
            new_index = find_index + 1;
            find_index = data.find_first_of(" ", new_index);
            res.status_code = data.substr(new_index, find_index - new_index);
            // paharse
            new_index = find_index + 1;
            find_index = data.find_first_of(CRLF, new_index);
            res.pharse = data.substr(new_index, find_index - new_index);
            // cSeq
            new_index = find_index + 2;
            find_index = data.find_first_of(CRLF, new_index);
            string Cseq_line = data.substr(new_index, find_index - new_index);
            Cseq_line.erase(remove(Cseq_line.begin(), Cseq_line.end(), ' '), Cseq_line.end());
            int separator_index = Cseq_line.find_first_of(":");
            res.cSeq = Cseq_line.substr(separator_index + 1, Cseq_line.size() - 1);

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
                    if(key == "Transport") {
                    
                    }
                    res.headers.insert(make_pair(key, value));
                }
            }
        }

        virtual string makeMessage() {
            string message;
            message.append(res.version + " ");
            message.append(res.status_code + " ");
            message.append(res.pharse);
            message.append(CRLF);

            message.append("CSeq:"+res.cSeq);
            message.append(CRLF);

            for (auto it = res.headers.begin(); it != res.headers.end(); it++)
            {
                message.append(it->first);
                message.append(":");
                message.append(it->second);
                message.append(CRLF);
            }
            message.append(CRLF);
            if (res.body.size() > 0) {
                // message.append("\r\n");
                message.append(res.body);
                message.append(CRLF);
            }
            return message;
        }
        void setStatus_coode(string input_status_code) {
            res.status_code = input_status_code;
        }
        void setPharse(string input_pharse) {
            res.pharse = input_pharse;
        }
};
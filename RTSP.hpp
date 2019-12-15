#include <string>
#include <map>
using namespace std;

struct Response {
    // private:
        string version;
        string status_code;
        string pharse;
        string cSeq;
        map<string, string> headers;
        string body;
};

struct Request {
    // private:
        string method;
        // string host_name;
        // string password;
        // string ip;
        // int port;
        // string path;
        string url;
        string version;
        string cSeq;
        map<string, string> headers;
        string body;    
};


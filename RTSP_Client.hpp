#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <iostream>
#include <string>
#include "RequestHandler.hpp"
#include "ResponseHandler.hpp"

using boost::asio::ip::tcp;

class RTSP_Client
{
private:
    int count;
    RequestHandler req;
    ResponseHandler res;

    boost::asio::streambuf response_;
    tcp::resolver::results_type endpoints_;
    tcp::socket socket_;
    std::string input_buffer_;
    bool stopped_;

    boost::asio::steady_timer deadline_;
    boost::asio::steady_timer heartbeat_timer_;

public:
    RTSP_Client(boost::asio::io_context &io_context): stopped_(false),
        socket_(io_context),
        deadline_(io_context),
        heartbeat_timer_(io_context), count(0) {}
    void start(tcp::resolver::results_type endpoints);
    void stop();

private:
    void start_connect(tcp::resolver::results_type::iterator endpoint_iter);
    void handle_connect(const boost::system::error_code &ec, tcp::resolver::results_type::iterator endpoint_iter);
    void start_read();
    void handle_read_status_line(const boost::system::error_code &ec, std::size_t n);
    void handle_read_headers(const boost::system::error_code &ec, std::size_t n);
    void handle_read_body(const boost::system::error_code &ec, std::size_t n);
    void start_write();
    void handle_write(const boost::system::error_code &ec);
    void check_deadline();
};
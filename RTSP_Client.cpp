#include "RTSP_Client.hpp"

void RTSP_Client::start(tcp::resolver::results_type endpoints)
{
    // Start the connect actor.
    endpoints_ = endpoints;
    start_connect(endpoints_.begin());

    deadline_.async_wait(boost::bind(&RTSP_Client::check_deadline, this));
}

void RTSP_Client::stop()
{
    stopped_ = true;
    boost::system::error_code ignored_ec;
    socket_.close(ignored_ec);
    deadline_.cancel();
    heartbeat_timer_.cancel();
}

void RTSP_Client::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
{
    if (endpoint_iter != endpoints_.end())
    {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

        // Set a deadline for the connect operation.
        deadline_.expires_after(boost::asio::chrono::seconds(60));

        // Start the asynchronous connect operation.
        socket_.async_connect(endpoint_iter->endpoint(),
                              boost::bind(&RTSP_Client::handle_connect,
                                          this, _1, endpoint_iter));
    }
    else
    {
        // There are no more endpoints to try. Shut down the client.
        stop();
    }
}

void RTSP_Client::handle_connect(const boost::system::error_code &ec, tcp::resolver::results_type::iterator endpoint_iter)
{
    if (stopped_)
        return;

    // The async_connect() function automatically opens the socket at the start
    // of the asynchronous operation. If the socket is closed at this time then
    // the timeout handler must have run first.
    if (!socket_.is_open())
    {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
    }

    // Check if the connect operation failed before the deadline expired.
    else if (ec)
    {
        std::cout << "Connect error: " << ec.message() << "\n";

        // We need to close the socket used in the previous connection attempt
        // before starting a new one.
        socket_.close();

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
    }

    // Otherwise we have successfully established a connection.
    else
    {
        std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
        // Start the heartbeat actor.
        start_write();
        // Start the input actor.
        start_read();
    }
}

void RTSP_Client::start_read()
{
    // Set a deadline for the read operation.
    // deadline_.expires_after(boost::asio::chrono::seconds(30));

    // Start an asynchronous operation to read a newline-delimited message.
    // boost::asio::async_read_until(socket_,
    //                               boost::asio::dynamic_buffer(input_buffer_), "\r\n",
    //                               boost::bind(&RTSP_Client::handle_read_status_line, this, _1, _2));

    boost::asio::async_read_until(socket_,
                                  boost::asio::dynamic_buffer(input_buffer_), "\r\n",
                                  boost::bind(&RTSP_Client::handle_read_status_line, this, _1, _2));

    // boost::asio::async_read(socket_, response_,
    //                         boost::asio::transfer_at_least(1),
    //                         boost::bind(&RTSP_Client::handle_read_status_line, this, _1, _2));
}

void RTSP_Client::handle_read_status_line(const boost::system::error_code &ec, std::size_t n)
{

    if (stopped_)
    {
        cout << "Read  Stop()\n";
        return;
    }

    if (!ec)
    {
        // // Extract the newline-delimited message from the buffer.
        std::string line(input_buffer_.substr(0, n - 1));
        input_buffer_.erase(0, n);
        // // Empty messages are heartbeats and so ignored.
        // if (!line.empty())
        // {
        //     std::cout << "StatusLine : " << line << "\n";
        //     res.saveResponseToFile(line);
        // }

        cout <<"---StatusLine---\n" <<  line << endl;

        // std::istream response_stream(&response_);
        // std::string rtsp_version;
        // unsigned int status_code;
        // std::string pharse;
        // response_stream >> rtsp_version;
        // response_stream >> status_code;
        // response_stream >> pharse;

        
        // start_read();
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(input_buffer_), "\r\n\r\n",
                            // boost::asio::transfer_at_least(1),
                            boost::bind(&RTSP_Client::handle_read_headers, this, _1, _2));
    }
    else
    {
        std::cout << "Error on receive: " << ec.message() << "\n";
        stop();
    }
}

void RTSP_Client::handle_read_headers(const boost::system::error_code &ec, std::size_t n)
{

    if (stopped_)
    {
        cout << "Read  Stop()\n";
        return;
    }
    if (!ec)
    {
        cout << "----Header----\n";
        std::string line(input_buffer_.substr(0, n - 1));
        input_buffer_.erase(0, n);
        cout << line << endl;
        if(line.find("Content-Length") != string::npos) {
            boost::asio::async_read(socket_, boost::asio::dynamic_buffer(input_buffer_),
                            boost::asio::transfer_at_least(1),
                            boost::bind(&RTSP_Client::handle_read_body, this, _1, _2));
        }
        else {
            start_read();
        }

        // std::istream response_stream(&response_);
        // string header;
        // int c = 0;
        // bool checkContentLength = false;
        // cout << "---Header----\n";
        // while (std::getline(response_stream, header) && header != "\r")
        // {
        //     std::cout << ++c << ": " << header << "\n";
        // } cout << "\n";
        
    }
    else
    {
        std::cout << "Error on receive: " << ec.message() << "\n";
        stop();
    }
}
void RTSP_Client::handle_read_body(const boost::system::error_code &ec, std::size_t n)
{
    if (stopped_)
    {
        cout << "Read  Stop()\n";
        return;
    }
    if (!ec)
    {
        cout << "---body---"<<n << "\n";
        std::string line(input_buffer_.substr(0, n - 1));
        input_buffer_.erase(0, n);
        cout << line << endl;
        // cout << &response_;

        // cout << input_buffer_ << endl;

        start_read();
    }
    else
    {
        std::cout << "Error on receive: " << ec.message() << "\n";
        stop();
    }
}

void RTSP_Client::start_write()
{
    if (stopped_)
    {
        cout << "start_write STOP\n";
        return;
    }

    if (count < 3)
    {
        // Start an asynchronous operation to send a heartbeat message.
        boost::asio::async_write(socket_, boost::asio::buffer(req.getMessageSet(count), req.getMessageSet(count).size()),
                                 boost::bind(&RTSP_Client::handle_write, this, _1));
    }
}

void RTSP_Client::handle_write(const boost::system::error_code &ec)
{
    if (stopped_)
    {
        cout << "handler_write 110000\n";
        return;
    }
    if (!ec)
    {
        count++;
        // Wait 10 seconds before sending the next heartbeat.
        heartbeat_timer_.expires_after(boost::asio::chrono::seconds(10));
        heartbeat_timer_.async_wait(boost::bind(&RTSP_Client::start_write, this));
    }
    else
    {
        std::cout << "Error on heartbeat: " << ec.message() << "\n";

        stop();
    }
}

void RTSP_Client::check_deadline()
  {
    if (stopped_)
      return;

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expiry() <= boost::asio::steady_timer::clock_type::now())
    {
      // The deadline has passed. The socket is closed so that any outstanding
      // asynchronous operations are cancelled.
      socket_.close();

      // There is no longer an active deadline. The expiry is set to the
      // maximum time point so that the actor takes no action until a new
      // deadline is set.
      deadline_.expires_at(boost::asio::steady_timer::time_point::max());
    }

    // Put the actor back to sleep.
    deadline_.async_wait(boost::bind(&RTSP_Client::check_deadline, this));
  }

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        tcp::resolver r(io_context);
        RTSP_Client c(io_context);

        c.start(r.resolve(argv[1], argv[2]));

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
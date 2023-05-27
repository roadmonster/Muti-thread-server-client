#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: client <host> <port>" << std::endl;
        return 1;
    }

    boost::asio::io_service io_service;
    tcp::socket socket(io_service);

    try {
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(argv[1], argv[2]);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        boost::asio::connect(socket, endpoint_iterator);

        std::string message;
        std::cout << "Enter your name: ";
        std::getline(std::cin, message);
        message += "\n";

        boost::asio::write(socket, boost::asio::buffer(message));

        boost::asio::streambuf response;
        boost::asio::async_read_until(socket, response, '\n',
            [&response](const boost::system::error_code& error, size_t bytes_transferred) {
                if (!error) {
                    std::istream input_stream(&response);
                    std::string response_string;
                    std::getline(input_stream, response_string);
                    std::cout << "Server says: " << response_string << std::endl;
                }
            });

        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

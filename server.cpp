#include <iostream>
#include <memory>
#include <thread>
#include <boost/asio.hpp>

using namespace boost::asio;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  typedef std::shared_ptr<TcpConnection> pointer;
  static pointer create(io_service& io_service) {
    return pointer(new TcpConnection(io_service));
  }
  ip::tcp::socket& socket() {
    return socket_;
  }
  void Start() {
    std::cout << "Starting connection" << std::endl;
    async_read_until(socket_, buf_, '\n', [this, self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
      if (!error) {
        std::istream input(&buf_);
        std::string client_name;
        std::getline(input, client_name);
        std::cout << "Received name: " << client_name << std::endl;
        std::string greeting = "Hello " + client_name + "\n";
        async_write(socket_, buffer(greeting), [this, self](const boost::system::error_code& error, size_t bytes_transferred) {
          if (!error) {
            std::cout << "Sent greeting to client " << std::endl;
          }
        });
      }
    });
  }

 private:
  TcpConnection(io_service& io_service) : socket_(io_service) {}

  ip::tcp::socket socket_;
  boost::asio::streambuf buf_;
};

class TcpServer {
 public:
  TcpServer(io_service& io_service, short port, int num_threads)
      : io_service_(io_service),
        acceptor_(io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
        num_threads_(num_threads) {}
  void Start() {
    std::cout << "Starting server" << std::endl;
    StartAccept();
    for (int i = 0; i < num_threads_; ++i) {
      threads_.emplace_back([this] { io_service_.run(); });
    }
    for (auto& thread : threads_) {
      thread.join();
    }
  }

 private:
  void StartAccept() {
    auto new_connection = TcpConnection::create(io_service_);
    acceptor_.async_accept(new_connection->socket(), [this, new_connection](const boost::system::error_code& error) {
      if (!error) {
        std::cout << "Accepted new connection" << std::endl;
        new_connection->Start();
        StartAccept();
      }
    });
  }

  io_service& io_service_;
  ip::tcp::acceptor acceptor_;
  int num_threads_;
  std::vector<std::thread> threads_;
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: server <port> <num_threads>" << std::endl;
    return 1;
  }
  short port = std::atoi(argv[1]);
  int num_threads = std::atoi(argv[2]);

  io_service io_service;
  std::make_shared<TcpServer>(io_service, port, num_threads)->Start();
  io_service.run();
  return 0;
}

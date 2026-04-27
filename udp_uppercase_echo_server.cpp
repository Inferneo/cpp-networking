#include <boost/algorithm/string/case_conv.hpp>
#include <boost/asio.hpp>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

using boost::asio::ip::udp;

class server {
  public:
  server(boost::asio::io_context& io_context, short port)
      : socket_(io_context, udp::endpoint(udp::v4(), port)) {
    do_receive();
  }

  void do_receive() {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buf), sender_endpoint_, [this](boost::system::error_code ec, std::size_t bytes_recvd) {
          if(!ec && bytes_recvd > 0) {
            handle_receive(bytes_recvd);
          } else {
            do_receive();
          }
        });
  }

  void handle_receive(std::size_t bytes_recvd) {
    std::string msg(recv_buf.data(), bytes_recvd);
    boost::algorithm::to_upper(msg);

    auto response = std::make_shared<std::string>(std::move(msg));

    socket_.async_send_to(boost::asio::buffer(*response),
                          sender_endpoint_,
                          [this, response](boost::system::error_code, std::size_t) { do_receive(); });
  }

  private:
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  std::array<char, 1024> recv_buf;
};

int main(int argc, char* argv[]) {
  try {
    if(argc != 2) {
      std::cerr << "Usage : ex1 <port>\n";
      return 1;
    }

    boost::asio::io_context io_context{};
    server s(io_context, atoi(argv[1]));
    io_context.run();
  } catch(std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "../ICMP/icmp_header.hpp"
#include "../ICMP/ipv4_header.hpp"

using boost::asio::steady_timer;
using boost::asio::ip::icmp;
using boost::system::error_code;
using std::size_t;
namespace chrono = boost::asio::chrono;

class subnet_pinger {
  public:
  subnet_pinger(boost::asio::io_context& io, const std::string& cidr)
      : socket_(io, icmp::v4())
      , timer_(io) {
    parse_cidr(cidr);
    start_receive();
    send_all();
    start_timeout();
  }

  private:
  void parse_cidr(const std::string& cidr) {
    auto slash = cidr.find('/');
    std::string ip = cidr.substr(0, slash);
    int prefix = std::stoi(cidr.substr(slash + 1));

    uint32_t addr = boost::asio::ip::make_address_v4(ip).to_uint();
    uint32_t mask = prefix == 0 ? 0 : 0xFFFFFFFF << (32 - prefix);
    uint32_t network = addr & mask;

    host_bits_ = 32 - prefix;
    network_base_ = network;
  }

  void send_all() {
    std::string body("scan");

    for(uint32_t i = 1; i < (1u << host_bits_) - 1; ++i) {
      uint32_t ip = network_base_ + i;
      icmp::endpoint dest(boost::asio::ip::address_v4(ip), 0);

      icmp_header req;
      req.type(icmp_header::echo_request);
      req.code(0);
      req.identifier(get_identifier());
      req.sequence_number(++sequence_);
      compute_checksum(req, body.begin(), body.end());

      boost::asio::streambuf buf;
      std::ostream os(&buf);
      os << req << body;

      socket_.send_to(buf.data(), dest);
    }
  }

  void start_receive() {
    reply_buffer_.consume(reply_buffer_.size());
    socket_.async_receive(reply_buffer_.prepare(65536), [this](error_code ec, size_t length) {
      if(!ec)
        handle_receive(length);
      start_receive();
    });
  }

  void handle_receive(std::size_t length) {
    reply_buffer_.commit(length);

    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    if(is && icmp_hdr.type() == icmp_header::echo_reply && icmp_hdr.identifier() == get_identifier()) {
      std::cout << ipv4_hdr.source_address() << " is alive\n";
    }
  }

  void start_timeout() {
    timer_.expires_after(chrono::seconds(3));
    timer_.async_wait([this](boost::system::error_code) { socket_.close(); });
  }

  static unsigned short get_identifier() {
#if defined(BOOST_ASIO_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
  }

  icmp::socket socket_;
  steady_timer timer_;
  boost::asio::streambuf reply_buffer_;

  uint32_t network_base_;
  int host_bits_;
  unsigned short sequence_ = 0;
};

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cerr << "Usage: scan <subnet>\n";
    std::cerr << "Example: 192.168.1.0/24\n";
    return 1;
  }

  boost::asio::io_context io;
  subnet_pinger p(io, argv[1]);
  io.run();
}

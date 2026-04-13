#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

//------------------------------------------------------------------------------

void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

class Session : public std::enable_shared_from_this<Session> {
  tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::empty_body> req_;
  http::response<http::string_body> res_;

  public:
  explicit Session(net::io_context& ioc)
      : resolver_(net::make_strand(ioc))
      , stream_(net::make_strand(ioc)) { }

  void run() {
    const char* host = "example.com";
    const char* port = "80";
    const char* target = "/";

    req_.version(11);
    req_.method(http::verb::get);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::accept, "text/html");
    req_.set(http::field::accept_language, "en-us");
    req_.set(http::field::accept_encoding, "identity");
    req_.set(http::field::connection, "close");
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    resolver_.async_resolve(host, port, beast::bind_front_handler(&Session::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if(ec)
      return fail(ec, "resolve");

    stream_.expires_after(std::chrono::seconds(30));

    stream_.async_connect(results, beast::bind_front_handler(&Session::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if(ec)
      return fail(ec, "connect");

    stream_.expires_after(std::chrono::seconds(30));

    http::async_write(stream_, req_, beast::bind_front_handler(&Session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t) {
    if(ec)
      return fail(ec, "write");

    http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t) {
    if(ec)
      return fail(ec, "read");

    // print only response body (like second program effectively does)
    std::cout << res_.body();

    beast::error_code shutdown_ec;
    auto err = stream_.socket().shutdown(tcp::socket::shutdown_both, shutdown_ec);
  }
};

//------------------------------------------------------------------------------

int main() {
  net::io_context ioc;
  std::make_shared<Session>(ioc)->run();
  ioc.run();
}

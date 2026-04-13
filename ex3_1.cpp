#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using resolver = boost::asio::ip::tcp::resolver;
using std::string;

string make_request(const string& host, net::io_context& io_context) {
  resolver resolver{ io_context };
  const auto results = resolver.resolve(host, "http");

  beast::tcp_stream stream{ io_context };
  stream.connect(results);

  http::request<http::string_body> req{ http::verb::get, "/", 11 };
  req.set(http::field::host, host);
  req.set(http::field::accept, "text/html");
  req.set(http::field::accept_language, "en-us");
  req.set(http::field::accept_encoding, "identity");
  req.set(http::field::connection, "close");
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(stream, req);

  beast::flat_buffer buffer;
  http::response<http::dynamic_body> res;
  http::read(stream, buffer, res);

  beast::error_code ec;
  auto ph = stream.socket().shutdown(tcp::socket::shutdown_both, ec);
  if(ec && ec != beast::errc::not_connected)
    throw beast::system_error{ ec };

  std::ostringstream response_stream;
  response_stream << res;
  return response_stream.str();
}

int main() {
  net::io_context ioc;
  try {
    const auto response = make_request("example.com", ioc);
    std::cout << response << std::endl;
  } catch(beast::system_error const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
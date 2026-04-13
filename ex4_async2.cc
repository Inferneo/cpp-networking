#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

std::string doc_root = ".";

bool ends_with(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string mime_type(const std::string& path) {
  if(ends_with(path, ".html"))
    return "text/html";
  if(ends_with(path, ".css"))
    return "text/css";
  if(ends_with(path, ".js"))
    return "application/javascript";
  if(ends_with(path, ".png"))
    return "image/png";
  return "text/plain";
}

struct Session : std::enable_shared_from_this<Session> {
  tcp::socket socket;
  beast::flat_buffer buffer;
  http::request<http::string_body> req;

  Session(tcp::socket s)
      : socket(std::move(s)) { }

  void start() {
    http::async_read(socket, buffer, req, beast::bind_front_handler(&Session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t) {
    if(ec)
      return;

    if(req.method() != http::verb::get) {
      auto res = http::response<http::string_body>{ http::status::bad_request, req.version() };
      res.body() = "only GET supported";
      res.prepare_payload();
      return write(std::move(res));
    }

    std::string path = doc_root + std::string(req.target());
    if(req.target() == "/")
      path += "index.html";

    beast::error_code file_ec;
    http::file_body::value_type file;
    file.open(path.c_str(), beast::file_mode::scan, file_ec);

    if(file_ec) {
      auto res = http::response<http::string_body>{ http::status::not_found, req.version() };
      res.body() = "file not found";
      res.prepare_payload();
      return write(std::move(res));
    }

    auto size = file.size();

    http::response<http::file_body> res{ std::piecewise_construct,
                                         std::make_tuple(std::move(file)),
                                         std::make_tuple(http::status::ok, req.version()) };

    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);

    write(std::move(res));
  }

  template <class Response>
  void write(Response&& res) {
    auto sp = std::make_shared<std::decay_t<Response>>(std::forward<Response>(res));

    http::async_write(socket, *sp, [self = shared_from_this(), sp](beast::error_code ec, std::size_t) {
      auto err = self->socket.shutdown(tcp::socket::shutdown_send, ec);
    });
  }
};

int main() {
  net::io_context ioc;

  tcp::acceptor acceptor{ ioc, { tcp::v4(), 8080 } };

  std::function<void()> do_accept;

  do_accept = [&]() {
    acceptor.async_accept([&](beast::error_code ec, tcp::socket socket) {
      if(!ec)
        std::make_shared<Session>(std::move(socket))->start();
      do_accept();
    });
  };

  do_accept();
  ioc.run();
}

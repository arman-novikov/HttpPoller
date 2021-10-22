#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <string>
#include <memory>
#include <string>

#include "requestcontext.hpp"

class Session : public std::enable_shared_from_this<Session>
{
    static constexpr uint32_t DEFAULT_REQUEST_TIMEOUT_MLS = 5000;
public:
    using tcp = boost::asio::ip::tcp;
    explicit Session(boost::asio::io_context& ioc,
                     uint32_t timeout_mls = DEFAULT_REQUEST_TIMEOUT_MLS);
    explicit Session(boost::asio::io_context& ioc,
                     const RequestContext& rctx,
                     uint32_t timeout_mls = DEFAULT_REQUEST_TIMEOUT_MLS);
    std::string get_result();
    void run(char const* host,
             char const* port,
             char const* target,
             boost::beast::http::verb method,
             std::string_view body,
             int version);

    void set_rctx(const RequestContext& rctx);
    RequestContext get_rctx() const;
    void set_timeout(uint32_t timeout_mls);
    void run_from_context();

private:
    void on_resolve(boost::beast::error_code ec,
                    tcp::resolver::results_type results);
    void on_connect(boost::beast::error_code ec,
                    tcp::resolver::results_type::endpoint_type);
    void on_write(boost::beast::error_code ec,
                  std::size_t bytes_transferred);
    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
    void on_fail(boost::beast::error_code ec, const char* what);
    std::chrono::milliseconds get_request_timeout_() const;

    tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    //boost::beast::http::request<boost::beast::http::empty_body> req_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    std::string fail_desc_;
    RequestContext rctx_;
    uint32_t request_timeout_mls_;
};

#include "session.hpp"

Session::Session(boost::asio::io_context& ioc, uint32_t timeout_mls):
        resolver_(boost::asio::make_strand(ioc)),
        stream_(boost::asio::make_strand(ioc)),
        request_timeout_mls_{timeout_mls}
{}

Session::Session(boost::asio::io_context& ioc,
                 const RequestContext& rctx,
                 uint32_t timeout_mls):
    resolver_(boost::asio::make_strand(ioc)),
    stream_(boost::asio::make_strand(ioc)),
    rctx_{rctx},
    request_timeout_mls_{timeout_mls}
{}

void Session::run(char const* host,
         char const* port,
         char const* target,
         boost::beast::http::verb method,
         int version)
{
    req_.version(version);
    req_.method(method);
    req_.target(target);
    req_.set(boost::beast::http::field::host, host);
    req_.set(boost::beast::http::field::user_agent,
             BOOST_BEAST_VERSION_STRING);

    resolver_.async_resolve(
        host,
        port,
        boost::beast::bind_front_handler(
            &Session::on_resolve,
            shared_from_this())
    );
}

void Session::run_from_context() {
    run(rctx_.host.data(),
        rctx_.port.data(),
        rctx_.target.data(),
        rctx_.method,
        rctx_.http_version);
}

void Session::set_rctx(const RequestContext& rctx) {
    rctx_ = rctx;
}

RequestContext Session::get_rctx() const {
    return rctx_;
}

void Session::on_resolve(boost::beast::error_code ec,
                tcp::resolver::results_type results)
{
    if(ec)
        return on_fail(ec, "resolve");

    stream_.expires_after(get_request_timeout_());

    stream_.async_connect(results,
        boost::beast::bind_front_handler(
            &Session::on_connect,
            shared_from_this())
    );
}

void Session::on_connect(boost::beast::error_code ec,
           tcp::resolver::results_type::endpoint_type)
{
    if(ec)
        return on_fail(ec, "connect");

    stream_.expires_after(get_request_timeout_());

     boost::beast::http::async_write(stream_, req_,
        boost::beast::bind_front_handler(
            &Session::on_write,
            shared_from_this()));
}

void Session::on_write(boost::beast::error_code ec,
              std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return on_fail(ec, "write");

     boost::beast::http::async_read(stream_, buffer_, res_,
        boost::beast::bind_front_handler(
            &Session::on_read,
            shared_from_this()));
}

void Session::on_read(boost::beast::error_code ec,
                      std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return on_fail(ec, "read");

    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    if(ec && ec != boost::beast::errc::not_connected)
        return on_fail(ec, "shutdown");
}


std::string Session::get_result() {
    return res_.body().size()? res_.body() : fail_desc_;
}

std::chrono::milliseconds Session::get_request_timeout_() const {
     return std::chrono::milliseconds(request_timeout_mls_);
}

void Session::set_timeout(uint32_t timeout_mls) {
    request_timeout_mls_ = timeout_mls;
}

void Session::on_fail(boost::beast::error_code ec, const char* what) {
    fail_desc_ = std::string(what) + std::string(" ") + ec.message();
}

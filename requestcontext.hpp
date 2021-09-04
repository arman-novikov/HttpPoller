#pragma once
#include <string>
#include <string_view>
#include <boost/beast/http.hpp>

struct RequestContext
{
    using verb = boost::beast::http::verb;
    RequestContext() = default;
    RequestContext(std::string_view host,
                   std::string_view port,
                   std::string_view target,
                   verb method = verb::get,
                   int http_version = 11);

    RequestContext(std::string_view host,
                   uint16_t port,
                   std::string_view target,
                   verb method = verb::get,
                   int http_version = 11);
    friend bool operator==(const RequestContext& r,
                           const RequestContext& l);
    RequestContext& operator=(const RequestContext&) = default;
    std::string host;
    std::string port;
    std::string target;
    verb method;
    int http_version;
    boost::beast::http::response<boost::beast::http::string_body> res;

    struct Hasher {
        size_t operator()(const RequestContext& ctx) const {
            return std::hash<std::string>{}(ctx.host + ctx.port + ctx.target);
        }
    };
};

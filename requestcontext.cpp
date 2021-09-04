#include "requestcontext.hpp"

RequestContext::RequestContext(std::string_view host,
                               std::string_view port,
                               std::string_view target,
                               verb method,
                               int http_version):
    host{host}, port{port},
    target{target}, method{method}, http_version{http_version}
{}

RequestContext::RequestContext(std::string_view host,
                               uint16_t port,
                               std::string_view target,
                               verb method,
                               int http_version):
    host{host}, port{std::to_string(port)},
    target{target}, method{method}, http_version{http_version}
{}


bool operator==(const RequestContext& r,
                const RequestContext& l) {
    return r.host + r.port + r.target == l.host + l.port + l.target;
}

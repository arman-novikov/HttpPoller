#pragma once
#include <unordered_map>
#include <mutex>
#include <boost/asio.hpp>
#include <condition_variable>
#include <thread>
#include "requestcontext.hpp"

class HttpPoller
{
    using verb = boost::beast::http::verb;
    using agents_t = std::unordered_map<
        RequestContext, std::string, RequestContext::Hasher
    >;
public:
    template<typename T>
    HttpPoller(const T& contexts);
    void Add(const RequestContext& contexts);
    void Add(std::string_view host,
             uint16_t port,
             std::string_view target,
             verb method = verb::get,
             int http_version = 11);
    void Poll();
    void StartPolling(size_t timeout_mls);
    void StopPolling();
    agents_t Get();
    ~HttpPoller() {
        StopPolling();
    }
private:
    void poll_routine_(size_t timeout_mls);
    std::thread poller_;
    boost::asio::io_context ioc_;
    agents_t agents_;
    std::mutex agents_mtx_, wrk_mtx_;
    std::condition_variable wrk_cv_;
    bool wrk_flag_;
};

template<typename T>
HttpPoller::HttpPoller(const T& contexts) {
    for (const auto& ctx: contexts)
        Add(ctx);
}

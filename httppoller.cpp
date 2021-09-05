#include <iostream>

#include "httppoller.hpp"

#include <unordered_set>
#include <algorithm>
#include <chrono>

#include "session.hpp"

void HttpPoller::Add(const RequestContext& ctx) {
    std::scoped_lock lock(agents_mtx_);
    agents_.insert({ctx, {""}});
}

void HttpPoller::Add(std::string_view host,
                       uint16_t port,
                       std::string_view target,
                       verb method,
                       int http_version)
{
    Add({host, port, target, method, http_version});
}

HttpPoller::agents_t HttpPoller::Get() {
    std::scoped_lock slck(agents_mtx_);
    return agents_;
}

void HttpPoller::StartPolling(size_t timeout_mls) {
    wrk_flag_ = true;
    poller_ = std::thread(&HttpPoller::poll_routine_, this, timeout_mls);
}

void HttpPoller::poll_routine_(size_t timeout_mls) {
    const auto timeout = std::chrono::milliseconds(timeout_mls);
    auto wrk_pred = [this]() -> bool {
        return wrk_flag_ == false;
    };
    std::unique_lock ulock{wrk_mtx_};
    long counter = 0;
    long sum = 0;
    do {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        Poll();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        ++counter;
        std::cout << "avr time difference = " << sum/counter << "mls" << std::endl;
    } while (!wrk_cv_.wait_for(ulock, timeout, wrk_pred));
    std::cerr << "poll_routine_ out" << std::endl;
}

void HttpPoller::StopPolling() {
    {
        std::scoped_lock slck(wrk_mtx_);
        wrk_flag_ = false;
    }
    wrk_cv_.notify_all();
    if (poller_.joinable()) {
        poller_.join();
    }
};

void HttpPoller::Poll() {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace net = boost::asio;
    std::vector<std::shared_ptr<Session>> cache;
    ioc_.restart();
    {
        std::scoped_lock lock(agents_mtx_);
        cache.reserve(agents_.size());
        for (const auto& [ctx, _]: agents_) {
            cache.push_back(std::make_shared<Session>(ioc_, ctx));
            cache.back()->run_from_context();
        }
    }
    ioc_.run();

    std::scoped_lock lock(agents_mtx_);
    std::for_each(cache.begin(), cache.end(),
                  [this](std::shared_ptr<Session> session){
       agents_[session->get_rctx()] = session->get_result();
    });

    //std::cerr << "polled" << std::endl;
}

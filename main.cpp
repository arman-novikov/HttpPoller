#include <vector>
#include <iostream>

#include "httppoller.hpp"


int main(int, char**)
{
    static const char* host = "192.168.1.44";
    static const char* target = "/v1/health";
    static const char* body = "{\"clients\": [\"72848f6a75ceb5ef3140fc01aa9fad715099e5c388e4e6798028e7da89bb4914\"]}";


    std::vector<RequestContext> contexts {
        //{"httpbin.org", "81", "/get"},
        //{host, "9092", std::string(target)},
        {host, "9091", "/v1/compute", boost::beast::http::verb::post, body},
        //{host, "9093", target},
        //{host, "9094", std::string_view(target)},
        //{host, 9100, target}, // invalid port
        ///{host, 9091, "/invalid/target"},
    };

    HttpPoller hp{contexts};
    hp.StartPolling(1000);
    while (true) {
        int x = 0;
        std::cin >> x;
        if (x > 0) {
            for (const auto& [ctx, uuid]: hp.Get()) {
                std::cout << ctx.host << ":" << ctx.port << ctx.target
                          << ": " << uuid << std::endl;
            }
        } else if (x < 0) {
            hp.Add({host, 9095, target});
        } else {
            return 0;
        }
    }
}

//TicketSystem main.cpp for test

#include "systemcore.hpp"

Sirius::System ticketSystem;
std::string buffer;

int main() {
    while (getline(std::cin, buffer)) {
        const auto& ret = ticketSystem.response(buffer);
        std::cout << ret.second << '\n';
        if (!ret.first) return 0;
    }
    return 0;
}
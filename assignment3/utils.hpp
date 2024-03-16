#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

std::string get_host_name();

std::string get_ip_from_name(std::string hostname);

int build_server(int port);

int build_client(const char * hostname, int port);

#endif  // UTILS_HPP
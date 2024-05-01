#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cstdio>

#define INFO_OUT
#define DEBUG_OUT

#ifdef INFO_OUT
#define INFO(...) \
printf("[INFO ] %s  %s:%d\n\t", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define INFO(...)
#endif

#ifdef DEBUG_OUT
#define DEBUG(...) \
printf("[DEBUG] %s  %s:%d\n\t", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define DEBUG(...)
#endif

#define ERROR(...) \
printf("[ERROR] %s  %s:%d\n\t", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
printf(__VA_ARGS__); \
printf("\n");


std::string get_host_name();

std::string get_ip_from_name(std::string hostname);

int build_server(int port);

int build_client(const char * hostname, int port);

int get_port_num(int socket);

int gen_random(int lower_bound, int upper_bound);

#endif  // UTILS_HPP
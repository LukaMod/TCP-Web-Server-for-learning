cmake_minimum_required(VERSION 3.12)
project(client_server_design)

# lib

add_library(library lib/error.c lib/event_handler.c lib/sock_unix.c)

#client

add_executable(client src/client.c src/web.c)

target_link_libraries(client PRIVATE library)

# kq server

add_executable(kq_server src/serv.c src/web.c)

target_link_libraries(kq_server PRIVATE library)

# one child per connection

add_executable(serv_fork_per src/serv_fork_per.c src/web.c)

target_link_libraries(serv_fork_per PRIVATE library)

# pre-threaded, per-thread accep

add_executable(serv_prethread src/serv_prethread.c src/web.c)

target_link_libraries(serv_prethread PRIVATE library)

include_directories(include)

add_compile_options(-Wall -Wextra -pedantic -ansi)
# Do not worry about the non Posix case in the original build/libasync.mk

set(async_sources
    DiscardFileEventHandler.cpp
    IOLoop.cpp
    IOThread.cpp
    GlobalIOThread.cpp)

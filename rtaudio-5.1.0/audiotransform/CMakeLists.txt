include_directories(..)
if (WIN32)
    include_directories(../include)
endif (WIN32)

list(GET LIB_TARGETS 0 LIBRTAUDIO)

add_executable(duplex duplex.cpp)
target_link_libraries(duplex ${LIBRTAUDIO} ${LINKLIBS})

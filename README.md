# Pinger

Ping library made using C++ 14 which sends ICMP ECHO requests and receives ECHO responses.

- [pinger](./pinger) - C++ 14 library and test file
- [old](./old) - old code written in C during college days (it has its memories :-D)
- [sample file](./pinger/test.cpp) - sample file showing how to use the library

## Building from source
```
git clone https://github.com/kiner-shah/Pinger.git
cd Pinger/pinger
mkdir build
cd build
cmake ..
cmake --build . --config Debug --parallel
```

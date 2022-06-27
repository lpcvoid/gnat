# gnat
A gnat is any of many species of tiny flying insects.

(Reasonably) modern C++ wrapper around mosquitto, a C library for MQTT.

I didn't like mosquittopp very much, so I made my own (very small) wrapper around
mosquitto. Comes with a few tests and works well for me and my very limited usecase.

## Building
In case you want to run tests, please clone this repo with submodules so
you also get `doctest`. Then execute:

```shell
cmake -B build
cmake --build build
```

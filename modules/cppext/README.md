# cpp-extensions
Some extensions to standard library that I have found myself wanting from time to time

### Build
Use dev_build flag if you want to build tests also.
```
cmake -Ddev_build=ON /path/to/cppext/root
```

### Adding to existing project
Add this project as a subdirectory in your projects CMakeLists.txt
```
...
add_subdirectory(/path/to/cppext/root)
...
```
and link your executables/libraries against it
```
...
add_executable(
    some_exe
)
target_link_libraries(
    some_exe
    cppext
)

target_include_directories(
    some_exe
    PUBLIC ${cppext_INCLUDE_DIRS}
)
...
```
or just copy the headers you need and mix with your other stuff.

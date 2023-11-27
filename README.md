# Messenger
A cross-platform desktop client-server messenger has been implemented.

![example_4](https://github.com/smirnovlad/Messenger/assets/86618271/13ce5b3c-4cb6-4587-bdea-9c0c3bc50343)

## Current developments
- User authorization and registration
- Message history storage
- Multi-device login with message synchronization
- Token-based user authentication
- Handling server crashes
- Synchronous message editing

## How to build
The Conan package manager and CMake are used for building the application.

To build the project, it is recommended to install Conan version 1.59.0:
```
pip3 install conan==1.59.0
```

Next, you need to execute the following commands from the project directory:
### Server
```
cd Server/
rm -rf build
mkdir build && cd build/
conan install .. --build=missing -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True
cmake ..
make
```

### Client
```
cd Client/
rm -rf build
mkdir build && cd build/
conan install .. --build=missing -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True
cmake ..
make
```

## How to run
To launch the application, you need to execute the following commands from the project directory:
### Server
```
cd Server/build/bin/
./Server
```

### Client
```
cd Client/build/bin/
./Client
```

## Implementation details
- [Documentation](documentation.docx)
- [Behavioral diagram of the application](UML/model.png)
- [Class diagram of the client-side of the application](UML/uml-Client.png)
- [Class diagram of the server-side of the application](UML/uml-Server.png)

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)


project("NetLib")

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto
        PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libcrypto.a)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl
        PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libssl.a)

add_library(curl STATIC IMPORTED)
set_target_properties(curl
        PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libcurl.a)

## 找到 Android 系统自带的 libz
#find_library(z-lib z)

add_library(
        ${CMAKE_PROJECT_NAME}
        SHARED
        main.cpp
        utils/net_task.cpp
        utils/crypto_utils.cpp
)

include_directories(
        utils
        include
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        android
        z
        curl
        ssl
        crypto
        log
)
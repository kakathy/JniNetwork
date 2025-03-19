# JNI 网络访问库 / JNI Network Access Library

## 简介 / Introduction

本项目是一个基于 **JNI (Java Native Interface)** 的网络访问库，专为 **ARM64** 和 **ARM32** 平台优化，提供高效、稳定的网络请求能力，并内置 **JSON 解析** 功能。

This project is a **JNI (Java Native Interface)** based network access library, optimized for **ARM64** and **ARM32** platforms. It provides efficient and stable network request capabilities and includes **JSON parsing** functionality.

## 特性 / Features

- **多平台支持 / Cross-platform support**：兼容 ARM64 和 ARM32 架构 / Compatible with ARM64 and ARM32 architectures.
- **高效网络请求 / Efficient network requests**：基于 `libcurl` 进行 HTTP/HTTPS 通信 / Uses `libcurl` for HTTP/HTTPS communication.
- **JSON 解析 / JSON Parsing**：集成 `nlohmann/json` 库，提供快速解析 JSON 数据的能力 / Integrates `nlohmann/json` for fast JSON data parsing.
- **易于集成 / Easy integration**：提供 JNI 接口，便于 Android 端调用 / Provides JNI interface for easy Android integration.
- **稳定性 / Stability**：优化内存管理，支持多线程请求 / Optimized memory management with multi-threading support.

## 依赖 / Dependencies

- `libcurl` （用于网络请求 / For network requests）
- `openssl` （用于安全通信 / For secure communication）
- `nlohmann/json` （用于 JSON 解析 / For JSON parsing）

## 使用 / Usage

### 1. 封装 NetTask 请求 / Encapsulating NetTask Request

- **支持自行封装 / Supports custom encapsulation**
- **JSON 解析库 / JSON Parsing Library**
- **AES 解密 / AES Decryption**

### 2. 请求示例 / Request Example

```cpp
string url = "http://t.weather.itboy.net/api/weather/city/101010100";
NetTask task;
task.SetUrl(url.c_str());
task.SetConnectTimeout(5);
task.DoGetString();
task.WaitTaskDone();
string result = task.GetResultString();
```

### 3. JSON 解析示例 / JSON Parsing Example

```cpp
const char *c_str = jsonstr.c_str();
json j = json::parse(c_str, nullptr, false, true);
json firstItem = j["message"];
```

### 4. AES 解密示例 / AES Decryption Example

```cpp
result = CryptoUtils::decryptAES(CryptoUtils::base64Decode(jsonResult), 
                                  your_key, 
                                  your_iv);
```

## 贡献 / Contribution

欢迎提交 Issue 和 Pull Request，一起优化该项目！

Contributions are welcome! Feel free to submit Issues and Pull Requests to improve this project.

## 许可证 / License

MIT License


//
// Created by LeonAsha on 2023/11/10.
//

#include <jni.h>
#include <string>

#include <thread>
#include <chrono>
#include <sys/wait.h>
#include "net_task.h"
#include "crypto_utils.cpp"
#include <json.hpp>
#include <android/log.h>

#define LOG_TAG "NDK_NETWORK"
//调试日志 0关闭 1打开
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using json = nlohmann::json;

using namespace std;

void parse(string &jsonstr) {
    try {
        const char *c_str = jsonstr.c_str();
        json j = json::parse(c_str, nullptr, false, true);
        json firstItem = j["message"];
        // 打印第一条数据
        LOGD("第一条数据: %s", firstItem.dump().c_str());
    } catch (const nlohmann::json::exception &e) {
        LOGD("json err");
    }
}

extern "C"
JNIEXPORT jstring JNICALL//无返回值
Java_com_leon_jninetdemo_MainActivity_getNetDataFromJNI(JNIEnv *env, jobject thiz) {
    string url = "http://t.weather.itboy.net/api/weather/city/101010100";
    NetTask task;
    task.SetUrl(url.c_str());
    task.SetConnectTimeout(5);
    task.DoGetString();
    task.WaitTaskDone();
    string result = task.GetResultString();
    parse(result);//json parse
    return env->NewStringUTF(result.c_str());
}


jint JNI_OnLoad(JavaVM *vm, void *reserved __unused) {

    JNIEnv *env = nullptr;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

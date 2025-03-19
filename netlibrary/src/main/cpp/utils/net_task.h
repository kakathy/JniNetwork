#ifndef NET_ASYNC_TASK
#define NET_ASYNC_TASK

#include <string>
#include "curl/curl.h"

/**
 * @brief Network buffer structure for storing HTTP response data
 */
typedef struct _net_buffer_t {
    char* buf;
    // Buffer original size
    int   size;
    // Length of data that was written to buffer
    int   datalen;
} net_buffer_t;

/**
 * @brief Callback function type for network operation completion
 * @param result Result code (0 for success, non-zero for error)
 * @param data Response data or file path
 * @param userdata User data pointer passed to SetCallback
 */
typedef void (*on_net_done_t)(int result, const std::string& data, void* userdata);

/**
 * @brief Network task class for handling HTTP requests using libcurl
 */
class NetTask
{
public:
    NetTask();
    ~NetTask();

    /**
     * @brief Set the URL for the HTTP request
     * @param url URL to request
     */
    void SetUrl(const char* url);

    /**
     * @brief Set the proxy for the HTTP request
     * @param proxy URL to request
     */
    void SetProxy(const char *proxy);

    /**
     * @brief Set callback function to be called when the request is complete
     * @param cb Callback function
     * @param para User data to pass to the callback
     */
    void SetCallback(on_net_done_t cb, void* para = nullptr);

    /**
     * @brief Add a string parameter to a POST request
     * @param item_name Parameter name
     * @param item_data Parameter value
     */
    void AddPostString(const char* item_name, const char* item_data);
    
    /**
     * @brief Add an image file to a POST request
     * @param item_name Parameter name
     * @param file_path Path to the image file
     * @param file_name Optional custom filename (defaults to basename of file_path)
     */
    void AddPostPicture(const char* item_name, const char* file_path, const char* file_name = nullptr);

    /**
     * @brief Configure the request to download response as a string
     * @return 0 on success, non-zero on error
     * @note Must call this or DoGetFile() before WaitTaskDone()
     */
    int DoGetString();

    /**
     * @brief Configure the request to download response as a file
     * @param range Optional byte range to download (e.g. "0-255")
     * @return 0 on success, non-zero on error
     * @note Must call this or DoGetString() before WaitTaskDone()
     */
    int DoGetFile(const char* range = nullptr);
    
    /**
     * @brief Execute the request and wait for it to complete
     * @return Result code (0 for success, non-zero for error)
     */
    int WaitTaskDone();

    /**
     * @brief Get the response string after a successful DoGetString request
     * @return Response string or nullptr if not available
     */
    const char* GetResultString() const;

    /**
     * @brief Get the downloaded file path after a successful DoGetFile request
     * @return File path or nullptr if not available
     */
    const char* GetFilePath() const;

    /**
     * @brief Set connection timeout
     * @param timeo Timeout in seconds
     */
    void SetConnectTimeout(int timeo);

private:
    void _init();
    void _add_post_file(const char* item_name, const char* file_path, const char* file_name, const char* content_type);
    int _on_work_done(int result);

    CURL* m_curl;
    
    int m_is_getfile;
    bool m_do_func_called;
    
    // For POST requests
    struct curl_httppost* m_formpost;
    struct curl_httppost* m_lastptr;
    struct curl_slist* m_headerlist;

    on_net_done_t m_cb;
    void* m_cb_para;

    // For file downloads
    FILE* m_file;
    std::string m_filepath;

    int m_net_result;
    std::string m_url;
    _net_buffer_t m_net_buf;

private:
    // Prevent copying
    NetTask(const NetTask&) = delete;
    NetTask& operator=(const NetTask&) = delete;


};
#endif

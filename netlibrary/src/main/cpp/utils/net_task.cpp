#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "net_task.h"
#include "curl/curl.h"

/**
 * @brief Get file size in bytes
 * @param fpath Path to the file
 * @return File size in bytes, or -1 on error
 */
static long get_filesize(const char *fpath) {
    if (!fpath) return -1;

    struct stat file_stat;
    if (stat(fpath, &file_stat) != 0) {
        return -1;
    }

    return file_stat.st_size;
}

/**
 * @brief Callback function for writing data to memory
 */
static size_t net_write_mem(void *ptr, size_t size, size_t nmemb, void *data) {
    // Ignore data if data pointer is null
    if (!data) return size * nmemb;

    net_buffer_t *buf = static_cast<net_buffer_t *>(data);

    // Calculate total bytes to copy
    size_t total_size = size * nmemb;
    size_t left_size = buf->size - buf->datalen;

    // Prevent buffer overflow
    size_t copy_size = (total_size > left_size) ? left_size : total_size;

    // Copy data to buffer
    if (copy_size > 0) {
        memcpy(buf->buf + buf->datalen, ptr, copy_size);
        buf->datalen += copy_size;
    }

    // Always return the full size to avoid curl error
    return total_size;
}

/**
 * @brief Callback function for writing data to file
 */
static size_t net_write_file(void *ptr, size_t size, size_t nmemb, void *stream) {
    if (!stream) return 0;
    return fwrite(ptr, size, nmemb, static_cast<FILE *>(stream));
}

/**
 * @brief Debug callback for curl verbose output
 */
static int debug_callback(CURL *handle,
                          curl_infotype type,
                          char *data,
                          size_t size,
                          void *userptr) {
    if (type == CURLINFO_TEXT && data) {
        printf("curl debug: %.*s", static_cast<int>(size), data);
    }
    return 0;
}

NetTask::NetTask()
        : m_curl(nullptr), m_is_getfile(0), m_do_func_called(false), m_formpost(nullptr),
          m_lastptr(nullptr), m_headerlist(nullptr), m_cb(nullptr), m_cb_para(nullptr),
          m_file(nullptr), m_net_result(0) {
    memset(&m_net_buf, 0, sizeof(m_net_buf));
    _init();
}

NetTask::~NetTask() {
    // Free memory buffer if allocated
    if (m_net_buf.buf) {
        free(m_net_buf.buf);
        m_net_buf.buf = nullptr;
    }

    // Close file if open
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
}

void NetTask::SetUrl(const char *url) {
    if (!url || !m_curl) return;

    curl_easy_setopt(m_curl, CURLOPT_URL, url);
    m_url = url;
}

void NetTask::SetProxy(const char *proxy) {
    if (!proxy || !m_curl) return;
    curl_easy_setopt(m_curl, CURLOPT_PROXY, proxy);
}

void NetTask::SetCallback(on_net_done_t cb, void *para) {
    m_cb = cb;
    m_cb_para = para;
}

void NetTask::_init() {
    // Initialize curl
    m_curl = curl_easy_init();
    if (!m_curl) return;

    // Configure curl options
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, "/tmp/ca.pem");
    curl_easy_setopt(m_curl, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 60 * 72L);

    // Uncomment to enable verbose debugging
    // curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
    // curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, debug_callback);
}

void NetTask::_add_post_file(const char *item_name, const char *file_path, const char *file_name,
                             const char *content_type) {
    if (!item_name || !file_path || !file_name || !content_type) return;

    curl_formadd(&m_formpost,
                 &m_lastptr,
                 CURLFORM_COPYNAME, item_name,
                 CURLFORM_FILE, file_path,
                 CURLFORM_FILENAME, file_name,
                 CURLFORM_CONTENTTYPE, content_type,
                 CURLFORM_END);
}

void NetTask::AddPostString(const char *item_name, const char *item_data) {
    if (!item_name || !item_data) return;

    curl_formadd(&m_formpost,
                 &m_lastptr,
                 CURLFORM_COPYNAME, item_name,
                 CURLFORM_COPYCONTENTS, item_data,
                 CURLFORM_END);
}

void NetTask::AddPostPicture(const char *item_name, const char *file_path, const char *file_name) {
    if (!item_name || !file_path) return;

    // Check if file exists
    if (access(file_path, F_OK) != 0) {
        return;
    }

    // If file_name not provided, extract from file_path
    const char *actual_file_name = file_name;
    if (!actual_file_name) {
        actual_file_name = strrchr(file_path, '/');

        if (!actual_file_name) {
            actual_file_name = file_path;
        } else {
            actual_file_name += 1;
        }
    }

    _add_post_file(item_name, file_path, actual_file_name, "image/jpeg");
}

int NetTask::DoGetString() {
    if (!m_curl) return -1;

    // Set POST data if available
    if (m_formpost) {
        curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, m_formpost);
    }

    // Configure to write to memory
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, net_write_mem);
    curl_easy_setopt(m_curl, CURLOPT_WRITEHEADER, nullptr);

    // Allocate memory buffer
    const size_t buffer_size = 200 * 1024; // 200KB
    m_net_buf.size = buffer_size;
    m_net_buf.buf = static_cast<char *>(malloc(buffer_size));
    if (!m_net_buf.buf) {
        return -1; // Memory allocation failed
    }

    m_net_buf.datalen = 0;
    memset(m_net_buf.buf, 0, m_net_buf.size);

    // Set data destination
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_net_buf);

    m_do_func_called = true;
    m_is_getfile = 0;

    return 0;
}

int NetTask::DoGetFile(const char *range) {
    if (!m_curl) return -1;

    // Set POST data if available
    if (m_formpost) {
        curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, m_formpost);
    }

    // Set range if provided
    if (range) {
        curl_easy_setopt(m_curl, CURLOPT_RANGE, range);
    }

    // Create temporary file
    char fpath[128];
    strcpy(fpath, "/tmp/XXXXXX");

    int fd = mkstemp(fpath);
    if (fd < 0) {
        return -1;
    }

    // Set close-on-exec flag
    fcntl(fd, F_SETFD, FD_CLOEXEC);

    // Open file for writing
    m_file = fdopen(fd, "wb");
    if (!m_file) {
        close(fd);
        unlink(fpath);
        return -1;
    }

    m_is_getfile = 1;
    m_filepath = fpath;

    // Configure to write to file
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, net_write_file);
    curl_easy_setopt(m_curl, CURLOPT_WRITEHEADER, nullptr);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, m_file);
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);

    m_do_func_called = true;

    return 0;
}

int NetTask::WaitTaskDone() {
    if (!m_curl || !m_do_func_called) {
        return -1;
    }

    // Perform the request
    CURLcode res = curl_easy_perform(m_curl);

    // Process results
    res = static_cast<CURLcode>(_on_work_done(static_cast<int>(res)));

    // Clean up
    if (m_formpost) {
        curl_formfree(m_formpost);
        m_formpost = nullptr;
        m_lastptr = nullptr;
    }

    if (m_headerlist) {
        curl_slist_free_all(m_headerlist);
        m_headerlist = nullptr;
    }

    curl_easy_cleanup(m_curl);
    m_curl = nullptr;

    return static_cast<int>(res);
}

int NetTask::_on_work_done(int result) {
    // Handle string response
    if (m_is_getfile == 0 && m_net_buf.buf) {
        if (m_net_buf.datalen > 0) {
            // Ensure null-termination
            size_t null_pos = (m_net_buf.datalen < m_net_buf.size) ?
                              m_net_buf.datalen : m_net_buf.size - 1;
            m_net_buf.buf[null_pos] = '\0';

            // Call callback if set
            if (m_cb) {
                std::string get_string = m_net_buf.buf;
                m_cb(result, get_string, m_cb_para);
            }
        }
    }
        // Handle file response
    else if (m_is_getfile) {
        double filesize = 0;
        curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);

        // Close file if open
        if (m_file) {
            fclose(m_file);
            m_file = nullptr;
        }

        // Delete file on error
        if (result != 0) {
            unlink(m_filepath.c_str());
        }
            // Verify file size
        else {
            long actual_size = get_filesize(m_filepath.c_str());
            if (filesize > 0 && actual_size != static_cast<long>(filesize)) {
                unlink(m_filepath.c_str());
                result = -1;
            }
        }

        // Call callback if set
        if (m_cb) {
            m_cb(result, m_filepath, m_cb_para);
        }
    }

    return result;
}

const char *NetTask::GetResultString() const {
    if (m_is_getfile == 0 && m_net_buf.buf) {
        return m_net_buf.buf;
    }
    return nullptr;
}

const char *NetTask::GetFilePath() const {
    if (m_is_getfile == 1) {
        return m_filepath.c_str();
    }
    return nullptr;
}

void NetTask::SetConnectTimeout(int timeo) {
    if (m_curl && timeo > 0) {
        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, static_cast<long>(timeo));
    }
}

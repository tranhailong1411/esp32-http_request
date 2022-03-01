#include "http_server_app.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"    

#include <esp_http_server.h>
static httpd_handle_t server = NULL;

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "example";
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    httpd_req_recv(req, buf,req->content_len);
    printf("Data:%s\n",buf);
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST ,
    .handler   = echo_post_handler ,
    .user_ctx  = NULL
};
static esp_err_t sw1_post_handler(httpd_req_t *req)
{
    char buf[100];
    httpd_req_recv(req, buf,req->content_len);
    printf("Data:%s\n",buf);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t sw1_post_data = {
    .uri       = "/switch1",
    .method    = HTTP_POST ,
    .handler   = sw1_post_handler ,
    .user_ctx  = NULL
};

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req,(const char *) index_html_start, index_html_end-index_html_start);
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};
/* An HTTP GET handler */
static esp_err_t hello_get_data_handler(httpd_req_t *req)
{
    const char* resp_str= (const char*) "{\"temperature\": \"27.1\", \"humidity\": \"80\"}";
    // httpd_resp_send(req,(const char *) index_html_start, index_html_end-index_html_start);
    httpd_resp_send(req, resp_str ,strlen(resp_str));
    return ESP_OK;
}

static const httpd_uri_t get_data_hello = {
    .uri       = "/getdatahello",
    .method    = HTTP_GET,
    .handler   = hello_get_data_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


/* An HTTP POST handler */
// static esp_err_t echo_post_handler(httpd_req_t *req)
// {
//     char buf[100];
//     int ret, remaining = req->content_len;

//     while (remaining > 0) {
//         /* Read the data for the request */
//         if ((ret = httpd_req_recv(req, buf,
//                         MIN(remaining, sizeof(buf)))) <= 0) {
//             if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//                 /* Retry receiving if timeout occurred */
//                 continue;
//             }
//             return ESP_FAIL;
//         }

//         /* Send back the same data */
//         httpd_resp_send_chunk(req, buf, ret);
//         remaining -= ret;

//         /* Log data received */
//         ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
//         ESP_LOGI(TAG, "%.*s", ret, buf);
//         ESP_LOGI(TAG, "====================================");
//     }

//     // End response
//     httpd_resp_send_chunk(req, NULL, 0);
//     return ESP_OK;
// }

// static const httpd_uri_t echo = {
//     .uri       = "/echo",
//     .method    = HTTP_POST,
//     .handler   = echo_post_handler,
//     .user_ctx  = NULL
// };


esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

void start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &get_data_hello);
        httpd_register_uri_handler(server, &sw1_post_data);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND,http_404_error_handler);
        httpd_register_uri_handler(server, &echo);
        //httpd_register_uri_handler(server, &echo);
   
    }
    else{
        ESP_LOGI(TAG, "Error starting server!");
    }
}
void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}


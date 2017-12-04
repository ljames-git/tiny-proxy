#include "curl/curl.h"
#include "TcpServer.h"
#include "HttpServer.h"

int main(int argc, char ** argv)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.163.com");
        /* example.com is redirected, so we tell libcurl to follow redirection */ 
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    return 0;


    /*
    //CTcpServer *s = new CTcpServer(9898);
    CTcpServer *s = new CHttpServer(8888);
    //s->get_model()->set_timeout(1000);

    if (s->start() == 0)
        s->get_model()->start();
    return 0;
    */
}

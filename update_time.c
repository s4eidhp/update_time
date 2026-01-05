#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

int main() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() failed\n");
        free(chunk.memory);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.keybit.ir/time/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(chunk.memory);
        return 1;
    }

    curl_easy_cleanup(curl);

    // Parse JSON for unix.en
    char *unix_start = strstr(chunk.memory, "\"unix\":");
    if (!unix_start) {
        fprintf(stderr, "Unix not found in response\n");
        free(chunk.memory);
        return 1;
    }

    char *unix_en = strstr(unix_start, "\"en\":");
    if (!unix_en) {
        fprintf(stderr, "Unix en not found\n");
        free(chunk.memory);
        return 1;
    }
    unix_en += 5; // skip "en":

    char *unix_end = strchr(unix_en, ',');
    if (!unix_end) unix_end = strchr(unix_en, '\n');
    if (!unix_end) unix_end = strchr(unix_en, '}');
    if (!unix_end) {
        fprintf(stderr, "Invalid unix format\n");
        free(chunk.memory);
        return 1;
    }
    *unix_end = '\0';

    long long unix_time_ll = atoll(unix_en);
    time_t unix_time = (time_t)unix_time_ll;

    // Set system time
    struct timeval tv;
    tv.tv_sec = unix_time;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) < 0) {
        perror("settimeofday");
        free(chunk.memory);
        return 1;
    }

    printf("System time updated successfully.\n");
    free(chunk.memory);
    return 0;
}
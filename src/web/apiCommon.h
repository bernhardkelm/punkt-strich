#pragma once

#include <ArduinoJson.h>

#include <cstdint>

class AsyncWebServerRequest;

/** Response type every API route answers with, the settings page is the only exception. */
constexpr const char* kJsonContentType = "application/json";

/** Cap for a JSON body, the handler allocates the announced Content-Length before reading it. */
constexpr int kMaxJsonBodyBytes = 512;

/**
 * Logs one API call in the punkt format.
 * @param {const char*} method - HTTP method of the request
 * @param {const char*} path - requested path
 * @param {const char*} detail - what the call carried, empty string when there is nothing to add
 * @returns {void}
 */
void logApi(const char* method, const char* path, const char* detail);

/** Error answered when loop() has not drained the command queue fast enough. */
constexpr const char* kQueueFullError = "busy, retry";

/**
 * Answers a request with a JSON error body, and logs the call with the message.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {const char*} message - reason, must not contain quotes or backslashes
 * @param {uint16_t} status - HTTP status, 400 for a rejected body
 * @returns {void}
 */
void sendApiError(AsyncWebServerRequest* request, const char* message, uint16_t status = 400);

/**
 * Answers a request with a 200 and a serialised JSON body, and logs the call.
 * @param {AsyncWebServerRequest*} request - request to answer, method and path are read from it
 * @param {const JsonDocument&} doc - body to serialise
 * @param {const char*} detail - what to log, null logs the body itself
 * @returns {void}
 */
void sendApiJson(AsyncWebServerRequest* request, const JsonDocument& doc,
                 const char* detail = nullptr);

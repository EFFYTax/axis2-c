/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <axis2_http_request_line.h>
#include <axutil_string.h>
#include <string.h>
#include <axis2_http_transport.h>
#include <stdio.h>

struct axis2_http_request_line
{
    axis2_char_t *http_version;
    axis2_char_t *method;
    axis2_char_t *uri;
};

AXIS2_EXTERN axis2_http_request_line_t *AXIS2_CALL
axis2_http_request_line_create(
    const axutil_env_t * env,
    const axis2_char_t * method,
    const axis2_char_t * uri,
    const axis2_char_t * http_version)
{
    axis2_http_request_line_t *request_line = NULL;

    AXIS2_PARAM_CHECK(env->error, method, NULL);
    AXIS2_PARAM_CHECK(env->error, uri, NULL);
    AXIS2_PARAM_CHECK(env->error, http_version, NULL);

    request_line = (axis2_http_request_line_t *)AXIS2_MALLOC(env->allocator,
        sizeof(axis2_http_request_line_t));

    if(!request_line)
    {
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }
    memset((void *)request_line, 0, sizeof(axis2_http_request_line_t));
    request_line->method = (axis2_char_t *)axutil_strdup(env, method);
    request_line->uri = (axis2_char_t *)axutil_strdup(env, uri);
    request_line->http_version = (axis2_char_t *)axutil_strdup(env, http_version);

    return request_line;
}

void AXIS2_CALL
axis2_http_request_line_free(
    axis2_http_request_line_t * request_line,
    const axutil_env_t * env)
{

    if(!request_line)
    {
        return;
    }

    if(request_line->method)
    {
        AXIS2_FREE(env->allocator, request_line->method);
    }
    if(request_line->uri)
    {
        AXIS2_FREE(env->allocator, request_line->uri);
    }
    if(request_line->http_version)
    {
        AXIS2_FREE(env->allocator, request_line->http_version);
    }

    AXIS2_FREE(env->allocator, request_line);
    return;
}

/**
 * Parses a line "<http method> <uri location> <http version>CRLF" and creates http_request_line
 * object. E.g "POST /axis2/services/echo HTTP/1.1\r\n"
 * @param env pointer to environment struct
 * @param str pointer to the line to be parsed
 * @return created object if success. NULL otherwise
 */
AXIS2_EXTERN axis2_http_request_line_t *AXIS2_CALL
axis2_http_request_line_parse_line(
    const axutil_env_t * env,
    const axis2_char_t * str)
{
    /*axis2_char_t *req_line = NULL;*/
    axis2_char_t *method = NULL;
    axis2_char_t *uri = NULL;
    axis2_char_t *http_version = NULL;
    axis2_http_request_line_t *ret = NULL;
    axis2_char_t *tmp = NULL;
    int i = 0;

    if(!str)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Invalid parameter is given to parse");
        return NULL;
    }

    tmp = axutil_strstr(str, AXIS2_HTTP_CRLF);
    if(!tmp)
    {
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_INVALID_HTTP_HEADER_START_LINE, AXIS2_FAILURE);
        return NULL;
    }

    i = (int)(tmp - str); /* We are sure that the difference lies within the int range */

    /*
    req_line = AXIS2_MALLOC(env->allocator, i * sizeof(axis2_char_t) + 1);
    if(!req_line)
    {
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }
    memcpy(req_line, str, i * sizeof(axis2_char_t));
    req_line[i] = AXIS2_ESC_NULL;
    tmp = req_line;
    */
    /* we don't need to do a malloc and memcpy, because this method is only used by
     * axis2_simple_http_svr_conn_read_request and it passes a temporary string. So, we can just
     * set it as req_line and set the escape characters. this is done after 1.6.0 and if no issues
     * found until 1.8.0, above code comment can be removed after 1.8.0
     */

    tmp = (axis2_char_t *)str;
    tmp[i] = AXIS2_ESC_NULL;

    /* find http method */
    method = tmp;
    tmp = strchr(tmp, AXIS2_SPACE);
    if(!tmp)
    {
        /*AXIS2_FREE(env->allocator, req_line);*/
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_INVALID_HTTP_HEADER_START_LINE, AXIS2_FAILURE);
        return NULL;
    }
    *tmp++ = AXIS2_ESC_NULL;

    /* find URI */
    uri = tmp;
    tmp = strrchr(tmp, AXIS2_SPACE);
    if(!tmp)
    {
        /*AXIS2_FREE(env->allocator, req_line);*/
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_INVALID_HTTP_HEADER_START_LINE, AXIS2_FAILURE);
        return NULL;
    }
    *tmp++ = AXIS2_ESC_NULL;

    /* find HTTP version */
    http_version = tmp;
    ret = axis2_http_request_line_create(env, method, uri, http_version);
    /*AXIS2_FREE(env->allocator, req_line);*/

    return ret;
}

axis2_char_t *AXIS2_CALL
axis2_http_request_line_get_method(
    const axis2_http_request_line_t * request_line,
    const axutil_env_t * env)
{
    return request_line->method;
}

axis2_char_t *AXIS2_CALL
axis2_http_request_line_get_http_version(
    const axis2_http_request_line_t * request_line,
    const axutil_env_t * env)
{
    return request_line->http_version;
}

axis2_char_t *AXIS2_CALL
axis2_http_request_line_get_uri(
    const axis2_http_request_line_t * request_line,
    const axutil_env_t * env)
{
    return request_line->uri;
}

axis2_char_t *AXIS2_CALL
axis2_http_request_line_to_string(
    axis2_http_request_line_t * request_line,
    const axutil_env_t * env)
{
    int alloc_len = 0;
    axis2_char_t *ret = NULL;

    alloc_len = axutil_strlen(request_line->method) + axutil_strlen(request_line->uri)
        + axutil_strlen(request_line->http_version) + 6;
    /* 5 = 2 * spaces + '/' +CR + LF + '\0' */

    ret = AXIS2_MALLOC(env->allocator, alloc_len * sizeof(axis2_char_t));
    if(!ret)
    {
        AXIS2_HANDLE_ERROR(env, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }

    if(request_line->uri[0] != AXIS2_F_SLASH)
    {
        sprintf(ret, "%s /%s %s%s", request_line->method, request_line->uri,
            request_line->http_version, AXIS2_HTTP_CRLF);
    }
    else
    {
        sprintf(ret, "%s %s %s%s", request_line->method, request_line->uri,
            request_line->http_version, AXIS2_HTTP_CRLF);
    }
    return ret;
}

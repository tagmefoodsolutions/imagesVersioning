#pragma once

extern "C" {
#include <ngx_http.h>
}

#include <weserv/utils/status.h>

namespace weserv::nginx {

ngx_int_t ngx_weserv_return_error(ngx_http_request_t *r,
                                  api::utils::Status status, ngx_chain_t *out);

}  // namespace weserv::nginx


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_ffm_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_ffm_create_conf(ngx_conf_t *cf);
static char *ngx_ffm_init_conf(ngx_conf_t *cf, void *conf);

static ngx_int_t ngx_http_ffm_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_ffm_init(ngx_conf_t *cf);


typedef struct {
    ngx_flag_t   enable;
    ngx_int_t  (*java_handler)(ngx_http_request_t *r);
} ngx_ffm_conf_t;


// One Global method reference. Must be assign before module call.
ngx_int_t  (*ngx_http_ffm_upcall)(ngx_http_request_t *r);

static ngx_command_t  ngx_ffm_commands[] = {

    { ngx_string("ffm"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_CONF_1MORE,
      ngx_ffm_enable,
      0,
      0,
      NULL },

      ngx_null_command
};

/*
static ngx_core_module_t  ngx_ffm_module_ctx = {
    ngx_string("ffm"),
    ngx_ffm_create_conf,
    ngx_ffm_init_conf
};
*/

static ngx_http_module_t  ngx_http_ffm_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_ffm_init,                  /* postconfiguration */

    ngx_ffm_create_conf,                                  /* create main configuration */
    ngx_ffm_init_conf,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};

ngx_module_t  ngx_ffm_module = {
    NGX_MODULE_V1,
    &ngx_http_ffm_module_ctx,                /* module context */
    ngx_ffm_commands,                   /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
ngx_ffm_enable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value = cf->args->elts;

    if (value == NULL) {
        return NGX_CONF_OK;
    }

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "(ngx_ffm_enable) with value %s", value[1].data);

    return NGX_CONF_OK;
}

static void *
ngx_ffm_create_conf(ngx_conf_t *cf)
{
    ngx_ffm_conf_t  *fcf;

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "(ngx_ffm_create_conf)");

    fcf = ngx_pcalloc(cf->pool, sizeof(ngx_ffm_conf_t));
    if (fcf == NULL) {
        return NULL;
    }

    fcf->enable = NGX_CONF_UNSET;

    return fcf;
}


static char *
ngx_ffm_init_conf(ngx_conf_t *cf, void *conf)
{
    ngx_ffm_conf_t *fcf = conf;

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "(ngx_ffm_init_conf)");

    ngx_conf_init_value(fcf->enable, 0);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_ffm_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_ffm_handler;

    return NGX_OK;
}

static ngx_int_t 
ngx_http_ffm_handler(ngx_http_request_t *r)
{
	ngx_log_t                 *log = r->connection->log;
	//ngx_chain_t                out;
	ngx_int_t                  rc;
	//ngx_str_t                  content = ngx_string("Hello world !");

	if (ngx_http_ffm_upcall == NULL) {
		ngx_log_error(NGX_LOG_NOTICE, log, 0, "(ngx_http_ffm_handler) No upcall.");
		// return NGX_HTTP_INTERNAL_SERVER_ERROR;
        return NGX_DECLINED;
	}
	
	ngx_log_error(NGX_LOG_INFO, log, 0, "(ngx_http_ffm_handler) signature is (%l).", r->signature);

    r->headers_out.last_modified_time = ngx_time();

	rc = ngx_http_ffm_upcall(r);
    if (rc < NGX_OK) {
    	ngx_log_error(NGX_LOG_NOTICE, log, 0, "(ngx_http_ffm_handler) Unable to ngx_http_ffm_upcall: (%l).", rc);
        return rc;
    }

	// rc = ngx_http_discard_request_body(r);
    // if (rc != NGX_OK) {
    // 	ngx_log_error(NGX_LOG_NOTICE, log, 0, "(ngx_http_ffm_handler) Unable to ngx_http_discard_request_body.");
    //     return rc;
    // }

    // r->headers_out.status = NGX_HTTP_OK;

	// ngx_str_set(&r->headers_out.content_type, "text/html");
    // r->headers_out.content_length_n = content.len;

    // rc = ngx_http_send_header(r);
	// if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
	// 	ngx_log_error(NGX_LOG_NOTICE, log, 0, "(ngx_http_ffm_handler) Unable to ngx_http_send_header.");
	// 	return rc;
    // }
    // ngx_log_error(NGX_LOG_INFO, log, 0, "(ngx_http_ffm_handler) ngx_http_send_header returns (%l).", rc);


	// out.buf = ngx_create_temp_buf(r->pool, content.len);
	// out.buf->last = ngx_cpymem( out.buf->last, content.data, content.len );
    // out.next = NULL;
    // out.buf->last_buf = 1;
    // out.buf->last_in_chain = 1;

	// rc = ngx_http_output_filter(r, &out);
	// ngx_log_error(NGX_LOG_INFO, log, 0, "(ngx_http_ffm_handler) ngx_http_output_filter returns (%l).", rc);

	return rc;
}

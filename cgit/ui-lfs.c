/* ui-lfs.c: functions for Git LFS Batch API support (download only)
 *
 * Copyright (C) JiJi <i@mmdjiji.com>
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include "cgit.h"
#include "ui-lfs.h"
#include "html.h"
#include "ui-shared.h"

#define LFS_MIME "application/vnd.git-lfs+json"
#define MAX_LFS_BODY 65536
#define OID_HEX_LEN 64

/*
 * Validate that a string is exactly a 64-character lowercase hex SHA-256.
 */
static int is_valid_lfs_oid(const char *s, size_t len)
{
	size_t i;

	if (len != OID_HEX_LEN)
		return 0;
	for (i = 0; i < len; i++) {
		if (!((s[i] >= '0' && s[i] <= '9') ||
		      (s[i] >= 'a' && s[i] <= 'f')))
			return 0;
	}
	return 1;
}

/*
 * Extract a JSON string value for the given key from a JSON fragment.
 * Writes into buf (up to buflen-1 chars) and returns buf, or NULL if not found.
 * Only handles simple cases: "key": "value"
 */
static const char *json_get_string(const char *json, const char *key,
				   char *buf, size_t buflen)
{
	char needle[256];
	const char *p, *start;
	size_t i;

	snprintf(needle, sizeof(needle), "\"%s\"", key);
	p = strstr(json, needle);
	if (!p)
		return NULL;
	p += strlen(needle);

	/* skip whitespace and colon */
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ':')
		p++;

	if (*p != '"')
		return NULL;
	p++; /* skip opening quote */

	start = p;
	for (i = 0; *p && *p != '"' && i < buflen - 1; p++, i++)
		buf[i] = *p;
	buf[i] = '\0';
	return buf;
}

/*
 * Extract a JSON number value for the given key.
 * Returns the number, or -1 if not found.
 */
static long long json_get_number(const char *json, const char *key)
{
	char needle[256];
	const char *p;

	snprintf(needle, sizeof(needle), "\"%s\"", key);
	p = strstr(json, needle);
	if (!p)
		return -1;
	p += strlen(needle);

	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ':')
		p++;

	return strtoll(p, NULL, 10);
}

/*
 * Build the filesystem path for an LFS object.
 * Returns a malloc'd string: <repo_path>/lfs/objects/<oid[0:2]>/<oid[2:4]>/<oid>
 */
static char *lfs_object_path(const char *oid)
{
	struct strbuf path = STRBUF_INIT;

	strbuf_addstr(&path, ctx.repo->path);
	strbuf_addstr(&path, "/lfs/objects/");
	strbuf_add(&path, oid, 2);
	strbuf_addch(&path, '/');
	strbuf_add(&path, oid + 2, 2);
	strbuf_addch(&path, '/');
	strbuf_addstr(&path, oid);

	return strbuf_detach(&path, NULL);
}

/*
 * Construct the base URL from CGI environment variables.
 */
static void get_base_url(struct strbuf *buf)
{
	const char *scheme, *host;

	scheme = getenv("HTTP_X_FORWARDED_PROTO");
	if (!scheme) {
		if (ctx.env.https && !strcmp(ctx.env.https, "on"))
			scheme = "https";
		else
			scheme = "http";
	}

	host = getenv("HTTP_X_FORWARDED_HOST");
	if (!host)
		host = ctx.env.http_host;
	if (!host)
		host = ctx.env.server_name;
	if (!host)
		host = "localhost";

	strbuf_addf(buf, "%s://%s", scheme, host);
}

static void lfs_send_error(int code, const char *statusmsg, const char *msg)
{
	ctx.page.status = code;
	ctx.page.statusmsg = statusmsg;
	ctx.page.mimetype = LFS_MIME;
	cgit_print_http_headers();
	htmlf("{\"message\":\"%s\"}", msg);
}

/*
 * Handle POST /<repo>/info/lfs/objects/batch
 */
static void handle_lfs_batch(void)
{
	char body[MAX_LFS_BODY];
	ssize_t len;
	char operation[32];
	const char *arr_start, *obj_start, *obj_end;
	struct strbuf base_url = STRBUF_INIT;
	int first_object;

	if (!ctx.env.request_method || strcmp(ctx.env.request_method, "POST")) {
		lfs_send_error(405, "Method Not Allowed", "Only POST is supported");
		return;
	}

	/* Read POST body */
	len = ctx.env.content_length;
	if (len <= 0 || (size_t)len >= sizeof(body)) {
		lfs_send_error(400, "Bad Request", "Invalid request body");
		return;
	}
	len = read(STDIN_FILENO, body, len);
	if (len <= 0) {
		lfs_send_error(400, "Bad Request", "Could not read request body");
		return;
	}
	body[len] = '\0';

	/* Extract operation */
	if (!json_get_string(body, "operation", operation, sizeof(operation))) {
		lfs_send_error(400, "Bad Request", "Missing operation field");
		return;
	}

	if (!strcmp(operation, "upload")) {
		lfs_send_error(403, "Forbidden", "Upload not supported (read-only)");
		return;
	}

	if (strcmp(operation, "download")) {
		lfs_send_error(400, "Bad Request", "Unknown operation");
		return;
	}

	get_base_url(&base_url);

	/* Send response header */
	ctx.page.status = 200;
	ctx.page.statusmsg = "OK";
	ctx.page.mimetype = LFS_MIME;
	cgit_print_http_headers();

	html("{\"transfer\":\"basic\",\"objects\":[");

	/* Iterate through objects array */
	arr_start = strstr(body, "\"objects\"");
	if (!arr_start)
		arr_start = body;
	arr_start = strchr(arr_start, '[');
	if (!arr_start) {
		html("]}");
		strbuf_release(&base_url);
		return;
	}

	first_object = 1;
	obj_start = arr_start + 1;

	while ((obj_start = strchr(obj_start, '{'))) {
		char oid[OID_HEX_LEN + 1];
		long long size;
		char *file_path;
		struct stat st;

		obj_end = strchr(obj_start, '}');
		if (!obj_end)
			break;

		/* Extract oid and size from this object block */
		{
			/* Work on a temporary copy of the object block */
			size_t block_len = obj_end - obj_start + 1;
			char *block = xmemdupz(obj_start, block_len);

			if (!json_get_string(block, "oid", oid, sizeof(oid))) {
				free(block);
				obj_start = obj_end + 1;
				continue;
			}
			size = json_get_number(block, "size");
			free(block);
		}

		if (!first_object)
			html(",");
		first_object = 0;

		if (!is_valid_lfs_oid(oid, strlen(oid))) {
			htmlf("{\"oid\":\"%s\",\"size\":%lld,"
			      "\"error\":{\"code\":422,"
			      "\"message\":\"Invalid OID\"}}",
			      oid, size);
			obj_start = obj_end + 1;
			continue;
		}

		file_path = lfs_object_path(oid);

		if (stat(file_path, &st)) {
			htmlf("{\"oid\":\"%s\",\"size\":%lld,"
			      "\"error\":{\"code\":404,"
			      "\"message\":\"Object not found\"}}",
			      oid, size);
		} else {
			htmlf("{\"oid\":\"%s\",\"size\":%lld,"
			      "\"authenticated\":true,"
			      "\"actions\":{\"download\":{"
			      "\"href\":\"%s/%s/info/lfs/objects/%s\"}}}",
			      oid, size,
			      base_url.buf, ctx.qry.repo, oid);
		}

		free(file_path);
		obj_start = obj_end + 1;
	}

	html("]}");
	strbuf_release(&base_url);
}

/*
 * Handle GET /<repo>/info/lfs/objects/<oid>
 * Serves the LFS object file directly.
 */
static void handle_lfs_download(const char *oid)
{
	char *file_path;
	struct stat st;

	if (!is_valid_lfs_oid(oid, strlen(oid))) {
		lfs_send_error(400, "Bad Request", "Invalid OID format");
		return;
	}

	file_path = lfs_object_path(oid);

	if (stat(file_path, &st)) {
		free(file_path);
		lfs_send_error(404, "Not Found", "Object not found");
		return;
	}

	ctx.page.mimetype = "application/octet-stream";
	ctx.page.size = st.st_size;
	cgit_print_http_headers();
	html_include(file_path);

	free(file_path);
}

/*
 * Main LFS request dispatcher.
 * Called from cgit_clone_info() when ctx.qry.path starts with "lfs/".
 */
void cgit_lfs_handle(void)
{
	const char *path = ctx.qry.path;

	if (!strcmp(path, "lfs/objects/batch")) {
		handle_lfs_batch();
	} else if (!strncmp(path, "lfs/objects/", 12) && path[12]) {
		handle_lfs_download(path + 12);
	} else {
		lfs_send_error(404, "Not Found", "Invalid LFS request");
	}
}

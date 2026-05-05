#!/bin/sh

set -e

# Check for incomming Nginx server commands or subcommands only
if [ "$1" = "nginx" ] || [ "${1#-}" != "$1" ]; then
    if [ "${1#-}" != "$1" ]; then
        set -- nginx "$@"
    fi

    chown nginx:nginx /var/cache/cgit
    chmod u+g /var/cache/cgit

    git config --system --add safe.directory '*'

    export HOME=/tmp

    # Basic auth: generate htpasswd and nginx snippet if credentials are set
    rm -f /etc/nginx/conf.d/auth.inc
    if [ -n "$BASIC_AUTH_USER" ] && [ -n "$BASIC_AUTH_PASS" ]; then
        htpasswd -cb /etc/nginx/.htpasswd "$BASIC_AUTH_USER" "$BASIC_AUTH_PASS"
        printf 'auth_basic "Restricted";\nauth_basic_user_file /etc/nginx/.htpasswd;\n' \
            > /etc/nginx/conf.d/auth.inc
    fi

    # Replace environment variables only if `USE_CUSTOM_CONFIG` is not defined or equal to `false`
    if [[ -z "$USE_CUSTOM_CONFIG" ]] || [[ "$USE_CUSTOM_CONFIG" = "false" ]]; then
        envsubst < /tmp/cgitrc.tmpl > /etc/cgitrc
        envsubst '${CGIT_THEME}' < /tmp/head-include.tmpl > /usr/share/webapps/cgit/head-include.html

        # Enable gravatar if requested
        if [[ "$ENABLE_GRAVATAR" = "1" ]] || [[ "$ENABLE_GRAVATAR" = "true" ]]; then
            echo "email-filter=exec:/usr/lib/cgit/filters/email-gravatar.py" >> /etc/cgitrc
        fi
    fi

    spawn-fcgi \
        -u nginx -g nginx \
        -s /var/run/fcgiwrap.sock \
        -n -- /usr/bin/fcgiwrap \
        & exec "$@"
else
    exec "$@"
fi

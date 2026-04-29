## Build stage: compile cgit from local source
FROM alpine:3.23 AS builder

RUN apk add --no-cache \
        build-base \
        curl \
        linux-headers \
        lua5.3-dev \
        openssl-dev \
        xz \
        zlib-dev

COPY cgit /src/cgit
WORKDIR /src/cgit

RUN make get-git
RUN make -j$(nproc) \
        prefix=/usr \
        CGIT_SCRIPT_PATH=/usr/share/webapps/cgit \
        CGIT_DATA_PATH=/usr/share/webapps/cgit \
        NO_GETTEXT=YesPlease \
        NO_REGEX=NeedsStartEnd \
        LUA_PKGCONFIG=lua5.3
RUN make install \
        prefix=/usr \
        CGIT_SCRIPT_PATH=/usr/share/webapps/cgit \
        CGIT_DATA_PATH=/usr/share/webapps/cgit \
        NO_GETTEXT=YesPlease \
        NO_REGEX=NeedsStartEnd \
        LUA_PKGCONFIG=lua5.3

## Runtime stage
FROM nginx:1.28.1-alpine3.23

ARG VERSION=0.0.0
ENV VERSION=${VERSION}

# CGit default options
ENV CGIT_TITLE="CGit"
ENV CGIT_DESC="The hyperfast web frontend for Git repositories"
ENV CGIT_VROOT="/"
ENV CGIT_SECTION_FROM_STARTPATH=0
ENV CGIT_MAX_REPO_COUNT=50

LABEL version="${VERSION}" \
    description="The hyperfast web frontend for Git repositories"

COPY --from=builder /usr/share/webapps/cgit /usr/share/webapps/cgit
COPY --from=builder /usr/lib/cgit/filters /usr/lib/cgit/filters

RUN set -eux \
    && apk add --no-cache \
        apache2-utils \
        ca-certificates \
        fcgiwrap \
        git \
        git-daemon \
        lua5.3-libs \
        mailcap \
        py3-markdown \
        py3-pygments \
        py3-docutils \
        groff \
        python3 \
        spawn-fcgi \
        tzdata \
        xz \
        zlib \
    && rm -rf /var/cache/apk/* \
    && rm -rf /tmp/*

COPY cgit.conf /tmp/cgitrc.tmpl
COPY docker-entrypoint.sh /
COPY nginx/nginx.conf /etc/nginx/nginx.conf
COPY nginx/conf.d/default.conf /etc/nginx/conf.d/default.conf

RUN set -eux \
    && mkdir -p /var/cache/cgit \
    && mkdir -p /srv/git

RUN set -eux \
    && nginx -c /etc/nginx/nginx.conf -t

ENTRYPOINT [ "/docker-entrypoint.sh" ]

EXPOSE 80

STOPSIGNAL SIGQUIT

CMD [ "nginx", "-g", "daemon off;" ]

# ---------------------------------------------------------------------
# Common base (not built alone)
# ---------------------------------------------------------------------
FROM nginx AS base
# Install the packages needed in both variants
RUN apt update && apt install -y spawn-fcgi libfcgi-dev &&\
	rm -rf /var/lib/apt/lists

# Copy your pre‑built backend binary and configuration
COPY ./backend.fcgi /usr/local/bin/
COPY ./nginx /etc/nginx/conf.d
COPY ./maze.html /usr/share/nginx/html/

# ---------------------------------------------------------------------
# Production image (default target)
# ---------------------------------------------------------------------
FROM base AS release
CMD ["sh", "-c", "nginx -g 'daemon off;' & spawn-fcgi -n -p 8080 ./usr/local/bin/backend.fcgi"]

# ---------------------------------------------------------------------
# Debug image
# ---------------------------------------------------------------------
FROM base AS debug
RUN apt update && apt install -y valgrind && \
	rm -rf /var/lib/apt/lists

CMD ["sh", "-c", "nginx -g 'daemon off;' & spawn-fcgi -n -p 8080 -- /usr/bin/valgrind --trace-children=yes --track-fds=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --tool=memcheck -s ./usr/local/bin/backend.fcgi"]

# deploy statically built backend in slim alpine container
FROM alpine:latest AS minimal
RUN apk --no-cache add spawn-fcgi

COPY ./backend.fcgi /usr/local/bin/
CMD ["sh", "-c", "spawn-fcgi -n -p 8080 ./usr/local/bin/backend.fcgi"]

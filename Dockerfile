FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:latest

# Install lame, mpg123 and their dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libmpg123-dev \
    lame \
    build-essential \
    libmp3lame-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /pg_service_template

FROM gcc:latest

RUN apt-get update && apt-get install -y cmake

WORKDIR /tmp/nauty

COPY src/nauty2_9_3.tar.gz .

RUN tar -xzf nauty2_9_3.tar.gz --strip-components=1 \
    && ./configure \
    && make \
    && cp geng /usr/local/bin/ \
    && rm -rf /tmp/nauty

WORKDIR /app

CMD ["sh", "-c", "mkdir -p src/build && g++ -std=c++20 src/main.cpp -o src/build/main && ./src/build/main"]
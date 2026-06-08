FROM gcc:latest

RUN apt-get update && apt-get install -y cmake libboost-dev

WORKDIR /app

CMD mkdir -p src/nauty/unpacked && \
    cd src/nauty/unpacked && \
    if [ ! -f nauty.h ]; then \
        tar -xzf ../nauty2_9_3.tar.gz --strip-components=1; \
    fi && \
    if [ ! -x geng ]; then \
        ./configure && make; \
    fi && \
    cp geng /usr/local/bin/ && \
    cd /app && \
    mkdir -p src/build && \
    g++ -std=c++20 src/main.cpp -o src/build/main && \
    ./src/build/main  
    #&& \
    #tail -f /dev/null
# uncomment to enter docker container
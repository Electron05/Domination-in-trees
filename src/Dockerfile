FROM gcc:latest

RUN apt-get update && apt-get install -y cmake libboost-dev

WORKDIR /app

CMD mkdir -p src/input/nauty2_9_3 && \
    cd src/input/nauty2_9_3 && \
    if [ ! -f nauty.h ]; then \
        tar -m -xzf ../nauty*.tar.gz --strip-components=1; \
    fi && \
    if [ ! -x geng ]; then \
        ./configure && make; \
    fi && \
    cp geng /usr/local/bin/ && \
    cd /app/src/input && \
    chmod +x generate_small_trees.sh generate_large_trees.sh && \
    bash ./generate_small_trees.sh && \
    bash ./generate_large_trees.sh && \
    cd /app && \
    mkdir -p src/build && \
    g++ -std=c++20 -O3 src/main.cpp -o src/build/main && \
    ./src/build/main
    #&& \
    #tail -f /dev/null
# uncomment to enter docker container
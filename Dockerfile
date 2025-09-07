FROM doner357/cpp-conan-base:latest-noble-c18g14cn2

WORKDIR /work
RUN sudo apt update
ENV REBUILD_TRIGGER="0"
RUN git clone https://github.com/ssanin82/htask.git
WORKDIR htask
RUN conan install . --output-folder=build --build=missing
RUN cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --parallel

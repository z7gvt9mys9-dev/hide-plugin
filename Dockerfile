FROM --platform=linux/amd64 registry.gitlab.steamos.cloud/steamrt/sniper/sdk

WORKDIR /app

RUN git clone --depth 1 https://github.com/alliedmodders/ambuild && \
    cd ambuild && python setup.py install && cd ..
RUN git clone --depth 1 https://github.com/alliedmodders/metamod-source
RUN git clone --depth 1 https://github.com/alliedmodders/hl2sdk-manifests
RUN git clone --depth 1 --branch cs2 https://github.com/alliedmodders/hl2sdk hl2sdk-cs2
RUN git config --global --add safe.directory '*'

RUN echo "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-21 main" >> /etc/apt/sources.list.d/llvm.list && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt update && apt install -y clang-21 && \
    ln -sf /usr/bin/clang-21 /usr/bin/clang && ln -sf /usr/bin/clang++-21 /usr/bin/clang++

ENV HL2SDKCS2=/app/hl2sdk-cs2
ENV MMSOURCE_DEV=/app/metamod-source
ENV HL2SDKMANIFESTS=/app/hl2sdk-manifests
WORKDIR /app/source
CMD ["/bin/bash", "-c", "mkdir -p dockerbuild && cd dockerbuild && python ../configure.py --enable-optimize --sdks cs2 && ambuild"]

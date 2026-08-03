FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-riscv64-unknown-elf \
    qemu-system-misc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /beetleos
COPY . .

CMD ["make", "run"]

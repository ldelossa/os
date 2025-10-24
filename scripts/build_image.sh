#!/bin/bash

set -e
set -o pipefail

kernel_size=$(stat -c %s ./bin/kernel.bin)
echo "kernel size is ${kernel_size} bytes"

kernel_sectors=$(((kernel_size + 511) / 512))
echo "kernel size is ${kernel_sectors} sectors"

cat ./bin/boot.bin ./bin/kernel.bin > ./bin/os-image.bin

# format string for creating raw 32-bit value (little-endian)
kernel_sectors_bytes=$(printf '\\x%02x\\x%02x\\x%02x\\x%02x' \
    $((kernel_sectors & 0xFF)) \
    $(((kernel_sectors >> 8) & 0xFF)) \
    $(((kernel_sectors >> 16) & 0xFF)) \
    $(((kernel_sectors >> 24) & 0xFF)))

# perform the patch (write 4 bytes for 32-bit value)
printf %b "$kernel_sectors_bytes" |
	dd of=./bin/os-image.bin bs=1 seek=2 count=4 conv=notrunc

image_size=$(stat -c %s ./bin/os-image.bin)
# if image size is not a multiple of 512, pad it, so its a valid disk image
if (( image_size % 512 != 0 )); then
	padding_size=$((512 - (image_size % 512)))
	echo "padding disk image by ${padding_size} bytes"
	dd if=/dev/zero bs=1 count=${padding_size} >> ./bin/os-image.bin
fi


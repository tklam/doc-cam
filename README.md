# Doc Cam
An application to identify a document in a picture, and then adjust the perspective to up-right.

## TODO
- Improve algos
- Port to android

## Docker image for compiling the source

To use the docker image for compiling the course code, simply:

```bash
# Build the image
make build-docker-img-linux

# Then run a container with the built image
docker run --rm -it -v `pwd`:/workspace  doc-cam-builder:1.0 /bin/bash
```

Build the project inside the container:

```bash
cd /workspace
mkdir -p build
cd build
cmake .. && make -j $(nproc)
```


For more, please refer to [Docker/Readme](Docker/Readme.md)

## Copyright issues
- All the images files in the data folder were downloaded from the internet. They are not mine.

SHELL := /bin/bash

.PHONY: build-docker-img-linux
build-docker-img-linux: Docker/Dockerfile.dev
	docker build \
		--build-arg UID=$(shell bash -c 'id -u') \
		--build-arg GID=$(shell bash -c 'id -g') \
		--build-arg UNAME=$(shell bash -c 'whoami') \
		-f Docker/Dockerfile.dev -t doc-cam-builder:1.0 .

.PHONY: build-docker-img-mac
build-docker-img-mac: Docker/Dockerfile.dev
	docker build \
		-f Docker/Dockerfile.dev -t doc-cam-builder:1.0 .
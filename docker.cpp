/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#include "utils/property.h"
#include "docker.h"
#include <curl/curl.h>
#include <iostream>
#include <utility>
#include "container_manager.h"
#include "network_manager.h"
#include "image_manager.h"
#include <utils/curl.h>

DockerClient::DockerClient(DockerConfig config) {
	this->config = std::move(config);
	this->dockerApiUrl =
			this->protocol + "://" + this->config.dockerHost + ":" + std::to_string(this->config.dockerPort);

	if (!ping()) {
		std::cerr << "Docker is not running or not reachable at " << dockerApiUrl << std::endl;
		std::cerr << "Please ensure Docker is running and accessible at the configured address." << std::endl;
		throw std::runtime_error("Docker connectivity failed: Unable to reach Docker daemon at " + dockerApiUrl + 
			". Please verify that Docker is running and the host/port configuration is correct.");
	}
}

std::unique_ptr<ContainerManager> DockerClient::containers() {
	return std::make_unique<ContainerManager>(std::shared_ptr<DockerClient>(this, [](DockerClient*){}));
}

std::unique_ptr<NetworkManager> DockerClient::networks() {
	return std::make_unique<NetworkManager>(std::shared_ptr<DockerClient>(this, [](DockerClient*){}));
}

std::unique_ptr<ImageManager> DockerClient::images() {
	return std::make_unique<ImageManager>(std::shared_ptr<DockerClient>(this, [](DockerClient*){}));
}

bool DockerClient::ping() {
	std::string url = dockerApiUrl + "/_ping";

	ReqUEST request(url, {});

	std::shared_ptr<CurlResponse> response = request.setMethod(method::HttpMethod::_GET).execute();

	if (!response) {
		std::cerr << "Failed to execute request." << std::endl;
		return false;
	}

	if (response->status_code == 200) {
		return true;
	} else {
		std::cerr << "Failed to ping Docker: " << response->error_message << std::endl;
		return false;
	}
}
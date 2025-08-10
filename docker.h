/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once
#include <string>
#include <memory>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "container_manager.h"
#include "network_manager.h"
#include "image_manager.h"

struct DockerConfig {
	std::string dockerHost = "localhost"; // Docker host address
	int dockerPort = 2375; // Docker port number
	std::string dockerApiVersion; // Docker API version
	std::string dockerTlsVerify; // Docker TLS verification setting
	std::string dockerCertPath; // Path to Docker TLS certificates

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DockerConfig, dockerHost, dockerPort, dockerApiVersion, dockerTlsVerify, dockerCertPath);
};

class DockerClient {
private:
	DockerConfig config;
	std::string dockerApiUrl;
	std::string protocol = "http";

public:

	explicit DockerClient(DockerConfig config);

	std::unique_ptr<ContainerManager> containers();
	std::unique_ptr<NetworkManager> networks();
	std::unique_ptr<ImageManager> images();

	~DockerClient() = default;

	[[nodiscard]] std::string getDockerApiUrl() const {
		return dockerApiUrl;
	}
	bool ping();
};

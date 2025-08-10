/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include "vector"
#include "container.h"
#include "map"
#include <nlohmann/json.hpp>
#include "types/container_types.h"

class DockerClient;

struct ContainerList {
	std::string id;
		std::string name;
		std::string image;
		std::string imageId;
		std::string command;
		containerTypes::ContainerStatus state;
		std::string status;
		long created;
		std::map<std::string, std::string> labels;

		// Ports
		struct Port {
				int privatePort;
				int publicPort;
				std::string ip;
				std::string type;
		};
		std::vector<Port> ports;

		// Network settings
		struct NetworkInfo {
				std::string ipAddress;
				std::string gateway;
				std::string macAddress;
		};
		std::map<std::string, NetworkInfo> networks;

		// Mounts
		struct Mount {
				std::string source;
				std::string destination;
				std::string type;
				bool readWrite;
		};
		std::vector<Mount> mounts;
};

class ContainerManager {
private:
		std::shared_ptr<DockerClient> dockerClient;
public:
		explicit ContainerManager(std::shared_ptr<DockerClient> client);
		
		bool create(const std::string &name, const nlohmann::json &config);

		bool exists(const std::string &containerName);

		nlohmann::json inspect(const std::string &id = "");

		// Function to remove a container
		void remove(const std::string &containerName);

		std::vector<ContainerList> list(bool all = false, int limit = 0, bool size = false,
																const std::string &filters = "");
};
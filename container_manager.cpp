/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#ifdef _WIN32

#include <windows.h>

#ifdef byte
#undef byte
#endif
#endif

#include "docker_compose_parser.h"
#include "container_manager.h"
#include "utils/curl.h"
#include "container.h"
#include "container.h"
#include "docker.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include <fmt/format.h>

using namespace nlohmann;

ContainerManager::ContainerManager(std::shared_ptr<DockerClient> dockerClient) : dockerClient(dockerClient) {
	if (!dockerClient) {
		throw std::runtime_error("Docker client is not initialized.");
	}
}

nlohmann::json ContainerManager::inspect(const std::string &id) {
	if (id.empty()) {
		throw std::runtime_error("Container ID cannot be empty");
	}

	ReqUEST request(fmt::format("{}/containers/{}/json", dockerClient->getDockerApiUrl(), id),
									std::vector<CurlParameter>{});

	std::shared_ptr<CurlResponse> response = request.setMethod(method::HttpMethod::_GET).execute();

	if (!response) {
		throw std::runtime_error("Failed to execute inspect request for container: " + id);
	}

	if (response->status_code != 200) {
		throw std::runtime_error("Failed to inspect container " + id + ": HTTP " + std::to_string(response->status_code));
	}

	try {
		return nlohmann::json::parse(response->body);
	} catch (const nlohmann::json::exception &e) {
		throw std::runtime_error("Failed to parse inspect response for container " + id + ": " + e.what());
	}
}

bool ContainerManager::exists(const std::string &containerName) {
	if (containerName.empty()) {
		throw std::runtime_error("Container name cannot be empty");
	}

	try {
		nlohmann::json response = this->inspect(containerName);

		if (response.is_null()) {
			std::cerr << "Container " << containerName << " does not exist." << std::endl;
			return false;
		}

		if (response.contains("Id") && !response["Id"].is_null()) {
			std::string id = response["Id"].get<std::string>();
			std::cout << "Container " << containerName << " exists with ID: " << id << std::endl;
			return true;
		} else {
			std::cerr << "Container " << containerName << " exists but does not have a valid ID." << std::endl;
			return false;
		}
	} catch (const std::exception &e) {
		// Container doesn't exist or other error
		std::cerr << "Container " << containerName << " does not exist or error occurred: " << e.what() << std::endl;
		return false;
	}
}

bool ContainerManager::create(const std::string &name, const nlohmann::json &config) {
	ReqUEST request(fmt::format("{}/containers/create?name={}", dockerClient->getDockerApiUrl(), name), std::vector<CurlParameter>{});
	request.setMethod(method::HttpMethod::_POST);
	request.setHeader("Content-Type: application/json");
	
	request.setBody(config.dump());

	std::shared_ptr<CurlResponse> response = request.execute();
	if (!response) {
		std::cerr << "Failed to execute request." << std::endl;
		return false;
	}
	if (response->status_code != 201) {
		std::cerr << "Error creating container: HTTP " << response->status_code << std::endl;
		std::cerr << "Response: " << response->body << std::endl;
		return false;
	}

	try{
		auto json_response = nlohmann::json::parse(response->body);
		if (json_response.contains("Id") && !json_response["Id"].is_null()) {
			std::string containerId = json_response["Id"].get<std::string>();
			std::cout << "Container created successfully with ID: " << containerId << std::endl;
			return true;
		} else {
			std::cerr << "Container creation response does not contain an ID." << std::endl;
			return false;
		}
	} catch (const std::exception &e) {
		std::cerr << "Exception while creating container: " << e.what() << std::endl;
		return false;
	}
}

void ContainerManager::remove(const std::string &containerName) {

}

/*
 * https://docs.docker.com/reference/api/engine/version/v1.50/#tag/Container/operation/ContainerList
 * */

std::vector<ContainerList> ContainerManager::list(bool all, int limit, bool size, const std::string &filters) {
	ReqUEST request(fmt::format(
											"{}/containers/json?all={}&limit={}&size={}&filters={}",
											dockerClient->getDockerApiUrl(),
											all,
											limit,
											size,
											filters.empty() ? "{}" : filters
									),
									std::vector<CurlParameter>{}
	);

	std::shared_ptr<CurlResponse> response = request.setMethod(method::HttpMethod::_GET).execute();

	if (!response) {
		std::cerr << "Failed to execute request." << std::endl;
		return std::vector<ContainerList>{};
	}

	// response_data contient la réponse JSON
	std::string response_data = response->body;

	try {
		auto json_response = nlohmann::json::parse(response_data);

		std::vector<ContainerList> containers_list;
		containers_list.reserve(json_response.size()); // Optimisation

		for (const auto &item: json_response) {
			try {
				ContainerList container;
				container.id = item.contains("Id") && !item["Id"].is_null() ? 
					item["Id"].get<std::string>() : "";
				
				// Gestion correcte du nom des containers
				if (item.contains("Names") && item["Names"].is_array() && !item["Names"].empty()) {
					std::string fullName = item["Names"][0].get<std::string>();
					container.name = fullName.starts_with("/") ? fullName.substr(1) : fullName;
				} else {
					container.name = "";
				}
				
				container.image = item.contains("Image") && !item["Image"].is_null() ? 
					item["Image"].get<std::string>() : "";
				container.imageId = item.contains("ImageID") && !item["ImageID"].is_null() ? 
					item["ImageID"].get<std::string>() : "";
				container.command = item.contains("Command") && !item["Command"].is_null() ? 
					item["Command"].get<std::string>() : "";
				
				// Gestion robuste du state
				if (item.contains("State") && !item["State"].is_null()) {
					container.state = stringToState(item["State"].get<std::string>());
				} else {
					container.state = containerTypes::ContainerStatus::UNKNOWN;
				}
				
				container.status = item.contains("Status") && !item["Status"].is_null() ? 
					item["Status"].get<std::string>() : "";
				container.created = item.contains("Created") && !item["Created"].is_null() ? 
					item["Created"].get<long>() : 0L;
				
				// Labels
				if (item.contains("Labels") && item["Labels"].is_object()) {
					for (const auto& [key, value] : item["Labels"].items()) {
						if (!value.is_null() && value.is_string()) {
							container.labels[key] = value.get<std::string>();
						}
					}
				}

				// Ports - gestion robuste
				if (item.contains("Ports") && item["Ports"].is_array()) {
					for (const auto &port : item["Ports"]) {
						ContainerList::Port p;
						p.privatePort = port.contains("PrivatePort") && !port["PrivatePort"].is_null() ? 
							port["PrivatePort"].get<int>() : 0;
						p.publicPort = port.contains("PublicPort") && !port["PublicPort"].is_null() ? 
							port["PublicPort"].get<int>() : 0;
						p.ip = port.contains("IP") && !port["IP"].is_null() ? 
							port["IP"].get<std::string>() : "";
						p.type = port.contains("Type") && !port["Type"].is_null() ? 
							port["Type"].get<std::string>() : "";
						container.ports.push_back(p);
					}
				}

			   // Network settings
			   const auto networks = item.value("Networks", nlohmann::json{});
			   for (const auto &network : networks.items()) {
				   ContainerList::NetworkInfo net_info;
				   net_info.ipAddress = network.value().value("IPAddress", "");
				   net_info.gateway = network.value().value("Gateway", "");
				   net_info.macAddress = network.value().value("MacAddress", "");
				   container.networks[network.key()] = net_info;
			   }

				// Mounts
				for (const auto &mount : item.value("Mounts", std::vector<nlohmann::json>{})) {
					ContainerList::Mount m;
					m.source = mount.value("Source", "");
					m.destination = mount.value("Destination", "");
					m.type = mount.value("Type", "");
					m.readWrite = mount.value("RW", false);
					container.mounts.push_back(m);
				}

				containers_list.push_back(container);
			} catch (const std::exception &e) {
				std::cerr << "Error creating container object: " << e.what() << std::endl;
				// Continue avec les autres containers
			}
		}

		return containers_list;
	} catch (const nlohmann::json::exception &e) {
		std::cerr << "JSON parsing error: " << e.what() << std::endl;
		std::cerr << "Raw response: " << response_data << std::endl;
		return std::vector<ContainerList>{};
	}
}
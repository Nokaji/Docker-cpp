/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include <string>
#include <any>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <nlohmann/json.hpp>
#include "types/container_types.h"

containerTypes::ContainerStatus stringToState(const std::string &status);
std::string stateToString(containerTypes::ContainerStatus status);

class DockerClient;

class Container {
private:
	std::shared_ptr<DockerClient> dockerClient;

	std::string appArmorProfile;
	std::vector<std::string> args;
	containerTypes::ContainerConfig config;
	std::string created;
	std::string driver;
	std::vector<std::string> execIDs;
	containerTypes::GraphDriver graphDriver;
	containerTypes::HostConfig hostConfig;
	std::string hostnamePath;
	std::string hostsPath;
	std::string id;
	std::string image;
	containerTypes::ImageManifestDescriptor imageManifestDescriptor;
	std::string logPath;
	std::string mountLabel;
	std::vector<containerTypes::Mount> mounts;
	std::string name;
	containerTypes::NetworkSettings networkSettings;
	std::string path;
	std::string platform;
	std::string processLabel;
	std::string resolvConfPath;
	int restartCount;
	containerTypes::ContainerState state;

	static std::vector<std::string> parseLogsResponse(const std::string &response);
		
public:
		// Constructor pour créer depuis JSON
		explicit Container(std::shared_ptr<DockerClient> dockerClient, const nlohmann::json &containerJson);

		std::vector<std::string> getRecentLogs(int lines = 100);

		void followLogsAsync(std::function<void(const std::string &)> callback, int maxDuration = 0);

		// Getters
		[[nodiscard]] std::string getId() const { return id; }
		[[nodiscard]] std::string getName() const { return name; }
		[[nodiscard]] std::string getImage() const { return image; }
		[[nodiscard]] containerTypes::ContainerState getState() const { return state; }
		[[nodiscard]] std::string getCreated() const { return created; }
		[[nodiscard]] std::string getDriver() const { return driver; }
		[[nodiscard]] int getRestartCount() const { return restartCount; }
		[[nodiscard]] std::string getAppArmorProfile() const { return appArmorProfile; }
		[[nodiscard]] std::string getLogPath() const { return logPath; }
		[[nodiscard]] std::string getMountLabel() const { return mountLabel; }
		[[nodiscard]] std::string getPath() const { return path; }
		[[nodiscard]] std::string getPlatform() const { return platform; }
		[[nodiscard]] std::string getProcessLabel() const { return processLabel; }
		[[nodiscard]] std::string getHostnamePath() const { return hostnamePath; }
		[[nodiscard]] std::string getHostsPath() const { return hostsPath; }
		[[nodiscard]] std::string getResolvConfPath() const { return resolvConfPath; }
		[[nodiscard]] const std::vector<containerTypes::Mount> &getMounts() const { return mounts; }
		[[nodiscard]] const containerTypes::GraphDriver &getGraphDriver() const { return graphDriver; }
		[[nodiscard]] const containerTypes::HostConfig &getHostConfig() const { return hostConfig; }
		[[nodiscard]] const containerTypes::NetworkSettings &getNetworkSettings() const { return networkSettings; }
		[[nodiscard]] const std::vector<std::string> &getArgs() const { return args; }
		[[nodiscard]] const std::vector<std::string> &getExecIDs() const { return execIDs; }
		[[nodiscard]] const containerTypes::ImageManifestDescriptor &getImageManifestDescriptor() const { return imageManifestDescriptor; }
		[[nodiscard]] const containerTypes::ContainerConfig &getConfig() const { return config; }

		// Methods
		void run();

		std::vector<std::string> logs(
				bool follow = false,
				bool capture_stdout = false,
				bool capture_stderr = false,
				long since = 0,
				long until = 0,
				bool timestamps = false,
				const std::string &tail = "all"
		);

		[[nodiscard]] nlohmann::json inspect();

		void stop();

		void restart();

        void refresh();

		void remove();

		std::string to_string();
};
/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#include "image_manager.h"
#include "utils/curl.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>
#include "docker.h"

ImageManager::ImageManager(std::shared_ptr<DockerClient> dockerClient) : dockerClient(dockerClient) {
    if (!this->dockerClient) {
        throw std::runtime_error("Docker client is not initialized.");
    }
}

std::vector<imageTypes::ImageConfig> ImageManager::list(bool all, std::map<std::string, std::string> filters, bool sharedSize, bool digests, bool manifests) {
    ReqUEST request(fmt::format("{}/images/json", dockerClient->getDockerApiUrl()), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    // Build query parameters
    std::vector<CurlParameter> params;
    if (all) {
        params.push_back({"all", "1"});
    }
    if (!filters.empty()) {
        nlohmann::json filterJson = filters;
        params.push_back({"filters", filterJson.dump()});
    }
    if (sharedSize) {
        params.push_back({"shared-size", "1"});
    }
    if (digests) {
        params.push_back({"digests", "1"});
    }
    if (manifests) {
        params.push_back({"manifests", "1"});
    }
    
    request.setParameters(params);

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to list images: " + response->error_message);
    }

    std::vector<imageTypes::ImageConfig> images;
    try {
        auto jsonResponse = nlohmann::json::parse(response->body);
        images.reserve(jsonResponse.size());

        for (const auto& item : jsonResponse) {
            imageTypes::ImageConfig imageConfig;
            imageConfig.id = item.value("Id", "");
            imageConfig.parentId = item.value("ParentId", "");
            imageConfig.created = item.value("Created", 0);
            imageConfig.size = item.value("Size", 0);
            imageConfig.sharedSize = item.value("SharedSize", 0);
            imageConfig.virtualSize = item.value("VirtualSize", 0);
            imageConfig.containers = item.value("Containers", 0);

            if (item.contains("RepoTags") && item["RepoTags"].is_array()) {
                for (const auto& tag : item["RepoTags"]) {
                    if (tag.is_string()) {
                        imageConfig.repoTags.push_back(tag.get<std::string>());
                    }
                }
            }

            if (item.contains("RepoDigests") && item["RepoDigests"].is_array()) {
                for (const auto& digest : item["RepoDigests"]) {
                    if (digest.is_string()) {
                        imageConfig.repoDigests.push_back(digest.get<std::string>());
                    }
                }
            }

            if (item.contains("Labels") && item["Labels"].is_object()) {
                for (const auto& [key, value] : item["Labels"].items()) {
                    if (value.is_string()) {
                        imageConfig.labels[key] = value.get<std::string>();
                    }
                }
            }

            images.push_back(imageConfig);
        }

        return images;

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse image list response: " << e.what() << std::endl;
        return {};
    }
}

std::string ImageManager::pull(const std::string& fromImage, 
                              const std::string& fromSrc, 
                              const std::string& repo,
                              const std::string& tag,
                              const std::string& message,
                              const std::vector<std::string>& changes,
                              const std::string& platform,
                              const std::string& registryAuth) {
    
    ReqUEST request(fmt::format("{}/images/create", dockerClient->getDockerApiUrl()), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_POST);
    
    // Build query parameters
    std::vector<CurlParameter> params;
    if (!fromImage.empty()) {
        params.push_back({"fromImage", fromImage});
    }
    if (!fromSrc.empty()) {
        params.push_back({"fromSrc", fromSrc});
    }
    if (!repo.empty()) {
        params.push_back({"repo", repo});
    }
    if (!tag.empty()) {
        params.push_back({"tag", tag});
    }
    if (!message.empty()) {
        params.push_back({"message", message});
    }
    if (!platform.empty()) {
        params.push_back({"platform", platform});
    }
    
    // Add changes as separate parameters
    for (const auto& change : changes) {
        params.push_back({"changes", change});
    }
    
    request.setParameters(params);
    
    // Add registry auth header if provided
    if (!registryAuth.empty()) {
        request.setHeader("X-Registry-Auth: " + registryAuth);
    }

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response) {
        std::cout << "No response from Docker server" << std::endl;
        throw std::runtime_error("Failed to pull image: No response from Docker daemon");
    }
    
    if (response->status_code != 200) {
        std::cout << fmt::format("HTTP Error: {} - {}", response->status_code, response->error_message) << std::endl;
        throw std::runtime_error("Failed to pull image: " + response->error_message);
    }

    // Docker API returns a stream of JSON objects for pull operations
    // We'll parse each line to show progress
    std::string result;
    std::istringstream stream(response->body);
    std::string line;
    
    std::cout << "📦 Download progress:" << std::endl;
    
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            try {
                auto jsonLine = nlohmann::json::parse(line);
                
                // Display progress information
                if (jsonLine.contains("status")) {
                    std::string status = jsonLine["status"].get<std::string>();
                    std::string progressInfo = "  " + status;
                    
                    if (jsonLine.contains("id")) {
                        std::string id = jsonLine["id"].get<std::string>();
                        progressInfo = fmt::format("  [{}] {}", id, status);
                    }
                    
                    if (jsonLine.contains("progress")) {
                        std::string progress = jsonLine["progress"].get<std::string>();
                        progressInfo += " " + progress;
                    }
                    
                    std::cout << progressInfo << std::endl;
                    result = status;
                }
                
                if (jsonLine.contains("error")) {
                    std::string error = jsonLine["error"].get<std::string>();
                    std::cout << fmt::format("Error: {}", error) << std::endl;
                    throw std::runtime_error("Pull failed: " + error);
                }
            } catch (const nlohmann::json::exception& e) {
                // Continue processing other lines
                continue;
            }
        }
    }
    
    return result.empty() ? "Pull completed" : result;
}

bool ImageManager::exists(const std::string& name) {    
    ReqUEST request(fmt::format("{}/images/{}/json", dockerClient->getDockerApiUrl(), name), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    std::shared_ptr<CurlResponse> response = request.execute();

    bool imageExists = response && response->status_code == 200;
    
    if (imageExists) {
        std::cout << fmt::format("Image '{}' found locally", name) << std::endl;
    } else {
        std::cout << fmt::format("Image '{}' not found (code: {})", name, response ? response->status_code : -1) << std::endl;
    }
    
    return imageExists;
}

std::string ImageManager::download(const std::string& name, const std::string& tag, const std::string& registryAuth)
{
    std::string fullImageName = fmt::format("{}:{}", name, tag);
    
    std::cout << fmt::format("Checking image '{}' existence...", fullImageName) << std::endl;
    
    // Check if the image already exists
    if (exists(fullImageName)) {
        std::cout << fmt::format("Image '{}' already exists locally.", fullImageName) << std::endl;
        return fmt::format("Image '{}' already exists.", fullImageName);
    }

    std::cout << fmt::format("Downloading image '{}'...", fullImageName) << std::endl;
    
    // Pull the image with progress feedback
    try {
        std::string result = pull(fullImageName, "", "", tag, "", {}, "", registryAuth);
        std::cout << fmt::format("Image '{}' downloaded successfully!", fullImageName) << std::endl;
        return result;
    } catch (const std::exception& e) {
        std::cout << fmt::format("Failed to download image '{}': {}", fullImageName, e.what()) << std::endl;
        throw;
    }
}

void ImageManager::remove(const std::string& name, bool force, bool noprune) {
    ReqUEST request(fmt::format("{}/images/{}", dockerClient->getDockerApiUrl(), name), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_DELETE);
    
    std::vector<CurlParameter> params;
    if (force) {
        params.push_back({"force", "1"});
    }
    if (noprune) {
        params.push_back({"noprune", "1"});
    }
    
    if (!params.empty()) {
        request.setParameters(params);
    }

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to remove image: " + response->error_message);
    }
}

nlohmann::json ImageManager::inspect(const std::string& name) {
    ReqUEST request(fmt::format("{}/images/{}/json", dockerClient->getDockerApiUrl(), name), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to inspect image: " + response->error_message);
    }

    try {
        return nlohmann::json::parse(response->body);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse image inspect response: " + std::string(e.what()));
    }
}

nlohmann::json ImageManager::history(const std::string& name) {
    ReqUEST request(fmt::format("{}/images/{}/history", dockerClient->getDockerApiUrl(), name), std::vector<CurlParameter>{});
    request.setMethod(method::HttpMethod::_GET);

    std::shared_ptr<CurlResponse> response = request.execute();

    if (!response || response->status_code != 200) {
        throw std::runtime_error("Failed to get image history: " + response->error_message);
    }

    try {
        return nlohmann::json::parse(response->body);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse image history response: " + std::string(e.what()));
    }
}
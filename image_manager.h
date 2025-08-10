/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once
#include "types/image_types.h"
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

class DockerClient;

class ImageManager
{
    private:
        std::shared_ptr<DockerClient> dockerClient;

    public:
        explicit ImageManager(std::shared_ptr<DockerClient> dockerClient);

        std::vector<imageTypes::ImageConfig> list(bool all = false, std::map<std::string, std::string> filters = {}, bool sharedSize = false, bool digests = false, bool manifests = false);

        bool exists(const std::string& name);

        // Pull or import an image
        std::string pull(const std::string& fromImage, 
                        const std::string& fromSrc = "", 
                        const std::string& repo = "",
                        const std::string& tag = "",
                        const std::string& message = "",
                        const std::vector<std::string>& changes = {},
                        const std::string& platform = "",
                        const std::string& registryAuth = "");
                        
        // Download an image if it does not exist
        std::string download(const std::string& name, 
                             const std::string& tag = "latest",
                             const std::string& registryAuth = "");

        // Remove an image
        void remove(const std::string& name, bool force = false, bool noprune = false);

        // Inspect an image
        nlohmann::json inspect(const std::string& name);
        nlohmann::json history(const std::string& name);

        ~ImageManager() = default;
};
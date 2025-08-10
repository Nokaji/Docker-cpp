/*
 * Copyright (c) 2025 Nokaji. Tous droits réservés.
 * Ce fichier fait partie du projet Kernel-James.
 */
#pragma once

#include <string>
#include <vector>
#include <map>

namespace imageTypes
{

    struct SizeUnPacked
    {
        int64_t unpacked;
    };

    struct SizeObject
    {
        int64_t total;
        int64_t content;
    };

    enum class Kind
    {
        image,
        attestation,
        unknown
    };

    struct OCIPlatform
    {
        std::string architecture;
        std::string os;
        std::string osVersion;
        std::vector<std::string> osFeatures;
        std::string variant;
    };

    struct ImageData
    {
        OCIPlatform platform;
        std::vector<std::string> containers;
        SizeUnPacked size;
    };

    struct AttestationDataObject
    {
        std::string for_;
    };

    struct OCIDescriptor
    {
        std::string mediaType;
        std::string digest;
        int64_t size;
        std::vector<std::string> urls = {};
        std::map<std::string, std::string> annotations = {};
        std::string data = "";
        OCIPlatform platform = {};
        std::string artifactType = "";
    };
    
    struct ImageManifestSummary
    {
        std::string id;
        OCIDescriptor descriptor;
        bool available;
        SizeObject size;
        Kind kind;
        ImageData imageData = {};
        AttestationDataObject attestationData = {};

    };

    struct ImageConfig
    {
        std::string id;
        std::string parentId;
        std::vector<std::string> repoTags;
        std::vector<std::string> repoDigests;
        int created;
        int64_t size;
        int64_t sharedSize;
        int64_t virtualSize;
        std::map<std::string, std::string> labels;
        int containers;
        ImageManifestSummary manifests;
        OCIDescriptor descriptor = {};
    };

    

    
};
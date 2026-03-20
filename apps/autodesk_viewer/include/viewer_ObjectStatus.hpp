#pragma once

#include <string>

namespace autodesk_viewer {
    enum class ObjectStatus {
        UPLOAD_AWAITING,
        UPLOADING,
        UPLOAD_FAILED,
        CONVERSION_AWAITING,
        CONVERTING,
        CONVERSION_FAILED,
        ACTIVE,
        DELETED,
        UNKNOWN
    };

    namespace ObjectStatusUtils {

        inline std::string to_string(ObjectStatus status) {
            switch (status) {
                case ObjectStatus::UPLOAD_AWAITING:     return "UPLOAD_AWAITING";
                case ObjectStatus::UPLOADING:           return "UPLOADING";
                case ObjectStatus::UPLOAD_FAILED:       return "UPLOAD_FAILED";
                case ObjectStatus::CONVERSION_AWAITING: return "CONVERSION_AWAITING";
                case ObjectStatus::CONVERTING:          return "CONVERTING";
                case ObjectStatus::CONVERSION_FAILED:   return "CONVERSION_FAILED";
                case ObjectStatus::ACTIVE:              return "ACTIVE";
                case ObjectStatus::DELETED:             return "DELETED";
                default:                                return "UNKNOWN";
            }
        }

        inline ObjectStatus from_string(const std::string& status_str) {
            if (status_str == "UPLOAD_AWAITING")     return ObjectStatus::UPLOAD_AWAITING;
            if (status_str == "UPLOADING")           return ObjectStatus::UPLOADING;
            if (status_str == "UPLOAD_FAILED")       return ObjectStatus::UPLOAD_FAILED;
            if (status_str == "CONVERSION_AWAITING") return ObjectStatus::CONVERSION_AWAITING;
            if (status_str == "CONVERTING")          return ObjectStatus::CONVERTING;
            if (status_str == "CONVERSION_FAILED")   return ObjectStatus::CONVERSION_FAILED;
            if (status_str == "ACTIVE")              return ObjectStatus::ACTIVE;
            if (status_str == "DELETED")             return ObjectStatus::DELETED;
            return ObjectStatus::UNKNOWN;
        }
    }
}
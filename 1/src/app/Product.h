#pragma once

#include <string>

namespace serein {

struct Product {
    std::string id;
    std::string name;
    std::string status;
    std::string last_update;
    std::string image_path;
    std::string description;
};

}  // namespace serein

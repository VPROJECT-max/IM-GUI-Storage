#include "app/Configuration.h"

namespace serein {

AppConfig make_default_config() {
    AppConfig config;
    config.application_name = "Serein";
    config.version = "0.4.0";
    config.assets.background = "Assets/valley-of-the-five-lakes-first-lake-canada-jasper-national-3840x2160-3887.jpg";
    config.products = {
        {"rust", "Rust", "Undetected", "05-05-2027", "Assets/Loader Games Pictures/RUST.png", "A focused survival experience profile."},
        {"warzone", "Call of Duty: Warzone", "Online", "05-05-2027", "Assets/Loader Games Pictures/CALL OF DUTY WARZONE.jpg", "A competitive deployment profile."},
        {"valorant", "Valorant", "Maintenance", "05-05-2027", "Assets/Loader Games Pictures/VALORANT.jpg", "A precision tactical profile."},
        {"cs2", "Counter-Strike 2", "Online", "05-05-2027", "Assets/Loader Games Pictures/VALVE.png", "A classic competitive profile."},
    };
    return config;
}

}  // namespace serein

json getSleepPreset(json settings) {
    settings["name"] = "sleep";
    settings["power"] = "off";
    return settings;
}
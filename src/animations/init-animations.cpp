
void initAnimations() {
    std::cout << "\n------- Loading Animations -------" << std::endl;
    std::string animationsFolderPath = "./resources/animations";
    for (const auto& entry : std::__fs::filesystem::directory_iterator(animationsFolderPath)) {
        std::string filePath = entry.path().string();
        std::string fileName = entry.path().filename().stem().string();
        std::string fileExtension = entry.path().extension().string();
        Animation animation(fileName, fileExtension);
        animations.push_back(animation);
    }
    std::cout << "----------------------------------\n" << std::endl;
    
}
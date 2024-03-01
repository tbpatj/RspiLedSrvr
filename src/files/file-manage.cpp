#include <fstream>

class FileManage {
    private:
        std::string filename;
        std::string content;
    public:
       FileManage(std::string fName) {
        filename = fName;
       }
       void write(std::string nContent){
            try{
                std::ofstream outputFile(filename, std::ios::out | std::ios::trunc);

                if(outputFile.is_open()){
                    outputFile << nContent;
                    content = nContent;
                    outputFile.close();
                } else {
                    std::cerr << "Error opening the file: " << filename << std::endl;
                }
            }catch(const std::exception& e){
                std::cerr << "Exception: " << e.what() << std::endl;
            }
       }
       std::string read(){
        try{
            std::ifstream inputFile(filename);
            if (inputFile.is_open()) {
                // Read the contents of the file into a string
                std::string fileContents;
                std::getline(inputFile, fileContents);

                // Close the file
                inputFile.close();
                content = fileContents;
                return fileContents;
            } else {
                std::cerr << "Error opening the file." << std::endl;
                return "";
            }
        }catch(const std::exception& e){
            std::cerr << "Exception: " << e.what() << std::endl;
            return "";
        }
       }
};
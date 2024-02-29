json CreateResponse(bool success, std::string message, std::string code, json data){
     json response = {
                {"status", success ? "success" : "error"}, // Add a new key-value pair to the JSON object
                {"message",message}, // Add another key-value pair named "response"
                {"code", code},
                {"data", data} // Remove .dump() to send data as a JSON object
            };
    return response;
}
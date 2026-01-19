#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <map>
#include <memory>
#include <pugixml.hpp>

struct FunctionBlock {
    std::string name;
    std::string type;
    std::map<std::string, std::string> inputs;
    std::map<std::string, std::string> outputs;
};

class CommandHandler {
private:
    std::map<std::string, std::shared_ptr<FunctionBlock>> blocks;
    
    std::string handle_create(const pugi::xml_document& doc, const std::string& request_id);
    std::string handle_write(const pugi::xml_document& doc, const std::string& request_id);
    std::string handle_start(const std::string& request_id);
    std::string handle_query(const std::string& request_id);
    
public:
    CommandHandler();
    ~CommandHandler();
    
    std::string process_command(const std::string& xml_command);
    bool load_boot_file(const std::string& filename);
};

#endif // COMMAND_HANDLER_H
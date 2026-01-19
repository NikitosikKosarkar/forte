#include "command_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>

CommandHandler::CommandHandler() {}

CommandHandler::~CommandHandler() {}

std::string CommandHandler::process_command(const std::string& xml_command) {
    std::string clean_command = xml_command;
    
    clean_command.erase(0, clean_command.find_first_not_of(" \t\n\r"));
    clean_command.erase(clean_command.find_last_not_of(" \t\n\r") + 1);
    
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(clean_command.c_str());
    
    if (!result) {
        std::cerr << "Ошибка парсинга XML: " << result.description() << std::endl;
        return "";
    }
    
    pugi::xml_node root = doc.first_child();
    std::string action = root.attribute("Action").value();
    std::string request_id = root.attribute("ID").value();
    
    std::cout << "Обработка команды: " << action << " (ID: " << request_id << ")" << std::endl;
    
    if (action == "CREATE") {
        return handle_create(doc, request_id);
    } else if (action == "WRITE") {
        return handle_write(doc, request_id);
    } else if (action == "START") {
        return handle_start(request_id);
    } else if (action == "QUERY") {
        return handle_query(request_id);
    }
    
    return "";
}

std::string CommandHandler::handle_create(const pugi::xml_document& doc, const std::string& request_id) {
    pugi::xml_node root = doc.first_child();
    
    pugi::xml_node fb_elem = root.find_child_by_attribute("FB", nullptr);
    if (!fb_elem.empty()) {
        std::string name = fb_elem.attribute("Name").value();
        std::string fb_type = fb_elem.attribute("Type").value();
        
        auto block = std::make_shared<FunctionBlock>();
        block->name = name;
        block->type = fb_type;
        
        blocks[name] = block;
        
        std::cout << "Создан блок: " << name << " типа " << fb_type << std::endl;
        return "";
    }
    
    pugi::xml_node conn_elem = root.find_child_by_attribute("Connection", nullptr);
    if (!conn_elem.empty()) {
        std::string source = conn_elem.attribute("Source").value();
        std::string destination = conn_elem.attribute("Destination").value();
        
        std::cout << "Создана связь: " << source << " -> " << destination << std::endl;
        return "";
    }
    
    return "";
}

std::string CommandHandler::handle_write(const pugi::xml_document& doc, const std::string& request_id) {
    pugi::xml_node root = doc.first_child();
    pugi::xml_node conn_elem = root.find_child_by_attribute("Connection", nullptr);
    
    if (!conn_elem.empty()) {
        std::string source_value = conn_elem.attribute("Source").value();
        std::string destination = conn_elem.attribute("Destination").value();
        
        size_t dot_pos = destination.find('.');
        if (dot_pos != std::string::npos) {
            std::string block_name = destination.substr(0, dot_pos);
            std::string port = destination.substr(dot_pos + 1);
            
            if (blocks.find(block_name) != blocks.end()) {
                blocks[block_name]->inputs[port] = source_value;
                std::cout << "Записано в " << destination << ": " << source_value << std::endl;
            }
        }
        return "";
    }
    
    return "";
}

std::string CommandHandler::handle_start(const std::string& request_id) {
    for (auto& pair : blocks) {
        auto& block = pair.second;
        
        if (block->type == "STRING2STRING") {
            if (block->inputs.find("IN") != block->inputs.end()) {
                std::string input_value = block->inputs["IN"];
                block->outputs["OUT"] = input_value;
                std::cout << block->name << ": " << input_value << " -> OUT" << std::endl;
            }
        } else if (block->type == "OUT_ANY_CONSOLE") {
            if (block->inputs.find("IN") != block->inputs.end()) {
                std::string value = block->inputs["IN"];
                std::string label = block->inputs.count("LABEL") > 0 ? block->inputs["LABEL"] : "";
                std::cout << "CONSOLE [" << label << "]: " << value << std::endl;
            }
        }
    }
    
    return "";
}

std::string CommandHandler::handle_query(const std::string& request_id) {
    std::string block_list;
    for (const auto& pair : blocks) {
        if (!block_list.empty()) {
            block_list += ", ";
        }
        block_list += pair.first;
    }
    
    return "";
}

bool CommandHandler::load_boot_file(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Файл " << filename << " не найден" << std::endl;
        return false;
    }
    
    try {
        std::string line;
        int line_num = 0;
        int commands_processed = 0;
        
        std::cout << "Загрузка конфигурации из " << filename << std::endl;
        
        while (std::getline(file, line)) {
            line_num++;
            
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            
            if (line.empty()) {
                continue;
            }
            
            size_t semicolon_pos = line.find(';');
            
            if (semicolon_pos != std::string::npos) {
                std::string resource_name = line.substr(0, semicolon_pos);
                std::string xml_part = line.substr(semicolon_pos + 1);
                
                resource_name.erase(resource_name.find_last_not_of(" \t\n\r") + 1);
                xml_part.erase(0, xml_part.find_first_not_of(" \t\n\r"));
                xml_part.erase(xml_part.find_last_not_of(" \t\n\r") + 1);
                
                std::cout << "[" << line_num << "] Ресурс: " << resource_name << std::endl;
                
                if (xml_part.empty()) {
                    continue;
                }
                
                try {
                    std::string response = process_command(xml_part);
                    commands_processed++;
                    std::cout << "[" << line_num << "] Обработано: " << response << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "[" << line_num << "] Ошибка обработки: " << e.what() << std::endl;
                }
            } else if (line[0] == '<') {
                try {
                    std::string response = process_command(line);
                    commands_processed++;
                    std::cout << "[" << line_num << "] Обработано (без ресурса): " << response << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "[" << line_num << "] Ошибка парсинга XML: " << e.what() << std::endl;
                }
            } else {
                std::cout << "[" << line_num << "] Непонятный формат: " << line.substr(0, 80) << "..." << std::endl;
            }
        }
        
        std::cout << "Конфигурация загружена. Обработано команд: " << commands_processed << std::endl;
        std::cout << "Создано блоков: " << blocks.size() << std::endl;
        
        file.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка загрузки .fboot файла: " << e.what() << std::endl;
        return false;
    }
}
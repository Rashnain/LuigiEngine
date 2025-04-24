#ifndef IMGUI_CONSOLE_HPP
#define IMGUI_CONSOLE_HPP

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>
#include <string>

#define LOGS_MAX_SIZE 100

class Console {

    private:

        Console() {}

        std::vector<std::string> logs;



    public:

        static Console& getInstance(){
            static Console instance;
            return instance;
        }    

        void addLog(std::string log){
            if(logs.size() > LOGS_MAX_SIZE){
                logs.erase(logs.begin());
            }
            logs.push_back(log);
        }

        void displayLogs(){
            for (size_t i = 0; i < logs.size(); i++){
                ImGui::Text("%s", logs[i].c_str());
            }
        }


};


#endif
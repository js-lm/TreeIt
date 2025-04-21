#include <raylib.h>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <raymath.h>

struct Node{
    Vector2 position;
    std::string value;
};

struct Line{
    Vector2 start;
    Vector2 end;
};

std::vector<std::string> values;
int leafSpacing{40};
int nodeRadius{20};
int lineThickness{3};
int textSize{20};
bool isTakingScreenshot{true};
Color backgroundColor{WHITE};
Color nodeColor{BLACK};
Color lineColor{BLACK};
Color textColor{BLACK};
Camera2D camera;
int windowWidth; 
int windowHeight;
std::vector<Node> nodes;
std::vector<Line> lines;
int treeHeight;

void init();
bool handleArguments(int argumentsCount, char *arguments[]);
void draw();
void drawNodes();
void drawLines();
void resetCamera();
void takeScreenshot();
Vector2 getNodePosition(int index);
Vector2 trimLineEndpoint(Vector2 from, Vector2 to);

int main(int argumentsCount, char *arguments[]){
    if(!handleArguments(argumentsCount, arguments)) return 1;
    init();
    if(isTakingScreenshot) takeScreenshot();
    else{
        while(!WindowShouldClose()){
            BeginDrawing();
            ClearBackground(backgroundColor);
            draw();
            EndDrawing();
        }
    }
    return 0;
}

void takeScreenshot(){
    RenderTexture2D texture{LoadRenderTexture(windowWidth, windowHeight)};

    BeginTextureMode(texture);
    ClearBackground(backgroundColor);
    draw();
    EndTextureMode();

    Image image{LoadImageFromTexture(texture.texture)};
    ImageFlipVertical(&image);

    time_t rawTime{std::time(nullptr)};
    auto timeInfo{*std::localtime(&rawTime)};
    std::stringstream fileName;
    fileName << "TreeIt! " << std::put_time(&timeInfo, "%Y-%m-%d %H-%M-%S") << ".png";
    ExportImage(image, fileName.str().c_str());

    UnloadImage(image);
    UnloadRenderTexture(texture);
}

bool handleArguments(int argumentsCount, char *arguments[]){
    auto parseValue{
        [](const std::string &valueString, auto &valueToChange, const std::string &message) -> bool{
            try{
                int tempValue{std::stoi(valueString)};
                if constexpr(std::is_same_v<decltype(valueToChange), unsigned char&>){
                    valueToChange = static_cast<unsigned char>(tempValue);
                }else{
                    valueToChange = static_cast<decltype(valueToChange)>(tempValue);
                }
                return true;
            }catch(std::exception &error){
                std::cerr << "\033[33m[Warning]\033[0m Invalid input for " << message << ": \""
                          << valueString << "\"" << std::endl;
                return false;
            }
        }
    };

    for(size_t i{1}; i < argumentsCount; i++){
        std::string argument{arguments[i]};

        if(argument == "--help" || argument == "-h"){
            std::cout << "TreeIt! v0.0.1\n"
                      << "Render a binary tree and export it as a PNG image\n\n"

                      << "GitHub Repo: https://github.com/js-lm/TreeIt\n"
                      << "Email: me@joshlam.dev\n\n"

                      << "Usage: tree-it --list=<comma-separated integers> [options]\n\n"

                      << "Options:\n"
                      << "  -d, --disable-image-export       Disable image export\n"
                      << "  -t, --transparent-background     Enable transparent background\n"
                      << "  -h, --help                       Show this help message\n"
                      << "      --node-spacing <value>       Set spacing between nodes (default: " << leafSpacing << ")\n"
                      << "      --node-radius <value>        Set radius of each node (default: " << nodeRadius << ")\n"
                      << "      --line-thickness <value>     Set thickness of connecting lines (default: " << lineThickness << ")\n"
                      << "      --label-size <value>         Set font size of node labels (default: " << textSize << ")\n"
                      << "      --node-color <r> <g> <b>     Set node color in RGB (0-255) (default: WHITE)\n"
                      << "      --line-color <r> <g> <b>     Set line color in RGB (0-255) (default: Black)\n"
                      << "      --label-color <r> <g> <b>    Set label color in RGB (0-255) (default: Black)\n\n"

                      << "Example: tree-it --node-color 122 226 207 --transparent-background --list=99,90,10,86,3,8,-5,75,74" << std::endl;
        }else if(argument == "--disable-image-export" || argument == "-d"){
            isTakingScreenshot = false;
        }else if(argument == "--node-spacing" && i + 1 < argumentsCount){
            if(!parseValue(arguments[++i], leafSpacing, "node spacing")) return false;
        }else if(argument == "--node-radius" && i + 1 < argumentsCount){
            if(!parseValue(arguments[++i], nodeRadius, "node radius")) return false;
        }else if(argument == "--line-thickness" && i + 1 < argumentsCount){
            if(!parseValue(arguments[++i], lineThickness, "line thickness")) return false;
        }else if(argument == "--label-size" && i + 1 < argumentsCount){
            if(!parseValue(arguments[++i], textSize, "label size")) return false;
        }else if(argument == "--node-color" && i + 3 < argumentsCount){
            if(!parseValue(arguments[++i], nodeColor.r, "Node Color R")) return false;
            if(!parseValue(arguments[++i], nodeColor.g, "Node Color G")) return false;
            if(!parseValue(arguments[++i], nodeColor.b, "Node Color B")) return false;
        }else if(argument == "--line-color" && i + 3 < argumentsCount){
            if(!parseValue(arguments[++i], lineColor.r, "Line Color R")) return false;
            if(!parseValue(arguments[++i], lineColor.g, "Line Color G")) return false;
            if(!parseValue(arguments[++i], lineColor.b, "Line Color B")) return false;
        }else if(argument == "--label-color" && i + 3 < argumentsCount){
            if(!parseValue(arguments[++i], textColor.r, "Label Color R")) return false;
            if(!parseValue(arguments[++i], textColor.g, "Label Color G")) return false;
            if(!parseValue(arguments[++i], textColor.b, "Label Color B")) return false;
        }else if(argument == "--transparent-background" || argument == "-t"){
            backgroundColor = BLANK;
        }else if(argument.find("--list=") == 0){
            try{
                std::stringstream substringStream(argument.substr(7));
                std::string thisValue;
        
                while(std::getline(substringStream, thisValue, ',')){
                    values.emplace_back(thisValue);
                }
            }catch(const std::exception &error){
                std::cerr << "\033[33m[Warning]\033[0m Failed to parse list \"" << argument 
                          << "\": " << error.what() << ". Use `--help` for input format." << std::endl;
                return false;
            }
        }else{
            std::cerr << "\033[33m[Warning]\033[0m Unknown option: \""
                      << argument << "\". Use `--help` to see available options." << std::endl;
            return false;
        }
    }

    return argumentsCount > 1 && !values.empty();
}

void init(){
    treeHeight = std::floor(std::log2(values.size()));

    for(size_t i{values.size()}; i--> 0;){
        Node node;
        node.value = values[i];
        node.position = getNodePosition(i);

        if(i != 0){
            Line line;
            line.start = node.position;
            line.end = getNodePosition(std::floor((i - 1) / 2));

            lines.emplace_back(line);
        }

        nodes.emplace_back(node);
    }

    int rightMostAtLeaveLevel{static_cast<int>(std::pow(2, treeHeight + 1) - 2)};
    Vector2 rightMostPosition{getNodePosition(rightMostAtLeaveLevel)};

    windowWidth = rightMostPosition.x + leafSpacing;
    windowHeight = rightMostPosition.y + leafSpacing;
    
    SetTraceLogLevel(LOG_NONE);
    SetWindowState(FLAG_WINDOW_TRANSPARENT);
    InitWindow(windowWidth, windowHeight, "TreeIt!");
    
    SetTargetFPS(60);

    resetCamera();
}

Vector2 getNodePosition(int index){
    int level{static_cast<int>(std::floor(std::log2(index + 1)))};
    int leftMostAtLevel{static_cast<int>(std::pow(2, level) - 1)};
    int distanceFromLeftmost{index - leftMostAtLevel};

    float padding{static_cast<int>(std::pow(2, (treeHeight - level))) * (leafSpacing + nodeRadius) / 2.0f};
    float x{padding + distanceFromLeftmost * padding * 2};
    float y{level * (leafSpacing + nodeRadius) + nodeRadius * 1.5f};

    return {x, y};
}

void resetCamera(){
    camera = {{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
}

void draw(){
    for(const auto &line : lines){
        Vector2 trimmedStart{trimLineEndpoint(line.start, line.end)};
        Vector2 trimmedEnd{trimLineEndpoint(line.end, line.start)};
        DrawLineEx(trimmedStart, trimmedEnd, lineThickness, lineColor);
    }

    for(const auto &node : nodes){
        DrawRing(node.position, nodeRadius - lineThickness, nodeRadius, 0, 360, 64, nodeColor);

        const std::string &label{node.value};

        DrawText(
            label.c_str(), 
            node.position.x - MeasureText(label.c_str() , textSize) / 2, 
            node.position.y - textSize / 2, 
            textSize, 
            textColor
        );
    }
}

Vector2 trimLineEndpoint(Vector2 from, Vector2 to){
    Vector2 direction{Vector2Subtract(to, from)};
    float length{Vector2Length(direction)};

    if(length == 0) return from;

    Vector2 normalizedDirection{Vector2Scale(direction, nodeRadius / length)};
    return Vector2Add(from, normalizedDirection);
}
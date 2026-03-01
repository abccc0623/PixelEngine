#pragma once

enum class LogLevel { INFO, WARN, ERR };
class Log
{
public:
    static void Init(const std::string& filename);
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);

    static void LogMemory(const char* file, int line, const std::string& msg) 
    {
        std::time_t now = std::time(nullptr);
        char timeBuf[20];
        struct tm tstruct;
        localtime_s(&tstruct, &now);
        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tstruct);

        // [시간][MEMORY] 파일명:줄번호 - 메시지 형식으로 출력
        std::cout << "\033[36m" << "[" << timeBuf << "][MEMORY] "
            << file << ":" << line << " - " << msg << "\033[0m" << std::endl;
    }
private:
    static std::ofstream& GetFileStream();
    static void Print(LogLevel levtl, const std::string& msg, const std::string& color);
};


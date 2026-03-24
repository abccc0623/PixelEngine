#pragma once
#include <string>
enum class LogLevel { INFO, WARN, ERR };

class PixelLog
{
public:
    static void Init(const std::string& filename);
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);

    static void LogMemory(const char* file, int line, const std::string& msg)
    {
        // ... (기존 코드 동일) ...
        std::cout << "[MEMORY] " << file << ":" << line << " - " << msg << std::endl;
    }

private:
    static std::ofstream& GetFileStream();
    // LogLevel이 어디 정의되어 있느냐에 따라 Pixel::LogLevel 혹은 내부 정의
    static void Print(LogLevel level, const std::string& msg, const std::string& color);
};

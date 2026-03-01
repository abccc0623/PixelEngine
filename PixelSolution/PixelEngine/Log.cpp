#include "pch.h"
#include "Export/PixelEngineAPI.h"
#include "Log.h"

extern LogCallbackFunc g_logCallback;
void Log::Init(const std::string& filename)
{
	GetFileStream().open(filename, std::ios::app);
}
void Log::Info(const std::string& msg)
{
	Print(LogLevel::INFO, msg, "\033[32m"); // Green
}
void Log::Warn(const std::string& msg)
{
	Print(LogLevel::WARN, msg, "\033[33m"); // Yellow
}
void Log::Error(const std::string& msg)
{
	Print(LogLevel::ERR, msg, "\033[31m"); // Red
}

std::ofstream& Log::GetFileStream()
{
	static std::ofstream file;
	return file;
}

void Log::Print(LogLevel Level, const std::string& msg, const std::string& color)
{
	// 시간 계산
	std::time_t now = std::time(nullptr);
	struct tm tstruct;
	char timeBuf[20];

	// MSVC 전용 안전한 함수
	localtime_s(&tstruct, &now);
	std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tstruct);

	DispatchNativeLog(msg.c_str(), (int)Level);

	std::string strLevel = "";
	switch (Level) {
	case LogLevel::INFO: 
		strLevel = "INFO";
		break;
	case LogLevel::WARN:
		strLevel =  "WARN";
		break;
	case LogLevel::ERR:  
		strLevel = "ERR"; 
		break;
	default:             
		strLevel = "UNKNOWN";
		break;
	}

	// 2. 파일 출력 (파일이 열려있을 경우에만)
	if (GetFileStream().is_open()) {
		GetFileStream() << "[" << timeBuf << "][" << strLevel << "] " << msg << std::endl;
		GetFileStream().flush();
	}
}



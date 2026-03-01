#pragma once
#include <iostream>
#include <map>
#include <string>

struct MemoryInfo 
{
    const char* file;
    int line;
    size_t size;
};

class MemoryTracker {
public:
    static MemoryTracker& Get()
    {
        static MemoryTracker instance;
        return instance;
    }

    bool m_isTracking = false;
    bool m_isReleased = false; // 엔진 종료 시작 플래그

    void Add(void* ptr, size_t size, const char* file, int line) {
        if (m_isTracking || m_isReleased) return;

        m_isTracking = true;
        m_allocMap[ptr] = { file, line, size };
        m_isTracking = false;
    }

    void Remove(void* ptr)
    {
        // 이미 트래커가 파괴되었거나 종료 중이면 추적 중단
        if (m_isTracking || m_isReleased || ptr == nullptr) return;

        m_isTracking = true;
        auto it = m_allocMap.find(ptr);
        if (it != m_allocMap.end()) {
            m_allocMap.erase(it);
        }
        m_isTracking = false;
    }

    void ReportLeaks()
    {
        m_isReleased = true; // 리포트 시작 시점부터 모든 추적 중단

        if (m_allocMap.empty()) {
            std::cout << "[INFO] Memory Check: No Leaks Detected." << std::endl;
        }
        else {
            std::cout << "\033[31m--- MEMORY LEAK REPORT ---\033[0m" << std::endl;
            for (auto const& [ptr, info] : m_allocMap) {
                std::cout << "[LEAK] " << info.file << ":" << info.line
                    << " | Address: " << ptr << " | Size: " << info.size << " bytes" << std::endl;
            }
        }
    }

private:
    std::map<void*, MemoryInfo> m_allocMap;
    MemoryTracker() : m_isTracking(false), m_isReleased(false) {}

    // 소멸자에서 플래그를 세워 소멸 후 접근 방지
    ~MemoryTracker() { m_isReleased = true; }
};

void* operator new(size_t size, const char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, const char* file, int line) noexcept;

// 매크로 정의
#define PE_NEW new(__FILE__, __LINE__)
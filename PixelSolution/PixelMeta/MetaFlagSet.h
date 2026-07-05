#pragma once
class MetaFlagSet
{
private:
    long m_Flags;

public:
    // 기본 생성자 (None으로 초기화)
    MetaFlagSet() : m_Flags(0) {}

    // 초기값을 받는 생성자
    MetaFlagSet(long initialFlags) : m_Flags(initialFlags) {}

    // [켜기] 특정 플래그 추가
    void Set(long flag)
    {
        m_Flags |= flag;
    }

    // [끄기] 특정 플래그 제거
    void Clear(long flag)
    {
        m_Flags &= ~flag;
    }

    // [확인] 특정 플래그가 켜져 있는지 검사
    bool Has(long flag) const
    {
        return (m_Flags & flag) != 0;
    }

    // [반전] 켜져 있으면 끄고, 꺼져 있으면 켬
    void Toggle(long flag)
    {
        m_Flags ^= flag;
    }

    // [초기화] 모든 플래그 끄기
    void ClearAll()
    {
        m_Flags = 0;
    }

    // 원본 정수값 반환 (제너레이터 등에서 날것의 값이 필요할 때)
    long GetRaw() const
    {
        return m_Flags;
    }
};